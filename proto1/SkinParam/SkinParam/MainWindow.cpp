/**
 * App and MainWindow
 */
#include "stdafx.h"

#include "MainWindow.h"
#include "Renderer.h"

#include "Triangle.h"
#include "Head.h"
#include "Light.h"

#include <dxgidebug.h>

using namespace Skin;
using namespace Utils;

namespace Skin {
	CMainApp app;
} // namespace Skin

class CMainWindow::UIRenderable : public Renderable {
private:
	CDXUTDialog* m_pDialog;
	DWORD lastTick;
public:
	UIRenderable(CDXUTDialog* pDialog) {
		m_pDialog = pDialog;
		lastTick = GetTickCount();
	}
	void init(ID3D11Device* pd3dDevice, IRenderer* pRenderer) override {

	}
	void cleanup(IRenderer* pRenderer) override {

	}
	bool inScene() const override {
		return false;
	}
	void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) override {
		DWORD tick = GetTickCount();
		float elapsedTime = (tick - lastTick) / 1000.0f;
		if (elapsedTime > 1.0f)
			elapsedTime = 1.0f;
		m_pDialog->OnRender(elapsedTime);
		lastTick = tick;
	}
};

CMainWindow* CMainApp::GetMainWindow()
{
	return static_cast<CMainWindow*>(m_pMainWnd);
}

BOOL CMainApp::InitInstance() {
	//_crtBreakAlloc = 204;
	CoInitialize(nullptr);
	SetRegistryKey(_APP_NAME_);

	m_pMainWnd = NULL;
	m_pMainWnd = new CMainWindow();
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

BOOL CMainApp::OnIdle(LONG lCount) {
	if (CWinApp::OnIdle(lCount))
		return TRUE;
	return GetMainWindow()->OnIdle(lCount);
}

BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

const double CMainWindow::CTRL_MINIFIER = 1 / 3.0;
const double CMainWindow::SHIFT_MAGNIFIER = 3.0;

CMainWindow::CMainWindow()
	: m_camera(Vector(0, -5, 0), Vector(0, 0, 0), Vector(0, 0, 1))
{
	CSize resolution(1284, 724);
	Create(NULL, _APP_NAME_, WS_OVERLAPPEDWINDOW & (~WS_SIZEBOX) & (~WS_MAXIMIZEBOX), 
		CRect(0, 0, resolution.cx, resolution.cy));

	// Adjust window size to exclude border size
	CRect windowRect;
	GetWindowRect(&windowRect);
	GetClientRect(&m_rectClient);
	MoveWindow(0, 0, resolution.cx + windowRect.Width() - m_rectClient.Width(), 
		resolution.cy + windowRect.Height() - m_rectClient.Height(), FALSE);
	GetClientRect(&m_rectClient);

	DXUTInit();
	DXUTSetWindow(m_hWnd, m_hWnd, m_hWnd, false);

	initUI();

	m_pRenderer = new Renderer(m_hWnd, CRect(0, 0, resolution.cx, resolution.cy), &m_config, &m_camera, this);
	GetClientRect(&m_rectClient);

	initUIDialogs();

	m_pTriangle = new Triangle();
	m_pHead = new Head();
	//m_pRenderer->addRenderable(m_pTriangle);
	m_pRenderer->addRenderable(m_pHead);

	m_pLight1 = new Light(Vector(3, 7, 0), Color::Black, Color::White * 3.2f, Color::White * 3.2f, 1.0f, 0.1f, 0.0f);
	m_pLight2 = new Light(Vector(-3, -3, 5), Color::Black, Color::White * 3.2f, Color::White * 3.2f, 1.0f, 0.1f, 0.0f);
	m_pRenderer->addLight(m_pLight1);
	m_pRenderer->addLight(m_pLight2);

	m_pRenderer->setGlobalAmbient(Color::White * 0.0005f);

	m_bChangingView = false;
	m_camera.restrictView(1.2, 8.0);

	m_pRenderer->addRenderable(m_puirInfo);
	m_pRenderer->addRenderable(m_puirGeneral);
	m_pRenderer->addRenderable(m_puirSSS);

	updateUI();
}

CMainWindow::~CMainWindow() {
	m_pRenderer->removeAllRenderables();
	m_pRenderer->removeAllLights();

	delete m_pRenderer;
	delete m_pTriangle;
	delete m_pHead;
	delete m_pLight1;
	delete m_pLight2;

	uninitUI();

	DXUTShutdown();
}

void CMainWindow::showInfo() {
	TStringStream tss;
	tss << _APP_NAME_ << _T(" (FPS: ")
		<< std::setiosflags(std::ios::fixed) << std::setprecision(1) << m_pRenderer->getFPS()
		<< _T(")");
	SetWindowText(tss.str().c_str());
}

double CMainWindow::getViewModifier(UINT nFlags) {
	if (nFlags & MK_CONTROL)
		return CTRL_MINIFIER;
	if (nFlags & MK_SHIFT)
		return SHIFT_MAGNIFIER;
	return 1.0;
}

BOOL CMainWindow::OnIdle(LONG lCount) {
	UNREFERENCED_PARAMETER(lCount);

	// Stop doing work while minimized
	if (IsIconic() || GetForegroundWindow() != this) {
		return FALSE;
	}

	m_pRenderer->render();

	showInfo();
	updateUI();
	return TRUE;
}

void CMainWindow::onCreateDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IDXGISwapChain* pSwapChain) {
	// don't init here -- let ui initializer do this
	//m_pDialogResourceManager->OnD3D11CreateDevice(pDevice, pDeviceContext);
}

