/**
 * App and MainWindow
 */

#pragma once

#include "Config.h"
#include "Camera.h"
#include "RenderableManager.h"
#include "DXUT.h"
#include "DXUTgui.h"
#include <unordered_map>

namespace Skin {
	class CMainWindow;
	class Renderer;
	class Triangle;
	class Head;
	struct Light;

	class CMainApp : public CWinApp {
	private:
		CMainWindow* GetMainWindow();
	public:
		BOOL InitInstance() override; // C++11 explicit override
		BOOL OnIdle(LONG lCount) override;
	};

	class CMainWindow : public CFrameWnd, public RenderableManager {
	private:
		CRect m_rectClient;
		Renderer* m_pRenderer;
		Config m_config;
		Camera m_camera;

		Triangle* m_pTriangle;
		Head* m_pHead;
		Light* m_pLight1;
		Light* m_pLight2;

		// mouse actions
		bool m_bChangingView;
		CPoint m_ptStart;

		static double getViewModifier(UINT nFlags);

		static const double CTRL_MINIFIER;
		static const double SHIFT_MAGNIFIER;

		void showInfo();

		// UI subsystem
		void initUI();
		void uninitUI();

		void initGeneralDialog();

		// Renders UI as a renderable
		class UIRenderable;

		CDXUTDialog* m_pdlgTessellation;
		std::vector<CDXUTDialog**> m_vppdlgs;

		UIRenderable* m_puirGeneral;
		std::vector<UIRenderable**> m_vppuirs;

		static const UINT CID_GENERAL_CHK_TESSELLATION = 0;
		CDXUTCheckBox* m_pchkTessellation;

		// UI Controls Message Mapping
		CDXUTDialogResourceManager* m_pDialogResourceManager;
		void OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl);
		static void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

		typedef void (CALLBACK CMainWindow::*GUIEventHandler)(CDXUTControl* sender, UINT nEvent);
		std::unordered_map<int, GUIEventHandler> m_mapMessages;
		void registerEventHandler(int nControlID, GUIEventHandler handler);
		void unregisterEventHandler(int nControlID);
		void CALLBACK chkTessellation_Handler(CDXUTControl* sender, UINT nEvent);

		// Message mapping
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

		void onCreateDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IDXGISwapChain* pSwapChain) override;
		void onResizedSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
		void onReleasingSwapChain() override;
		void onDestroyDevice() override;
	};

} // namespace Skin
