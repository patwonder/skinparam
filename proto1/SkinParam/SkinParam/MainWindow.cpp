/**
 * App and MainWindow
 */
#include "stdafx.h"

#include "MainWindow.h"
#include "Renderer.h"

#include "Triangle.h"
#include "Head.h"
#include "Light.h"

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

	CMainWindow* pWnd;
	m_pMainWnd = NULL;
	m_pMainWnd = pWnd = new CMainWindow();
	pWnd->init();

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
const float CMainWindow::BLINK_DIM_FACTOR = 0.5f;

CMainWindow::CMainWindow()
	: m_camera(Vector(0, -5, 0), Vector(0, 0, 0), Vector(0, 0, 1)),
	  m_vps(0.005, 0.7, 0.01, 0.75)
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
}

void CMainWindow::init() {
	DXUTInit();
	DXUTSetWindow(m_hWnd, m_hWnd, m_hWnd, false);

	initUI();

	m_pRenderer = new Renderer(m_hWnd, CRect(0, 0, m_rectClient.Width(), m_rectClient.Height()), &m_config, &m_camera, this);
	GetClientRect(&m_rectClient);

	m_pTriangle = new Triangle();
	m_pHead = new Head();
	//m_pRenderer->addRenderable(m_pTriangle);
	m_pRenderer->addRenderable(m_pHead);

	memset(m_pLights, 0, sizeof(m_pLights));
	m_pLights[0] = new Light(Vector(2.7574, 6.4340, 0), Color::Black, Color::White * 3.2f, Color::White * 3.2f, 1.0f, 0.1f, 0.0f);
	m_pLights[1] = new Light(Vector(-3.2025, -3.2025, 5.3374), Color::Black, Color::White * 3.2f, Color::White * 3.2f, 1.0f, 0.1f, 0.0f);
	m_pRenderer->addLight(m_pLights[0]);
	m_pRenderer->addLight(m_pLights[1]);

	m_pRenderer->setGlobalAmbient(Color::White * 0.25f);

	m_bChangingView = false;
	m_camera.restrictView(1.2, 8.0);

	m_bChangingLight = false;
	m_pCurrentLight = m_pLights[0];
	m_nCurrentLightID = 0;

	m_nBlinkStartTick = 0;

	updateSkinParams();

	initUIDialogs();

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
	for (UINT i = 0; i < NUM_LIGHTS; i++) {
		delete m_pLights[i];
	}

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

	// do blink
	ULONGLONG tick = GetTickCount64();
	ULONGLONG diff = tick - m_nBlinkStartTick;
	for (UINT i = 0; i < NUM_LIGHTS; i++) {
		float intensity = (float)m_psldLights[i]->GetValue() / 10.0f;
		if (i == m_nCurrentLightID && diff <= BLINK_DURATION) {
			diff = diff % BLINK_INTERVAL;
			if (diff < BLINK_SUB_DURATION) {
				intensity *= BLINK_DIM_FACTOR;
			}
		}
		Light& l = *m_pLights[i];
		l.coSpecular = l.coDiffuse = Color::White * intensity;
	}

	updateUI();

	m_pRenderer->render(m_bChangingLight ? &m_pRenderer->getLightCamera(*m_pCurrentLight) : nullptr);

	showInfo();
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
	SetupControlForBool(m_pdlgGeneral, CID_GENERAL_CHK_BLOOM,           "Bl(o)om"       , Bloom        );

	m_pdlgGeneral->AddStatic(CID_GENERAL_LBL_DRAG_LIGHT_CAPTION1, _T("Left drag to move"), width - 156, tmp += 20, 190, 20);
	m_pdlgGeneral->AddStatic(CID_GENERAL_LBL_DRAG_LIGHT_CAPTION2, _T("selected (l)ight"), width - 156, tmp += 20, 190, 20);

	for (UINT i = 0; i < NUM_LIGHTS; i++) {
		UINT baseid = CID_GENERAL_LIGHT_START + CID_GENERAL_LIGHT_STRIDE * i;
		UINT intensity = (UINT)(m_pLights[i]->coDiffuse.red * 10.0f + 0.5f);
		TStringStream tss, tss2;
		tss << _T("Light ") << i + 1;
		m_pdlgGeneral->AddRadioButton(baseid + CID_GENERAL_RD_LIGHT_SELECT_OFFSET, CID_GENERAL_RDGROUP_LIGHT, tss.str().c_str(),
			width - 156, tmp += 20, 120, 20, m_pCurrentLight == m_pLights[i], 0, false, &m_prdLights[i]);
		m_pdlgGeneral->AddSlider(baseid + CID_GENERAL_SLD_LIGHT_INTENSITY_OFFSET, width - 156, tmp += 20, 100, 20, 0, 100, intensity,
			false, &m_psldLights[i]);
		tss2 << std::setiosflags(std::ios::fixed) << std::setprecision(1) << (float)intensity / 10.0f;
		m_pdlgGeneral->AddStatic(baseid + CID_GENERAL_LBL_LIGHT_INTENSITY_OFFSET, tss2.str().c_str(), width - 41, tmp, 35, 20,
			false, &m_plblLights[i]);

		registerEventHandler(baseid + CID_GENERAL_RD_LIGHT_SELECT_OFFSET, &CMainWindow::rdLight_Handler);
		registerEventHandler(baseid + CID_GENERAL_SLD_LIGHT_INTENSITY_OFFSET, &CMainWindow::sldLight_Handler);
	}

	UINT intensity = (UINT)(m_pRenderer->getGlobalAmbient().red * 100.0f + 0.5f);
	TStringStream tss;
	m_pdlgGeneral->AddStatic(CID_GENERAL_LBL_AMBIENT_CAPTION, _T("Ambient intensity"), width - 156, tmp += 20, 120, 20);
	m_pdlgGeneral->AddSlider(CID_GENERAL_SLD_AMBIENT_INTENSITY, width - 156, tmp += 20, 100, 20, 0, 100, intensity,
		false, &m_psldAmbient);
	tss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << (float)intensity / 100.0f;
	m_pdlgGeneral->AddStatic(CID_GENERAL_LBL_AMBIENT_INTENSITY, tss.str().c_str(), width - 41, tmp, 35, 20,
		false, &m_plblAmbient);

	registerEventHandler(CID_GENERAL_SLD_AMBIENT_INTENSITY, &CMainWindow::sldAmbient_Handler);

	// dump
	m_pdlgGeneral->AddButton(CID_GENERAL_BTN_DUMP, _T("Dump Pipeline (F8)"), width - 156, tmp += 30, 150, 30, 0,
		false, &m_pbtnDump);

	registerEventHandler(CID_GENERAL_BTN_DUMP, &CMainWindow::btnDump_Handler);

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
	m_pdlgSSS->AddStatic(CID_SSS_LBL_CAPTION, _T("Subsurface Scattering"), 6, tmp = height - 246, 200, 20);
	m_pdlgSSS->AddCheckBox(CID_SSS_CHK_ENABLE_SSS, _T("Enable (S)SS"), 6, tmp += 20, 200, 20, m_pRenderer->getSSS(), 0, false, &m_pchkEnableSSS);
	m_pdlgSSS->AddCheckBox(CID_SSS_CHK_ADAPTIVE_GAUSSIAN, _T("A(d)aptive Blurring"), 6, tmp += 20, 200, 20, m_pRenderer->getAdaptiveGaussian(), 0, false, &m_pchkAdaptiveGaussian);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_SSS_STRENGTH_LABEL, _T("SSS Strength: "), 6, tmp += 20, 200, 20);
	m_pdlgSSS->AddSlider(CID_SSS_SLD_SSS_STRENGTH, 6, tmp += 20, 120, 20, 0, 300, (int)(100.0f * m_pRenderer->getSSSStrength() + 0.5f), false, &m_psldSSSStrength);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_SSS_STRENGTH, _T("1.00"), 140, tmp, 60, 20, false, &m_plblSSSStrength);

