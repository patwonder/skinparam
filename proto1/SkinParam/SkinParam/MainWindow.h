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
		void updateUI();
		void initUIDialogs();
		void initDialog(CDXUTDialog* pdlg);
		void initInfoDialog();
		void initGeneralDialog();
		void initSSSDialog();

		// Renders UI as a renderable
		class UIRenderable;

		CDXUTDialog* m_pdlgInfo;
		CDXUTDialog* m_pdlgGeneral;
		CDXUTDialog* m_pdlgSSS;
		std::vector<CDXUTDialog**> m_vppdlgs;

		UIRenderable* m_puirInfo;
		UIRenderable* m_puirGeneral;
		UIRenderable* m_puirSSS;
		std::vector<UIRenderable**> m_vppuirs;

		static const UINT CID_INFO_LBL_SCREEN_SIZE_LABEL = 0;
		static const UINT CID_INFO_LBL_SCREEN_SIZE = 1;
		static const UINT CID_INFO_LBL_FPS_LABEL = 2;
		static const UINT CID_INFO_LBL_FPS = 3;
		static const UINT CID_INFO_LBL_DRIVER_TYPE_LABEL = 4;
		static const UINT CID_INFO_LBL_DRIVER_TYPE = 5;
		static const UINT CID_GENERAL_LBL_CAPTION = 100;
		static const UINT CID_GENERAL_CHK_TESSELLATION = 101;
		static const UINT CID_GENERAL_CHK_BUMP = 102;
		static const UINT CID_GENERAL_CHK_WIREFRAME = 103;
//		static const UINT CID_GENERAL_CHK_VSM_BLUR = 104;
		static const UINT CID_GENERAL_CHK_POST_PROCESS_AA = 105;
		static const UINT CID_GENERAL_CHK_BLOOM = 106;
		static const UINT CID_SSS_LBL_CAPTION = 200;
		static const UINT CID_SSS_CHK_ENABLE_SSS = 201;
		static const UINT CID_SSS_LBL_SSS_STRENGTH_LABEL = 202;
		static const UINT CID_SSS_SLD_SSS_STRENGTH = 203;
		static const UINT CID_SSS_LBL_SSS_STRENGTH = 204;
		static const UINT CID_SSS_CHK_ADAPTIVE_GAUSSIAN = 205;

		// Info dialog
		CDXUTStatic* m_plblScreenSize;
		CDXUTStatic* m_plblFPS;
		CDXUTStatic* m_plblDriverType;
		// General dialog
		CDXUTCheckBox* m_pchkTessellation;
		CDXUTCheckBox* m_pchkBump;
		CDXUTCheckBox* m_pchkWireframe;
		CDXUTCheckBox* m_pchkVSMBlur;
		CDXUTCheckBox* m_pchkPostProcessAA;
		CDXUTCheckBox* m_pchkBloom;
		// SSS dialog
		CDXUTCheckBox* m_pchkEnableSSS;
		CDXUTCheckBox* m_pchkAdaptiveGaussian;
		CDXUTSlider* m_psldSSSStrength;
		CDXUTStatic* m_plblSSSStrength;

		// UI Controls Message Mapping
		CDXUTDialogResourceManager* m_pDialogResourceManager;
		void OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl);
		static void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

		typedef void (CALLBACK CMainWindow::*GUIEventHandler)(CDXUTControl* sender, UINT nEvent);
		std::unordered_map<int, GUIEventHandler> m_mapMessages;
		void registerEventHandler(int nControlID, GUIEventHandler handler);
		void unregisterEventHandler(int nControlID);
#define DeclareHandlerForBool(name) void CALLBACK chk##name##_Handler(CDXUTControl* sender, UINT nEvent)
		DeclareHandlerForBool(Tessellation);
		DeclareHandlerForBool(Bump);
		DeclareHandlerForBool(Wireframe);
//		DeclareHandlerForBool(VSMBlur);
		DeclareHandlerForBool(PostProcessAA);
		DeclareHandlerForBool(Bloom);
		void CALLBACK chkEnableSSS_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK chkAdaptiveGaussian_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK sldSSSStrength_Handler(CDXUTControl* sender, UINT nEvent);

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