void CMainWindow::onResizedSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) {
	m_pDialogResourceManager->OnD3D11ResizedSwapChain(pDevice, pBackBufferSurfaceDesc);
}

void CMainWindow::onReleasingSwapChain() {
	m_pDialogResourceManager->OnD3D11ReleasingSwapChain();
}

void CMainWindow::onDestroyDevice() {
	m_pDialogResourceManager->OnD3D11DestroyDevice();
}

void CMainWindow::initDialog(CDXUTDialog* pdlg) {
	// perform common initializations for dialogs
	pdlg->Init(m_pDialogResourceManager);
	pdlg->SetCallback(CMainWindow::OnGUIEvent, this);
}

void CMainWindow::initInfoDialog() {
	m_pdlgInfo = new CDXUTDialog();
	m_puirInfo = new UIRenderable(m_pdlgInfo);
	initDialog(m_pdlgInfo);

	int tmp;
	CDXUTStatic* lblTmp;
	m_pdlgInfo->AddStatic(CID_INFO_LBL_DRIVER_TYPE_LABEL, _T("Driver: "), 6, tmp = 6, 60, 20, false, &lblTmp);
	m_pdlgInfo->AddStatic(CID_INFO_LBL_DRIVER_TYPE, _T(""), 70, tmp, 80, 20, false, &m_plblDriverType);
	m_pdlgInfo->AddStatic(CID_INFO_LBL_SCREEN_SIZE_LABEL, _T("Resolution: "), 6, tmp += 20, 90, 20, false, &lblTmp);
	m_pdlgInfo->AddStatic(CID_INFO_LBL_SCREEN_SIZE, _T(""), 100, tmp, 80, 20, false, &m_plblScreenSize);
	m_pdlgInfo->AddStatic(CID_INFO_LBL_FPS_LABEL, _T("FPS: "), 6, tmp += 20, 40, 20, false, &lblTmp);
	m_pdlgInfo->AddStatic(CID_INFO_LBL_FPS, _T(""), 50, tmp, 50, 20, false, &m_plblFPS);

	m_pdlgInfo->SetVisible(true);
}

void CMainWindow::initGeneralDialog() {
	m_pdlgGeneral = new CDXUTDialog();
	m_puirGeneral = new UIRenderable(m_pdlgGeneral);
	initDialog(m_pdlgGeneral);

	int width = m_rectClient.Width();
	int tmp;
	CDXUTStatic* lblTmp;
	DBG_UNREFERENCED_LOCAL_VARIABLE(lblTmp);
	m_pdlgGeneral->AddStatic(CID_GENERAL_LBL_CAPTION, _T("Rendering Options"), width - 156, tmp = 6, 130, 20);
#define AddCheckBoxForBool(dialog, cid, text, name) dialog->AddCheckBox(cid, _T(text), width - 156, tmp += 20, 120, 20, m_pRenderer->get##name(), 0, false, &m_pchk##name)
#define RegisterHandlerForBool(cid, name) registerEventHandler(cid, &CMainWindow::chk##name##_Handler)
#define SetupControlForBool(dialog, cid, text, name) do { AddCheckBoxForBool(dialog, cid, text, name); RegisterHandlerForBool(cid, name); } while (false)

	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_TESSELLATION,    "(T)essellation", Tessellation );
	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_BUMP,            "(B)ump mapping", Bump         );
	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_WIREFRAME,       "Wire(f)rame"   , Wireframe    );
//	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_VSM_BLUR,        "(V)SM Blurring", VSMBlur      );
	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_POST_PROCESS_AA, "FX(A)A"        , PostProcessAA);
	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_BLOOM,           "Bloom"         , Bloom        );

	//m_dlgTessellation.EnableNonUserEvents(true);
	m_pdlgGeneral->SetVisible(true);
}

