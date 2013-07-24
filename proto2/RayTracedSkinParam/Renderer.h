/**
 * Main renderer class
 */

#pragma once

#include "Program.h"

namespace RLSkin {
	class Renderer {
	private:
		// constructor parameters
		HWND m_hwnd;
		CRect m_rectView;

		// OpenRL context
		OpenRLContext m_rlContext;
		RLtexture m_rlMainTexture;
		RLframebuffer m_rlMainFramebuffer;
		RLbuffer m_rlTempBuffer;

		// Shaders
		std::vector<Shader**> m_vppShaders;
		std::vector<Program**> m_vppPrograms;
		Shader* m_pMainFrameShader;
		Program* m_pMainProgram;

		void initRL();
		void initShaders();
		void uninitShaders();
	public:
		Renderer(HWND hwnd, CRect rectView);
		~Renderer();

		void render();
		void onError(RLenum error, const void* privateData, size_t privateSize, const char* message);
		static void onError(RLenum error, const void* privateData, size_t privateSize, const char* message, void* userData);
	};
} // namespace RLSkin