#define AddControlsForParam(param, PARAM, label, minv, maxv, curv) do { \
	m_pdlgSSS->AddStatic(CID_SSS_LBL_F_##PARAM##_LABEL, _T(label), 6, tmp += 20, 40, 20); \
	m_pdlgSSS->AddSlider(CID_SSS_SLD_F_##PARAM, 46, tmp, 120, 20, minv, maxv, curv, false, &m_psldSSS_f_##param); \
	m_pdlgSSS->AddStatic(CID_SSS_LBL_F_##PARAM, getParamDescription(vps.f_##param).c_str(), 180, tmp, 60, 20, false, &m_plblSSS_f_##param); \
} while (false)
#define RegisterHandlerForParam(param, PARAM) registerEventHandler(CID_SSS_SLD_F_##PARAM, &CMainWindow::sldSSS_f_##param##_Handler)
#define SetupControlsForParam(param, PARAM, label, minv, maxv, curv) do { AddControlsForParam(param, PARAM, label, minv, maxv, curv); \
	RegisterHandlerForParam(param, PARAM); } while (false)

	const VariableParams& vps = m_vps;
	SetupControlsForParam(mel,   MEL,   "mel",  0, 100, (int)(sqrt(vps.f_mel * 20000) + .5));
	SetupControlsForParam(eu,    EU,    "eum",  0, 100, (int)(vps.f_eu    *  100 +   .5));
	SetupControlsForParam(blood, BLOOD, "bld",  0, 100, (int)(sqrt(vps.f_blood * 100000) + .5));
	SetupControlsForParam(ohg,   OHG,   "ohg", 40,  80, (int)(vps.f_ohg   *   40 + 40.5));

	m_pdlgSSS->AddStatic(CID_SSS_LBL_ROUGHNESS_LABEL, _T("Rough"), 6, tmp += 20, 60, 20);
	m_pdlgSSS->AddSlider(CID_SSS_SLD_ROUGHNESS, 66, tmp, 100, 20, 5, 100, 30, false, &m_psldSSSRoughness);
	m_pdlgSSS->AddStatic(CID_SSS_LBL_ROUGHNESS, _T("0.30"), 180, tmp, 60, 20, false, &m_plblSSSRoughness);

	m_pdlgSSS->SetVisible(true);

	registerEventHandler(CID_SSS_CHK_ENABLE_SSS, &CMainWindow::chkEnableSSS_Handler);
	registerEventHandler(CID_SSS_CHK_ADAPTIVE_GAUSSIAN, &CMainWindow::chkAdaptiveGaussian_Handler);
	registerEventHandler(CID_SSS_SLD_SSS_STRENGTH, &CMainWindow::sldSSSStrength_Handler);
	registerEventHandler(CID_SSS_SLD_ROUGHNESS, &CMainWindow::sldSSSRoughness_Handler);
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
	m_prdLights[m_nCurrentLightID]->SetChecked(true);

	// SSS dialog
	m_pchkEnableSSS->SetChecked(m_pRenderer->getSSS());
	m_pchkAdaptiveGaussian->SetChecked(m_pRenderer->getAdaptiveGaussian());
	m_pchkAdaptiveGaussian->SetEnabled(m_pchkEnableSSS->GetChecked());
	m_psldSSSStrength->SetEnabled(m_pchkEnableSSS->GetChecked());
}

