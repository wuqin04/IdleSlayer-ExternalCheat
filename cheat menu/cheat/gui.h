#pragma once
#include <d3d9.h>
#include <Psapi.h>

namespace gui
{
	// constant window size
	constexpr int HEIGHT{ 500 };
	constexpr int WIDTH{ 800 };

	inline bool EXIT{ true };

	// winapi windows variables
	inline HWND window{ nullptr };
	inline WNDCLASSEXA windowClass{};

	// pos for win movement
	inline POINTS pos{};
	
	// directX state variables
	inline PDIRECT3D9 d3d{ nullptr };
	inline LPDIRECT3DDEVICE9 device{ nullptr };
	inline D3DPRESENT_PARAMETERS presentParameters{};

	// handle window creation and destruction
	void CreateHWin(const char* winName, const char* className) noexcept;
	void DestroyHWin() noexcept;

	// handle device creation and destruction
	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	// handle ImGUI creation and destruction
	void CreateImGUI() noexcept;
	void DestroyImGUI() noexcept;

	// rendering stuff
	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

}