/**
 * App and MainWindow
 */
#include "stdafx.h"

#include "MainWindow.h"

using namespace Skin;

namespace Skin {
	CMainApp app;
} // namespace Skin

CMainWindow* CMainApp::GetMainWindow()
{
	return static_cast<CMainWindow*>(m_pMainWnd);
}

BOOL CMainApp::InitInstance() {
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

CMainWindow::CMainWindow() {
	CSize resolution(800, 600);
	Create(NULL, _APP_NAME_, WS_OVERLAPPEDWINDOW & (~WS_SIZEBOX) & (~WS_MAXIMIZEBOX), 
		CRect(0, 0, resolution.cx, resolution.cy));

	// Adjust window size to exclude border size
	CRect windowRect;
	GetWindowRect(&windowRect);
	GetClientRect(&m_rectClient);
	MoveWindow(windowRect.left, windowRect.top, resolution.cx + windowRect.Width() - m_rectClient.Width(), 
		resolution.cy + windowRect.Height() - m_rectClient.Height());
	GetClientRect(&m_rectClient);
}

CMainWindow::~CMainWindow()
{
	
}

BOOL CMainWindow::OnIdle(LONG lCount) {
	return FALSE;
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

afx_msg void CMainWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
}

afx_msg void CMainWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
}

afx_msg void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnRButtonDown(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnLButtonUp(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnRButtonUp(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
}

afx_msg void CMainWindow::OnMouseMove(UINT nFlags, CPoint point) {
}

afx_msg BOOL CMainWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	return TRUE;
}

afx_msg void CMainWindow::OnClose() {
	CFrameWnd::OnClose();
}

afx_msg void CMainWindow::OnDestroy() {
	CFrameWnd::OnDestroy();
}