static TOstream& operator<<(TOstream& os, const Vector& vec) {
	return os << vec.x << _T(" ") << vec.y << _T(" ") << vec.z;
}

static TOstream& operator<<(TOstream& os, const Color& vec) {
	return os << vec.red << _T(" ") << vec.green << _T(" ") << vec.blue;
}

void CMainWindow::copyViewAsPBRT() {
	TStringStream tss;
	static const TCHAR* eol = _T("\r\n");
	tss << _T("LookAt ") << m_camera.getVecEye() << _T(" ")
		<< m_camera.getVecLookAt() << _T(" ") << m_camera.getVecUp()
		<< eol;
	tss << _T("Camera \"perspective\" \"float fov\" [")
		<< m_pRenderer->getFOVDegrees() << _T("]") << eol;
	tss << eol;

	for (int i = 0; i < NUM_LIGHTS; i++) {
		const Light* pLight = m_pLights[i];
		if (pLight && !pLight->coDiffuse.colorEquals(Color::Black)) {
			tss << _T("  AttributeBegin") << eol;
			tss << _T("    AreaLightSource \"area\" \"color L\" [ ")
				<< pLight->coDiffuse * 1e3f << _T(" ] \"integer nsamples\" [4]")
				<< eol;
			tss << _T("    Translate ") << pLight->vecPosition << eol;
			tss << _T("    Shape \"sphere\" \"float radius\" 0.5") << eol;
			tss << _T("  AttributeEnd") << eol;
			tss << eol;
		}
	}

	TString str = tss.str();

	if (!OpenClipboard())
		return;
	EmptyClipboard();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (str.length() + 1) * sizeof(TCHAR));
	if (!hglbCopy) {
		CloseClipboard();
		return;
	}
	LPTSTR lpstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lpstrCopy, str.c_str(), (str.length() + 1) * sizeof(TCHAR));
	GlobalUnlock(hglbCopy);

	SetClipboardData(CF_UNICODETEXT, hglbCopy);

	CloseClipboard();
}