void CMainWindow::initSSSDialog() {
	m_pdlgSSS = new CDXUTDialog();
	m_puirSSS = new UIRenderable(m_pdlgSSS);
	initDialog(m_pdlgSSS);

	int height = m_rectClient.Height();
	int tmp;
	CDXUTStatic* lblTmp;
	DBG_UNREFERENCED_LOCAL_VARIABLE(lblTmp);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_CAPTION, _T("Subsurface Scattering"), 6, tmp = height - 106, 200, 20);
	m_pdlgSSS->AddCheckBox(CID_SSS_CHK_ENABLE_SSS, _T("Enable (S)SS"), 6, tmp += 20, 200, 20, m_pRenderer->getSSS(), 0, false, &m_pchkEnableSSS);
	m_pdlgSSS->AddCheckBox(CID_SSS_CHK_ADAPTIVE_GAUSSIAN, _T("A(d)aptive Blurring"), 6, tmp += 20, 200, 20, m_pRenderer->getAdaptiveGaussian(), 0, false, &m_pchkAdaptiveGaussian);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_SSS_STRENGTH_LABEL, _T("SSS Strength: "), 6, tmp += 20, 200, 20);
	m_pdlgSSS->AddSlider(CID_SSS_SLD_SSS_STRENGTH, 6, tmp += 20, 120, 20, 0, 300, (int)(100.0f * m_pRenderer->getSSSStrength() + 0.5f), false, &m_psldSSSStrength);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_SSS_STRENGTH, _T("1.00"), 140, tmp, 60, 20, false, &m_plblSSSStrength);
	
	m_pdlgSSS->SetVisible(true);

	registerEventHandler(CID_SSS_CHK_ENABLE_SSS, &CMainWindow::chkEnableSSS_Handler);
	registerEventHandler(CID_SSS_CHK_ADAPTIVE_GAUSSIAN, &CMainWindow::chkAdaptiveGaussian_Handler);
	registerEventHandler(CID_SSS_SLD_SSS_STRENGTH, &CMainWindow::sldSSSStrength_Handler);
}

void CMainWindow::initUIDialogs() {
	initInfoDialog();
	initGeneralDialog();
	initSSSDialog();

	m_pDialogResourceManager->OnD3D11CreateDevice(m_pRenderer->getDevice(), m_pRenderer->getDeviceContext());
}

void CMainWindow::initUI() {
	m_pDialogResourceManager = new CDXUTDialogResourceManager();

	m_vppuirs.push_back(&m_puirInfo);
	m_vppuirs.push_back(&m_puirGeneral);
	m_vppuirs.push_back(&m_puirSSS);

	m_vppdlgs.push_back(&m_pdlgInfo);
	m_vppdlgs.push_back(&m_pdlgGeneral);
	m_vppdlgs.push_back(&m_pdlgSSS);
}

void CMainWindow::uninitUI() {
	for (CDXUTDialog** ppdlg : m_vppdlgs) {
		if (*ppdlg)
			delete *ppdlg;
		*ppdlg = nullptr;
	}
	m_vppdlgs.clear();
	for (UIRenderable** ppuir : m_vppuirs) {
		if (*ppuir)
			delete *ppuir;
		*ppuir = nullptr;
	}
	m_vppuirs.clear();

	delete m_pDialogResourceManager;
}

void CMainWindow::updateUI() {
	// Info dialog
	{
		TStringStream tss;
		tss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << m_pRenderer->getFPS();
		m_plblFPS->SetText(tss.str().c_str());
		int iFPS = (int)(m_pRenderer->getFPS() + 0.05);
		D3DXCOLOR color;
		if (iFPS >= 40)
			color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
		else if (iFPS >= 20)
			color = D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
		else color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
		m_plblFPS->SetTextColor(color);
	}
	{
		TStringStream tss;
		tss << m_rectClient.Width() << "x" << m_rectClient.Height();
		m_plblScreenSize->SetText(tss.str().c_str());
	}
	switch (m_pRenderer->getDriverType()) {
	case D3D_DRIVER_TYPE_HARDWARE:
		m_plblDriverType->SetText(_T("Hardware"));
		break;
	case D3D_DRIVER_TYPE_SOFTWARE:
		m_plblDriverType->SetText(_T("Software"));
		break;
	case D3D_DRIVER_TYPE_REFERENCE:
		m_plblDriverType->SetText(_T("Reference"));
		break;
	case D3D_DRIVER_TYPE_WARP:
		m_plblDriverType->SetText(_T("WARP"));
		break;
	default:
		m_plblDriverType->SetText(_T("Unknown"));
		break;
	}

	// General dialog
#define UpdateUIForBool(name) m_pchk##name->SetChecked(m_pRenderer->get##name())
	UpdateUIForBool(Tessellation);
	UpdateUIForBool(Bump);
	UpdateUIForBool(Wireframe);
//	UpdateUIForBool(VSMBlur);
	UpdateUIForBool(PostProcessAA);
	UpdateUIForBool(Bloom);

	// SSS dialog
	m_pchkEnableSSS->SetChecked(m_pRenderer->getSSS());
	m_pchkAdaptiveGaussian->SetChecked(m_pRenderer->getAdaptiveGaussian());
	m_pchkAdaptiveGaussian->SetEnabled(m_pchkEnableSSS->GetChecked());
	m_psldSSSStrength->SetEnabled(m_pchkEnableSSS->GetChecked());
}

