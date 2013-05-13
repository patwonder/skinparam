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

CMainWindow* CMainApp::GetMainWindow()
{
	return static_cast<CMainWindow*>(m_pMainWnd);
}

BOOL CMainApp::InitInstance() {
	//_crtBreakAlloc = 418;
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
	CSize resolution(1280, 720);
	Create(NULL, _APP_NAME_, WS_OVERLAPPEDWINDOW & (~WS_SIZEBOX) & (~WS_MAXIMIZEBOX), 
		CRect(0, 0, resolution.cx, resolution.cy));

	// Adjust window size to exclude border size
	CRect windowRect;
	GetWindowRect(&windowRect);
	GetClientRect(&m_rectClient);
	MoveWindow(windowRect.left, windowRect.top, resolution.cx + windowRect.Width() - m_rectClient.Width(), 
		resolution.cy + windowRect.Height() - m_rectClient.Height());
	GetClientRect(&m_rectClient);

	m_pRenderer = new Renderer(m_hWnd, CRect(0, 0, resolution.cx, resolution.cy), &m_config, &m_camera);
	m_pTriangle = new Triangle();
	m_pHead = new Head();
	//m_pRenderer->addRenderable(m_pTriangle);
	m_pRenderer->addRenderable(m_pHead);

	m_pLight1 = new Light(Vector(3, 7, 0), Color::White * 0.05f, Color::White * 2.0f, Color::White * 1.0f, 1.0f, 0.1f, 0.0f);
	m_pLight2 = new Light(Vector(-4, -2, 5), Color::White * 0.05f, Color::White * 2.0f, Color::White * 1.0f, 1.0f, 0.1f, 0.0f);
	m_pRenderer->addLight(m_pLight1);
	m_pRenderer->addLight(m_pLight2);

	m_bChangingView = false;
	m_camera.restrictView(2.0, 8.0);
}

CMainWindow::~CMainWindow() {
	m_pRenderer->removeAllRenderables();
	m_pRenderer->removeAllLights();

	delete m_pRenderer;
	delete m_pTriangle;
	delete m_pHead;
	delete m_pLight1;
	delete m_pLight2;
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
	return TRUE;
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg) {
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
