#pragma once
#include "global.h"
#include "ESP.h"
#include <thread>
#include <iostream>
HWND hGameWnd;
HWND hOverlayWnd;
RECT wndRect;

void Update() {
    while (true)
    {
        if (rendering)
        {

            if (GetAsyncKeyState(VK_F1) & 1)
                if (showmenu)
                    Crosshair = !Crosshair;

            if (GetAsyncKeyState(VK_F2) & 1)
                if (showmenu)
                    esp = !esp;

            if (GetAsyncKeyState(VK_F3) & 1)
                if (showmenu)
                    TeamEsp = !TeamEsp;

            if (GetAsyncKeyState(VK_F4) & 1)
                if (showmenu)
                    EnnemiESp = !EnnemiESp;

            if (GetAsyncKeyState(VK_F5) & 1)
                if (showmenu)
                    distanceESp = !distanceESp;

            if (GetAsyncKeyState(VK_F6) & 1)
                if (showmenu)
                    HealthEsp = !HealthEsp;

            if (GetAsyncKeyState(VK_F7) & 1)
                if (showmenu)
                    espLine = !espLine;

            if (GetAsyncKeyState(VK_UP) & 1)
            {
                if (showmenu)
                    if (distanceMax < 3000)
                        distanceMax = distanceMax + 10.f;
            }
            if (GetAsyncKeyState(VK_DOWN) & 1)
            {
                if (showmenu)
                    if (distanceMax > 10)
                        distanceMax = distanceMax - 10.f;
            }

            
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            showmenu = !showmenu;

    }
}

static void render(FOverlay* overlay) {
    while (true) {
        overlay->begin_scene();
        overlay->clear_scene();
        frame++;
        BaseThread2();
        RenderMenu();
        ESPLoop();
        overlay->end_scene();
    }
}

static void _init(FOverlay* overlay) {
    if (!overlay->window_init()) {
        printf("[!] Error init overlay window\n");
        Sleep(5000);
        return;
    }
    else {
        printf("[+] init overlay window\n");

    }

    if (!overlay->init_d2d())
        return;

    std::thread r(render, overlay);
    std::thread up(Update);

    r.join();
    up.detach();
    overlay->d2d_shutdown();
    return;
}

int main()
{
    DRV::Init();
    std::cout << "[+] Waiting For Squad " << std::endl;
    while (!squad->Attach("SquadGame.exe"))
    {
        Sleep(20);
    }

    // ###################################################### I was checking to see if everything was okay ###################################################### \\


    process_base = squad->GetModuleBase();
    std::cout << "[+] Found Squad Base ---> " << "0x" << std::hex << process_base << std::dec << std::endl;

    uWorld = squad->rpm<uintptr_t>(process_base + 0x503b098); // Offsets::GWorld
    std::cout << "[+] Found Uworld = " << "0x" << std::hex << uWorld << std::dec << std::endl;

    gameInstance = squad->rpm<uintptr_t>(uWorld + 0x170); // Offsets::Classes::UWorld::OwningGameInstance
    std::cout << "[+] Found gameInstance = " << "0x" << std::hex << gameInstance << std::dec << std::endl;

    persistentLevel = squad->rpm<uintptr_t>(uWorld + 0x30); // Offsets::Classes::UWorld::PersistentLevel
    std::cout << "[+] Found persistentLevel = " << "0x" << std::hex << persistentLevel << std::dec << std::endl;

    localPlayerPtr = squad->rpm<uintptr_t>(gameInstance + 0x38); // Offsets::Classes::UWorld::UGameInstance::LocalPlayers
    std::cout << "[+] Found LocalPlayerPtr = " << "0x" << std::hex << localPlayerPtr << std::dec << std::endl;

    localPlayer = squad->rpm<uintptr_t>(localPlayerPtr);
    std::cout << "[+] Found LocalPlayer = " << "0x" << std::hex << localPlayer << std::dec << std::endl;

    playerController = squad->rpm<uintptr_t>(localPlayer + 0x30); // Offsets::Classes::UWorld::UGameInstance::UPlayer::PlayerController
    std::cout << "[+] Found playerController = " << "0x" << std::hex << playerController << std::dec << std::endl;

    PlayerStateLocalPlayer = squad->rpm<uintptr_t>(playerController + 0x250);// Offsets::Classes::AActor::UObject::AController::PlayerState 0x250
    std::cout << "[+] Found PlayerStateLocalPlayer = " << "0x" << std::hex << PlayerStateLocalPlayer << std::dec << std::endl;

    LocalTeamId.TeamId = squad->rpm<int>(PlayerStateLocalPlayer + 0x420);// Offsets::Classes::1PlayerState::Ainfo::AActor::UObject::ASQPlayerState::TeamId 0x420
    std::cout << "[+] Found LocalTeamId = " << LocalTeamId.TeamId << std::endl;

    pawn = squad->rpm<uintptr_t>(playerController + 0x2c8); // Offsets::Classes::UWorld::UGameInstance::UPlayer::APlayerController::AcknowledgedPawn
    std::cout << "[+] Found Pawn = " << "0x" << std::hex << pawn << std::dec << std::endl;

    cameraManager = squad->rpm<uintptr_t>(playerController + 0x2e0); //Offsets::Classes::UWorld::UGameInstance::UPlayer::APlayerController::PlayerCameraManager aussi offset_camera_manager
    std::cout << "[+] Found cameraManager = " << "0x" << std::hex << cameraManager << std::dec << std::endl;

    cameraCache = squad->rpm<FCameraCacheEntry>(cameraManager + 0x1A40); // offset_camera_cache
    std::cout << "[+] Found Camera Cache x POV Rotation = " << cameraCache.POV.Rotation.x << std::endl;

    actorsArray = squad->rpm<uintptr_t>(persistentLevel + 0x98); // Offsets::Classes::UWorld::ULevel::UNetConnection::OwningActor
    std::cout << "[+] Found Actor Array = " << "0x" << std::hex << actorsArray << std::dec << std::endl;

    actorsCount = squad->rpm<int>(persistentLevel + 0xa0); // Offsets::Classes::UWorld::ULevel::UNetConnection::MaxPacket
    std::cout << "[+] Found Actor Count = " << actorsCount << std::endl;

    // ###################################################### I was checking to see if everything was okay ###################################################### \\

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    HDC monitor = GetDC(hDesktop);
    int current = GetDeviceCaps(monitor, VERTRES);
    int total = GetDeviceCaps(monitor, DESKTOPVERTRES);
    ScreenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
    ScreenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2;
    g_overlay = { 0 };  
    _init(g_overlay);

}