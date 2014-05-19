
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/**
 * App and MainWindow
 */

#pragma once

#include "Config.h"
#include "Camera.h"
#include "RenderableManager.h"
#include "DXUT.h"
#include "DXUTgui.h"
#include "GaussianParams.h"
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
		int ExitInstance() override;
	};

	class CMainWindow : public CFrameWnd, public RenderableManager {
	private:
		static const UINT NUM_LIGHTS = 2;

		CRect m_rectClient;
		Renderer* m_pRenderer;
		Config m_config;
		Camera m_camera;

		Triangle* m_pTriangle;
		Head* m_pHead;
		Light* m_pLights[NUM_LIGHTS];

		VariableParams m_vps;

		// mouse actions
		bool m_bChangingView;
		bool m_bChangingLight;
		CPoint m_ptStart;
		Light* m_pCurrentLight;
		UINT m_nCurrentLightID;
		ULONGLONG m_nBlinkStartTick;
		static const UINT BLINK_DURATION = 900;
		static const UINT BLINK_INTERVAL = 300;
		static const UINT BLINK_SUB_DURATION = 150;
		static const float BLINK_DIM_FACTOR;

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

		// Utilities
		void copyViewAsPBRT();
		void updateSkinParams();
		static Utils::TString getParamDescription(float param);
		void loadView();
		void loadViewFromFile(const Utils::TString& filename);
		void saveView();
		void saveViewToFile(const Utils::TString& filename);
		void doPerf();

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
		static const UINT CID_GENERAL_LBL_DRAG_LIGHT_CAPTION1 = 107;
		static const UINT CID_GENERAL_LBL_DRAG_LIGHT_CAPTION2 = 108;
		static const UINT CID_GENERAL_LIGHT_START = 109;
		static const UINT CID_GENERAL_LIGHT_STRIDE = 3;
		static const UINT CID_GENERAL_RDGROUP_LIGHT = 0;
		static const UINT CID_GENERAL_RD_LIGHT_SELECT_OFFSET = 0;
		static const UINT CID_GENERAL_SLD_LIGHT_INTENSITY_OFFSET = 1;
		static const UINT CID_GENERAL_LBL_LIGHT_INTENSITY_OFFSET = 2;
		static const UINT CID_GENERAL_LBL_AMBIENT_CAPTION = CID_GENERAL_LIGHT_START + CID_GENERAL_LIGHT_STRIDE * NUM_LIGHTS;
		static const UINT CID_GENERAL_SLD_AMBIENT_INTENSITY = CID_GENERAL_LBL_AMBIENT_CAPTION + 1;
		static const UINT CID_GENERAL_LBL_AMBIENT_INTENSITY = CID_GENERAL_LBL_AMBIENT_CAPTION + 2;
		static const UINT CID_GENERAL_BTN_DUMP = CID_GENERAL_LBL_AMBIENT_CAPTION + 3;
		static const UINT CID_GENERAL_BTN_LOAD_SCENE = CID_GENERAL_LBL_AMBIENT_CAPTION + 4;
		static const UINT CID_GENERAL_BTN_SAVE_SCENE = CID_GENERAL_LBL_AMBIENT_CAPTION + 5;
		static const UINT CID_SSS_LBL_CAPTION = 200;
		static const UINT CID_SSS_CHK_ENABLE_SSS = 201;
		static const UINT CID_SSS_LBL_SSS_STRENGTH_LABEL = 202;
		static const UINT CID_SSS_SLD_SSS_STRENGTH = 203;
		static const UINT CID_SSS_LBL_SSS_STRENGTH = 204;
		static const UINT CID_SSS_CHK_ADAPTIVE_GAUSSIAN = 205;
		static const UINT CID_SSS_LBL_F_MEL_LABEL = 206;
		static const UINT CID_SSS_SLD_F_MEL = 207;
		static const UINT CID_SSS_LBL_F_MEL = 208;
		static const UINT CID_SSS_LBL_F_EU_LABEL = 209;
		static const UINT CID_SSS_SLD_F_EU = 210;
		static const UINT CID_SSS_LBL_F_EU = 211;
		static const UINT CID_SSS_LBL_F_BLOOD_LABEL = 212;
		static const UINT CID_SSS_SLD_F_BLOOD = 213;
		static const UINT CID_SSS_LBL_F_BLOOD = 214;
		static const UINT CID_SSS_LBL_F_OHG_LABEL = 215;
		static const UINT CID_SSS_SLD_F_OHG = 216;
		static const UINT CID_SSS_LBL_F_OHG = 217;
		static const UINT CID_SSS_LBL_ROUGHNESS_LABEL = 218;
		static const UINT CID_SSS_SLD_ROUGHNESS = 219;
		static const UINT CID_SSS_LBL_ROUGHNESS = 220;
		static const UINT CID_SSS_CHK_USELIVEFIT = 221;
		static const UINT CID_SSS_LBL_PROGRESS_LABEL = 222;
		static const UINT CID_SSS_LBL_PROGRESS = 223;

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
		CDXUTRadioButton* m_prdLights[NUM_LIGHTS];
		CDXUTSlider* m_psldLights[NUM_LIGHTS];
		CDXUTStatic* m_plblLights[NUM_LIGHTS];
		CDXUTSlider* m_psldAmbient;
		CDXUTStatic* m_plblAmbient;
		CDXUTButton* m_pbtnDump;
		// SSS dialog
		CDXUTCheckBox* m_pchkEnableSSS;
		CDXUTCheckBox* m_pchkAdaptiveGaussian;
		CDXUTSlider* m_psldSSSStrength;
		CDXUTStatic* m_plblSSSStrength;
		CDXUTSlider* m_psldSSS_f_mel;
		CDXUTStatic* m_plblSSS_f_mel;
		CDXUTSlider* m_psldSSS_f_eu;
		CDXUTStatic* m_plblSSS_f_eu;
		CDXUTSlider* m_psldSSS_f_blood;
		CDXUTStatic* m_plblSSS_f_blood;
		CDXUTSlider* m_psldSSS_f_ohg;
		CDXUTStatic* m_plblSSS_f_ohg;
		CDXUTSlider* m_psldSSSRoughness;
		CDXUTStatic* m_plblSSSRoughness;
		CDXUTCheckBox* m_pchkSSSUseLiveFit;
		CDXUTStatic* m_plblSSSProgress;

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
		void CALLBACK rdLight_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK sldLight_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK sldAmbient_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK btnDump_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK btnLoadScene_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK btnSaveScene_Handler(CDXUTControl* sender, UINT nEvent);
#define DeclareHandlerForParam(param) void CALLBACK sldSSS_f_##param##_Handler(CDXUTControl* sender, UINT nEvent)
		DeclareHandlerForParam(mel);
		DeclareHandlerForParam(eu);
		DeclareHandlerForParam(blood);
		DeclareHandlerForParam(ohg);
		void CALLBACK sldSSSRoughness_Handler(CDXUTControl* sender, UINT nEvent);
		void CALLBACK chkSSSUseLiveFit_Handler(CDXUTControl* sender, UINT nEvent);

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

		void init();
		BOOL OnIdle(LONG lCount);

		void onCreateDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IDXGISwapChain* pSwapChain) override;
		void onResizedSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
		void onReleasingSwapChain() override;
		void onDestroyDevice() override;
	};

} // namespace Skin