void CMainWindow::updateSkinParams() {
	m_pRenderer->setSkinParams(m_vps);
}

TString CMainWindow::getParamDescription(float param) {
	TStringStream tss;
	tss << std::setiosflags(std::ios::fixed) << std::setw(4) << std::setprecision(1) << param * 100 << _T("%");
	return tss.str();
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

void CALLBACK CMainWindow::rdLight_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT id = (sender->GetID() - CID_GENERAL_LIGHT_START - CID_GENERAL_RD_LIGHT_SELECT_OFFSET) / CID_GENERAL_LIGHT_STRIDE;

	if (m_prdLights[id]->GetChecked()) {
		m_pCurrentLight = m_pLights[id];
		m_nCurrentLightID = id;
		m_nBlinkStartTick = GetTickCount64();
	}
}

void CALLBACK CMainWindow::sldLight_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT id = (sender->GetID() - CID_GENERAL_LIGHT_START - CID_GENERAL_SLD_LIGHT_INTENSITY_OFFSET) / CID_GENERAL_LIGHT_STRIDE;

	UINT intensity = m_psldLights[id]->GetValue();
	float fint = (float)intensity / 10.0f;

	Light& l = *m_pLights[id];
	l.coDiffuse = l.coSpecular = Color::White * fint;

	TStringStream tss;
	tss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << fint;
	m_plblLights[id]->SetText(tss.str().c_str());
}

void CALLBACK CMainWindow::sldAmbient_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT intensity = m_psldAmbient->GetValue();
	float fint = (float)intensity / 100.0f;

	m_pRenderer->setGlobalAmbient(Color::White * fint);

	TStringStream tss;
	tss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << fint;
	m_plblAmbient->SetText(tss.str().c_str());
}

void CALLBACK CMainWindow::btnDump_Handler(CDXUTControl* sender, UINT nEvent) {
	m_pRenderer->dump();
}

void CALLBACK CMainWindow::sldSSS_f_mel_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT imel = m_psldSSS_f_mel->GetValue();
	float fmel = (float)(imel * imel) / 20000.f;

	m_vps.f_mel = fmel;
	m_plblSSS_f_mel->SetText(getParamDescription(fmel).c_str());

	updateSkinParams();
}

