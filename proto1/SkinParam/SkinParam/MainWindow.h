/**
 * App and MainWindow
 */

#pragma once

namespace Skin {
	class CMainWindow;

	class CMainApp : public CWinApp {
	private:
		CMainWindow* GetMainWindow();
	public:
		BOOL InitInstance() override; // C++11 explicit override
		BOOL OnIdle(LONG lCount) override;
	};

	class CMainWindow : public CFrameWnd {
	private:
		CRect m_rectClient;

		BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnDestroy();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnClose();
		DECLARE_MESSAGE_MAP(); /* protected modifier in macro */

	public:
		CMainWindow();
		~CMainWindow();

		BOOL OnIdle(LONG lCount);
	};

} // namespace Skin
