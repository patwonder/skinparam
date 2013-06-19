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
	};
} // namespace RLSkin
