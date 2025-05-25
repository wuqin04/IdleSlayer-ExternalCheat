#include "gui.h"
#include "memory.h"
#include "address.h"
#include "font.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND win,
	UINT message, 
	WPARAM wideParameter,
	LPARAM longParameter
);

LRESULT CALLBACK WinProc(
	HWND win,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
)
{
	if (ImGui_ImplWin32_WndProcHandler(win, message, wideParameter, longParameter))
		return true;

	switch (message)
	{	
		case WM_SIZE:
		{
			if (gui::device && wideParameter != SIZE_MINIMIZED)
			{
				gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
				gui::presentParameters.BackBufferHeight = HIWORD(wideParameter);
				gui::ResetDevice();
			}
		}
		return 0;

		case WM_SYSCOMMAND:
		{
			if ((wideParameter & 0xfff0) == SC_KEYMENU) // disable ALT application menu
				return 0;
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			gui::pos = MAKEPOINTS(longParameter); // set click pos
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			if (wideParameter == MK_LBUTTON)
			{
				const auto points = MAKEPOINTS(longParameter);
				auto rect = ::RECT{};

				GetWindowRect(gui::window, &rect);

				rect.left += points.x - gui::pos.x;
				rect.top += points.y - gui::pos.y;

				if (gui::pos.x >= 0 && gui::pos.x <= gui::WIDTH
					&& gui::pos.y >= 0 && gui::pos.y <= 19)
				{
					SetWindowPos(
						gui::window,
						HWND_TOPMOST,
						rect.left,
						rect.top,
						0, 0,
						SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
		}
		return 0;
	}
	return DefWindowProc(win, message, wideParameter, longParameter);
}



void gui::CreateHWin(const char* winName, const char* className) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WinProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = 0;

	RegisterClassExA(&windowClass);

	window = CreateWindowA(
		className,
		winName,
		WS_POPUP,
		100, 100,
		WIDTH, HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWin() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
	{
		return false;
	}

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
	{
		IM_ASSERT(0);
	}

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGUI() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();
	
	io.IniFilename = NULL;


	// ImGui Theme Color 
	ImGui::StyleColorsDark();
	
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
	
	ImFontConfig fontCfg{};
	fontCfg.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromMemoryTTF(font::queenSideLight, font::queenSideLightSize, 18, &fontCfg);
}

void gui::DestroyImGUI() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message; 
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	 // Start ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);
	
	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"Idle Slayer Cheat Menu v1.0",
		&EXIT,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);


	// implement my gui logic here

	// global variables
	HWND hWnd{};
	DWORD procID{ NULL };
	DWORD handleCount{ NULL };
	DWORD threadCount{ NULL };
	uintptr_t moduleAddress{ 0x0 };

	// game variables
	double coins{ 0 };

	// get win handle
	mem::GetWinHandle(hWnd);

	// get game pid
	procID = mem::GetProcID(hWnd);

	// get process handle
	HANDLE procHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, procID);


	if ( procID == NULL )
	{
		ImGui::OpenPopup("Error ProcID");
	}

	if ( ImGui::BeginPopupModal("Error ProcID", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
	{
		ImGui::Text("The process ID is null, close the cheat menu and open Idle Slayer before opening the cheat.");

		if ( ImGui::Button("Okay") )
		{
			ImGui::CloseCurrentPopup();
			gui::EXIT = false;
		}

		ImGui::EndPopup();
	}
	else
	{
		handleCount = mem::GetHandleCount(procHandle);
		threadCount = mem::GetThreadCount(procID);
		moduleAddress = mem::GetModuleAddress(procID, _T("GameAssembly.dll"));

		if ( moduleAddress == 0x0 )
		{
			ImGui::OpenPopup("Error");
		}

		if ( ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
		{
			ImGui::Text("Finding the Module Address failed. Contact the developer to fix this issue.");
			ImGui::Text("The module address is = %d", moduleAddress);
			ImGui::Separator();
			ImGui::Text("Contact developer via Official Discord server : https://discord.gg/hX2rbH9WGN");

			if ( ImGui::Button("Okay") )
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}



	if ( ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_::ImGuiTabBarFlags_NoTooltip) )
	{

		if ( ImGui::BeginTabItem("General Information") )
		{

			ImGui::SeparatorText("Introduction"); // general information region
			ImGui::Text("Welcome to Idle Slayer External Cheat v1.0");
			ImGui::Text("--Created by Wuqin, 2025");
			ImGui::NewLine();
			ImGui::Text("This tool is intended solely for educational purposes. It is designed to help users gain a deeper understanding");
			ImGui::Text("of reverse engineering, disassembly, and debugging techniques in a practical context. The goal of this");
			ImGui::Text("project is to explore the underlying logic of game manipulation - not to promote or encourage cheating in any form.");
			ImGui::Spacing();
			ImGui::Text("Idle Slayer External Cheat v1.0 is the final release and will not receive future updates or support.");
			ImGui::Text("Use it responsibly and at your own risk.");
			ImGui::NewLine();

			ImGui::SeparatorText("System Context"); // The main region of cheat

			// update the game information
			// display what have store here

			ImGui::Text("> Process Name      : Idle Slayer.exe");
			ImGui::Text("> Process ID             : %d", procID);
			ImGui::Text("> Architecture         : x64 supported only");
			ImGui::Text("> Game Version      : v6.3.1");
			ImGui::Text("> Module Name      : GameAssembly.dll");
			ImGui::Text("> Module Address   : 0x%X", moduleAddress);
			ImGui::Text("> Build Date           : 2025/5/25");
			ImGui::Text("> Threads              : %d", threadCount);
			ImGui::Text("> Handles              : %d", handleCount);
			ImGui::Text("> Window Title      : Idle Slayer");
			ImGui::Text("> Privileges            : Adminstrator");

			ImGui::EndTabItem();
		}

		if ( ImGui::BeginTabItem("Utilities") )
		{
			
			ImGui::SeparatorText("Currency");

			static bool freeze{ false };
			ImGui::Checkbox("Freeze States/Apply Changes", &freeze);

			ImGui::Spacing();

			ImGui::Text("Coins            : ");
			
			ImGui::SameLine();
			static double coinsAmount{ 99999999 };
			ImGui::InputDouble("##Input Double", &coinsAmount);	

			// Read all the memory address here
			uintptr_t coinsPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::coins, address::offset::coins) };
			uintptr_t spPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::slayerPoint, address::offset::slayerPoint) };
			uintptr_t powerUpPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::powerUp, address::offset::powerUp) };
			uintptr_t jewelsPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::jewels, address::offset::jewels) };

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Slayer Points  : ");

			ImGui::SameLine();
			static double spAmount{ 9999 };
			ImGui::InputDouble("##slayer point", &spAmount);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Power Ups    : ");

			ImGui::SameLine();
			static double powerUpAmount{ 999 };
			ImGui::InputDouble("##Power up ", &powerUpAmount);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Jewels         : ");

			ImGui::SameLine();
			static double jewelsAmount{ 99 };
			ImGui::InputDouble("##Jewels", &jewelsAmount);


			if ( freeze )
			{
				WriteProcessMemory(procHandle, (BYTE*)(coinsPtrAddr), &coinsAmount, sizeof(uintptr_t), 0);

				WriteProcessMemory(procHandle, (BYTE*)(spPtrAddr), &spAmount, sizeof(uintptr_t), 0);

				WriteProcessMemory(procHandle, (BYTE*)(powerUpPtrAddr), &powerUpAmount, sizeof(uintptr_t), 0);

				WriteProcessMemory(procHandle, (BYTE*)(jewelsPtrAddr), &jewelsAmount, sizeof(uintptr_t), 0);
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Function Manager");
			ImGui::Spacing();

			ImGui::Text("This category includes cheats discovered during reverse engineering. Please note that some are currently non-functional");
			ImGui::Text("due to the complexity of their implementation (aka, skill issue)");
			ImGui::Spacing();
			ImGui::Separator();

			ImGui::Text("Max Level with One Tap (BROKEN)                                           ");
			ImGui::SameLine(); 

			static bool toggle1TML{ false };
			ImGui::Checkbox("##1Tap Max Level", &toggle1TML);

			ImGui::Spacing();

			ImGui::Text("Auto Quest Completion (BROKEN)                                            ");
			ImGui::SameLine();

			static bool toggleAQC{ false };
			ImGui::Checkbox("##Auto Quest Complete", &toggleAQC);

			ImGui::Spacing();
			
			ImGui::Text("Free Upgrade Purchase (BROKEN)                                            ");
			ImGui::SameLine();

			static bool toggleFUB{ false };
			ImGui::Checkbox("##Free Upgrades Buy", &toggleFUB);

			ImGui::Spacing();

			ImGui::Text("Portal Cooldown Disabled                                                        ");
			ImGui::SameLine();

			static bool togglePortalCD{ false };
			ImGui::Checkbox("##Portal Cooldown", &togglePortalCD);

			uintptr_t protalCDPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::portalCD, address::offset::portalCD) };

			if ( togglePortalCD )
			{
				double noCD{ 0 };

				WriteProcessMemory(procHandle, (BYTE*)(protalCDPtrAddr), &noCD, sizeof(uintptr_t), 0);

			}

			ImGui::Spacing();

			ImGui::Text("Boost Cooldown Disabled (Manual reset needed after portal use)");
			ImGui::SameLine();

			static bool toggleBoostCD{ false };
			ImGui::Checkbox("##Boost Cooldown", &toggleBoostCD);
			ImGui::SameLine();

			uintptr_t boostCDPtrAddr{ mem::Read(procHandle, moduleAddress, address::base::boostCD, address::offset::boostCD) };

			if ( toggleBoostCD )
			{
				float noCD{ 0.5f };

				WriteProcessMemory(procHandle, (BYTE*)(boostCDPtrAddr), &noCD, sizeof(uintptr_t), 0);

			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	
	ImGui::End();
}