void CMainWindow::OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl) {
	auto iter = m_mapMessages.find(nControlID);
	if (m_mapMessages.end() == iter) return;

	GUIEventHandler handler = iter->second;
	(this->*handler)(pControl, nEvent);
}

void CALLBACK CMainWindow::OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext) {
	if (pUserContext == NULL)
		return;
	try {
		CMainWindow* pWindow = static_cast<CMainWindow*>(pUserContext);
		pWindow->OnGUIEvent(nEvent, nControlID, pControl);
	} catch (...) {
		return;
	}
}

void CMainWindow::registerEventHandler(int nControlID, GUIEventHandler handler) {
	m_mapMessages[nControlID] = handler;
}

void CMainWindow::unregisterEventHandler(int nControlID) {
	m_mapMessages.erase(nControlID);
}

#define DefineHandlerForBool(name) \
	void CALLBACK CMainWindow::chk##name##_Handler(CDXUTControl* sender, UINT nEvent) { \
		m_pRenderer->set##name(m_pchk##name->GetChecked()); \
	}
DefineHandlerForBool(Tessellation)
DefineHandlerForBool(Bump)
DefineHandlerForBool(Wireframe)
//DefineHandlerForBool(VSMBlur)
DefineHandlerForBool(PostProcessAA)
DefineHandlerForBool(Bloom)

void CALLBACK CMainWindow::chkEnableSSS_Handler(CDXUTControl* sender, UINT nEvent) {
	m_pRenderer->setSSS(m_pchkEnableSSS->GetChecked());
}

void CALLBACK CMainWindow::chkAdaptiveGaussian_Handler(CDXUTControl* sender, UINT nEvent) {
	m_pRenderer->setAdaptiveGaussian(m_pchkAdaptiveGaussian->GetChecked());
}

void CALLBACK CMainWindow::sldSSSStrength_Handler(CDXUTControl* sender, UINT nEvent) {
	double strength = m_psldSSSStrength->GetValue() / 100.0;
	TStringStream tss;
	tss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << strength;
	m_plblSSSStrength->SetText(tss.str().c_str());
	m_pRenderer->setSSSStrength((float)strength);
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg) {
	if (m_bChangingView)
		return FALSE;

	if (m_pDialogResourceManager->MsgProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam)) {
		return TRUE;
	}

	if (m_pdlgGeneral->MsgProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam)) {
		return TRUE;
	}

	if (m_pdlgSSS->MsgProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam)) {
		return TRUE;
	}

	return FALSE;
}

afx_msg void CMainWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// bit 14 : previous key state
	if (nFlags & 0x4000u) return;
	switch (nChar) {
	case 'F':
		m_pRenderer->toggleWireframe();
		break;
	case 'T':
		m_pRenderer->toggleTessellation();
		break;
	case 'B':
		m_pRenderer->toggleBump();
		break;
	case 'S':
		m_pRenderer->toggleSSS();
		break;
	case 'A':
		m_pRenderer->togglePostProcessAA();
		break;
	case 'V':
		m_pRenderer->toggleVSMBlur();
		break;
	case 'D':
		m_pRenderer->toggleAdaptiveGaussian();
		break;
	case VK_F8:
		m_pRenderer->dump();
		break;
	}
}

afx_msg void CMainWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
}

afx_msg void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnRButtonDown(UINT nFlags, CPoint point) {
	SetCapture();
	m_bChangingView = true;
	m_ptStart = point;
}

afx_msg void CMainWindow::OnLButtonUp(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnRButtonUp(UINT nFlags, CPoint point) {
	m_bChangingView = false;
	ReleaseCapture();
}

afx_msg void CMainWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnMouseMove(UINT nFlags, CPoint point) {
	if (m_bChangingView) {
		CPoint ptDist = (point - m_ptStart);
		m_ptStart = point;

		double modifier = getViewModifier(nFlags);
		m_camera.changeView(modifier * ptDist.x * Math::PI / 720, modifier * ptDist.y * Math::PI / 720);
	}
}

afx_msg BOOL CMainWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	double delta = 0.2 / WHEEL_DELTA * zDelta;
	m_camera.moveView(getViewModifier(nFlags) * delta);

	return FALSE;
}

afx_msg void CMainWindow::OnClose() {
	CFrameWnd::OnClose();
}

afx_msg void CMainWindow::OnDestroy() {
	CFrameWnd::OnDestroy();
}
