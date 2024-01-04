#pragma once
#include <d3d9.h>

namespace gui
{
	//constant window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	inline bool exit = true;

	//winapi window vars
	inline HWND window = nullptr;
	inline WNDCLASSEXA window_class = { };

	//points for window movement
	inline POINTS window_pos = { 0, 0 };

	//direct x state vars
	inline LPDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 d3d_device = nullptr;
	inline D3DPRESENT_PARAMETERS d3d_params = { };

	// Forward declaration of the create_window function

	void create_window(
		const char* windowName,
		const char* className) noexcept;
	void destroy_window() noexcept;

	//handle device creation & destruction
	void create_device() noexcept;
	void reset_device() noexcept;
	void destroy_device() noexcept;

	//handle ImGui creation & destruction
	void create_gui() noexcept;
	void destroy_gui() noexcept;

	void begin_render() noexcept;
	void end_render() noexcept;
	void render() noexcept;

}
