#include "gui.h"
#include <d3d9.h>


#include "../menu/imgui.h"
#include "../menu/imgui_impl_dx9.h"
#include "../menu/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wParam,
		lParam))
		return true;

	switch (message)
	{
	case WM_SIZE:
	{
		if (gui::create_device && wParam != SIZE_MINIMIZED)
		{
			gui::d3d_params.BackBufferWidth = LOWORD(lParam);
			gui::d3d_params.BackBufferHeight = HIWORD(lParam);
			gui::reset_device();
		}
		return 0;
	}

	case WM_SYSCOMMAND:
	{
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		gui::window_pos = MAKEPOINTS(lParam);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		if (wParam & MK_LBUTTON)
		{
			POINTS current_pos = MAKEPOINTS(lParam);
			gui::window_pos.x -= current_pos.x - gui::window_pos.x;
			gui::window_pos.y -= current_pos.y - gui::window_pos.y;
			SetWindowPos(
				window,
				NULL,
				gui::window_pos.x,
				gui::window_pos.y,
				NULL,
				NULL,
				SWP_NOSIZE | SWP_NOZORDER
			);
		}
		return 0;
	}

	default:
		return DefWindowProc(
			window,
			message,
			wParam,
			lParam);
	}return 0;
}

void gui::create_window(
	const char* windowName,
	const char* className) noexcept
{
	window_class.cbSize = sizeof(WNDCLASSEXA);
	window_class.style = CS_CLASSDC;
	window_class.lpfnWndProc = reinterpret_cast<WNDPROC>(WindowProcess);
	window_class.cbClsExtra = 0L;
	window_class.cbWndExtra = 0L;
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.hIcon = NULL;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = className;
	window_class.hIconSm = NULL;

	// Check if the window class registration succeeds
	if (!RegisterClassExA(&window_class))
	{
		MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
		return;
	}

	RegisterClassExA(&window_class);

	window = CreateWindowExA(
		NULL,
		className,
		windowName,
		WS_POPUP | WS_VISIBLE,
		window_pos.x,
		window_pos.y,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		NULL,
		window_class.hInstance,
		NULL);

	if (!gui::window)
	{
		MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
		return;
	}

	ShowWindow(gui::window, SW_SHOWDEFAULT);
	UpdateWindow(gui::window);
}

void gui::destroy_window() noexcept
{
	DestroyWindow(window);
	UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
}

void gui::create_device() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return;

	ZeroMemory(&d3d_params, sizeof(d3d_params));
	d3d_params.Windowed = TRUE;
	d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3d_params.BackBufferFormat = D3DFMT_UNKNOWN;
	d3d_params.EnableAutoDepthStencil = TRUE;
	d3d_params.AutoDepthStencilFormat = D3DFMT_D16;
	d3d_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3d_params,
		&d3d_device) < 0)
	{
		d3d->Release();
		return;
	};

}
void gui::reset_device() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = d3d_device->Reset(&d3d_params);
	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}
void gui::destroy_device() noexcept
{
	if (d3d_device)
	{
		d3d_device->Release();
		d3d_device = nullptr;
	}
	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::create_gui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(d3d_device);

}
void gui::destroy_gui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::begin_render() noexcept
{
	MSG message;
	while (PeekMessage(&message, window, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
void gui::end_render() noexcept
{
	ImGui::EndFrame();

	d3d_device->SetRenderState(D3DRS_ZENABLE, false);
	d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	d3d_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.f, 0);
	
	if (d3d_device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		d3d_device->EndScene();
	}

	const auto result = d3d_device->Present(NULL, NULL, NULL, NULL);

	//handle loss of d3d9 device
	if (result == D3DERR_DEVICELOST && d3d_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		reset_device();
}
bool showWindow = true;  // Add this variable to control window visibility

void gui::render() noexcept
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(800, 600));

	// Use a boolean variable to control window visibility
	if (ImGui::Begin(
		"cheat",
		&showWindow,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::Button("test");
		ImGui::End();
	}
}