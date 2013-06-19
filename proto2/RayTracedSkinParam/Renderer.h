/**
 * Main renderer class
 */

#pragma once

namespace RLSkin {
	class Renderer {
	private:
		// constructor parameters
		HWND m_hwnd;
		CRect m_rectView;

		// OpenRL context
		OpenRLContext m_context;
	public:
		Renderer(HWND hwnd, CRect rectView);
		~Renderer();

		void render();
		void onError(RLenum error, const void* privateData, size_t privateSize, const char* message);
		static void onError(RLenum error, const void* privateData, size_t privateSize, const char* message, void* userData);
	};
} // namespace RLSkin