void CALLBACK CMainWindow::sldSSS_f_eu_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT ieu = m_psldSSS_f_eu->GetValue();
	float feu = (float)ieu / 100.f;

	m_vps.f_eu = feu;
	m_plblSSS_f_eu->SetText(getParamDescription(feu).c_str());

	updateSkinParams();
}

void CALLBACK CMainWindow::sldSSS_f_blood_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT iblood = m_psldSSS_f_blood->GetValue();
	float fblood = (float)(iblood * iblood) / 100000.f;

	m_vps.f_blood = fblood;
	m_plblSSS_f_blood->SetText(getParamDescription(fblood).c_str());

	updateSkinParams();
}

void CALLBACK CMainWindow::sldSSS_f_ohg_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT iohg = m_psldSSS_f_ohg->GetValue();
	float fohg = (float)iohg / 100.f;

	m_vps.f_ohg = fohg;
	m_plblSSS_f_ohg->SetText(getParamDescription(fohg).c_str());

	updateSkinParams();
}

void CALLBACK CMainWindow::sldSSSRoughness_Handler(CDXUTControl* sender, UINT nEvent) {
	UINT irough = m_psldSSSRoughness->GetValue();
	float frough = (float)irough / 100.f;

	m_pHead->setRoughness(frough);

	TStringStream tss;
	tss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << frough;
	m_plblSSSRoughness->SetText(tss.str().c_str());
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg) {
	if (m_bChangingView || m_bChangingLight)
		return FALSE;

	// prioritize mouse wheel message handling
	// thus preventing DXUT controls robbing focus
	if (pMsg->message == WM_MOUSEWHEEL) {
		return FALSE;
	}

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
	case 'O':
		m_pRenderer->toggleBloom();
		break;
	case 'L':
		m_nCurrentLightID++;
		m_nCurrentLightID %= NUM_LIGHTS;
		m_pCurrentLight = m_pLights[m_nCurrentLightID];
		m_nBlinkStartTick = GetTickCount64();
		break;
	case VK_F8:
		m_pRenderer->dump();
		break;
	case 'C':
		if (nFlags & MK_CONTROL)
			copyViewAsPBRT();
	}
}

afx_msg void CMainWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
}

afx_msg void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point) {
	if (!m_bChangingView)
		SetCapture();
	m_bChangingLight = true;
	m_ptStart = point;
}

afx_msg void CMainWindow::OnRButtonDown(UINT nFlags, CPoint point) {
	if (!m_bChangingLight)
		SetCapture();
	m_bChangingView = true;
	m_ptStart = point;
}

afx_msg void CMainWindow::OnLButtonUp(UINT nFlags, CPoint point) {
	m_bChangingLight = false;
	if (!m_bChangingView)
		ReleaseCapture();
}

afx_msg void CMainWindow::OnRButtonUp(UINT nFlags, CPoint point) {
	m_bChangingView = false;
	if (!m_bChangingLight)
		ReleaseCapture();
}

afx_msg void CMainWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnMouseMove(UINT nFlags, CPoint point) {
	if (m_bChangingView) {
		CPoint ptDist = (point - m_ptStart);

		double modifier = getViewModifier(nFlags);
		m_camera.changeView(modifier * ptDist.x * Math::PI / 720, modifier * ptDist.y * Math::PI / 720);
	}
	if (m_bChangingLight) {
		Camera c = m_pRenderer->getLightCamera(*m_pCurrentLight);
		CPoint ptDist = (point - m_ptStart);

		double modifier = getViewModifier(nFlags);
		c.changeView(modifier * ptDist.x * Math::PI / 720, modifier * ptDist.y * Math::PI / 720);
		m_pCurrentLight->vecPosition = c.getVecEye();
	}
	if (m_bChangingView || m_bChangingLight)
		m_ptStart = point;
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
