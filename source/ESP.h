#include "global.h"
#include <cmath>
// ###################################################### Some useful stuff (I was lazy) ###################################################### \\

HWND hwnd = NULL;
HWND hwnd_active = NULL;
HWND OverlayWindow = NULL;
auto CrosshairColor = D2D1::ColorF(0, 100, 255, 255);
#define M_PI 3.14159265358979323846264338327950288419716939937510


struct FMinimalViewInfo
{
	Vector3 Location;
	Vector3 Rotation;
	float FOV;
};
struct FCameraCacheEntry
{
	char pad_0x0[0x10];
	FMinimalViewInfo POV;
};

class UPlayer {
private:
public:
	uintptr_t instance;
	uintptr_t mesh;
	uintptr_t root_component;
	uintptr_t instigator;
	uintptr_t PlayerState;
	std::string name;
	int objectId;
	Vector3 origin;
	float health;
	float dist;
	int team;
};

class APlayerState
{
private:
public:
	uintptr_t SoldiersOnScreenSortedByNearestDistanceCache;// 0x0488
	int TeamId;// 0x0420 
};

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}


FCameraCacheEntry cameraCache;
uintptr_t uWorld;
uintptr_t gameInstance;
uintptr_t persistentLevel;
uintptr_t localPlayerPtr;
uintptr_t localPlayer;
uintptr_t playerController;
uintptr_t pawn;
uintptr_t cameraManager;
uintptr_t actorsArray;
uintptr_t PlayerStateLocalPlayer;
uintptr_t PlayerState;
APlayerState LocalTeamId;
int actorsCount;

static std::string GetNameById(uint32_t actor_id) {
	char name[256];

	uint32_t chunk_offset = actor_id >> 16;
	uint16_t name_offset = (uint16_t)actor_id;
	uintptr_t fname_pool = process_base + 0x4f1df40;

	uintptr_t name_pool_chunk = squad->rpm<uintptr_t>(fname_pool + ((chunk_offset + 2) * 8));
	if (name_pool_chunk) {
		uintptr_t entry_offset = name_pool_chunk + (uint32_t)(2 * name_offset);
		if (entry_offset) {

			uint16_t name_entry = squad->rpm<uint16_t>(entry_offset);

			uint32_t name_length = (name_entry >> 6);

			if (name_length > 256)
			{
				name_length = 255;
			}

			auto result = squad->ReadRaw(entry_offset + 0x2, &name, name_length);
			return name;


		}
	}
	return std::string("NULL");
}

bool WorldToScreenX(Vector3 WorldLocation, FMinimalViewInfo CameraCacheL, Vector3& Screenlocation)
{

	auto POV = CameraCacheL;
	Vector3 Rotation = POV.Rotation;
	D3DMATRIX tempMatrix = Matrix(Rotation);

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - POV.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		return false;

	float FovAngle = POV.FOV;

	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return true;
}

// ###################################################### ESP Features ###################################################### \\

void ESPLoop() {

	hwnd = FindWindowA("UnrealWindow", NULL); // Target Window With his lpClassName ( UnrealWindow )
	OverlayWindow = FindWindow("CEF-OSC-WIDGET", "NVIDIA GeForce Overlay");
	hwnd_active = GetForegroundWindow();

	if (hwnd_active == hwnd) {

		ShowWindow(OverlayWindow, SW_SHOW);
	}
	else
	{
		ShowWindow(OverlayWindow, SW_HIDE);
	}

	if (Crosshair)
	{
		g_overlay->draw_line(ScreenCenterX - 15, ScreenCenterY - 15, ScreenCenterX + 15, ScreenCenterY + 15, CrosshairColor);
		g_overlay->draw_line(ScreenCenterX - 15, ScreenCenterY + 15, ScreenCenterX + 15, ScreenCenterY - 15, CrosshairColor);

	}
}

void RenderMenu()
{
	if (showmenu && rendering)
	{
		g_overlay->draw_text(5, 5, D2D1::ColorF(255, 20, 20, 255), "SHOW/HIDE [INSERT]");

		if (Crosshair)
			g_overlay->draw_text(5, 80, D2D1::ColorF(0, 255, 0, 255), "F1 Crosshair : ON");
		else
			g_overlay->draw_text(5, 80, D2D1::ColorF(255, 0, 0, 255), "F1 Crosshair : OFF");

		if (esp)
			g_overlay->draw_text(5, 100, D2D1::ColorF(0, 255, 0, 255), "F2 Esp : ON");
		else
			g_overlay->draw_text(5, 100, D2D1::ColorF(255, 0, 0, 255), "F2 Esp : OFF");

		if (TeamEsp)
			g_overlay->draw_text(5, 120, D2D1::ColorF(0, 255, 0, 255), "F3 TeamEsp : ON");
		else
			g_overlay->draw_text(5, 120, D2D1::ColorF(255, 0, 0, 255), "F3 TeamEsp : OFF");

		if (EnnemiESp)
			g_overlay->draw_text(5, 140, D2D1::ColorF(0, 255, 0, 255), "F4 Ennemi ESP : ON");
		else
			g_overlay->draw_text(5, 140, D2D1::ColorF(255, 0, 0, 255), "F4 Ennemi ESP : OFF");

		if (distanceESp)
			g_overlay->draw_text(5, 160, D2D1::ColorF(0, 255, 0, 255), "F5 Distance ESP : ON");
		else
			g_overlay->draw_text(5, 160, D2D1::ColorF(255, 0, 0, 255), "F5 Distance ESP : OFF");

		if (HealthEsp)
			g_overlay->draw_text(5,180, D2D1::ColorF(0, 255, 0, 255), "F6 Health ESP : ON");
		else
			g_overlay->draw_text(5, 180, D2D1::ColorF(255, 0, 0, 255), "F6 Health ESP : OFF");

		if (espLine)
			g_overlay->draw_text(5, 200, D2D1::ColorF(0, 255, 0, 255), "F7 ESPLine : ON");
		else
			g_overlay->draw_text(5, 200, D2D1::ColorF(255, 0, 0, 255), "F7 ESPLine : OFF");

		std::string gg = std::to_string(int(distanceMax));
		const char* ggT = gg.c_str();

		g_overlay->draw_text(5, 220, D2D1::ColorF(0, 255, 0, 255), "UP/Down DistanceMax = ");
		g_overlay->draw_text(160, 220, D2D1::ColorF(0, 0, 255, 255), ggT);

	}
}


inline PVOID BaseThread2() 
{
	if (esp)
	{
		uWorld = squad->rpm<uintptr_t>(process_base + 0xbc2ffa0); // Offsets::GWorld /
		gameInstance = squad->rpm<uintptr_t>(uWorld + 0x190); // Offsets::Classes::UWorld::OwningGameInstance /
		persistentLevel = squad->rpm<uintptr_t>(uWorld + 0x38); // Offsets::Classes::UWorld::PersistentLevel /
		localPlayerPtr = squad->rpm<uintptr_t>(gameInstance + 0x40); // Offsets::Classes::UWorld::UGameInstance::LocalPlayers /
		localPlayer = squad->rpm<uintptr_t>(localPlayerPtr);  // LocalPlayer lol /
		playerController = squad->rpm<uintptr_t>(localPlayer + 0x38); // Offsets::Classes::UWorld::UGameInstance::UPlayer::PlayerController /
		PlayerStateLocalPlayer = squad->rpm<uintptr_t>(playerController + 0x238);// Offsets::Classes::AActor::UObject::AController::PlayerState /
		LocalTeamId.TeamId = squad->rpm<int>(PlayerStateLocalPlayer + 0x420);// Offsets::Classes::1PlayerState::Ainfo::AActor::UObject::ASQPlayerState::TeamId 0x420
		pawn = squad->rpm<uintptr_t>(playerController + 0x2b8); // Offsets::Classes::UWorld::UGameInstance::UPlayer::APlayerController::AcknowledgedPawn /
		cameraManager = squad->rpm<uintptr_t>(playerController + 0x2d0); //Offsets::Classes::UWorld::UGameInstance::UPlayer::APlayerController::PlayerCameraManager aussi offset_camera_manager
		cameraCache = squad->rpm<FCameraCacheEntry>(cameraManager + 0x2a0); // offset_camera_cache
		actorsArray = squad->rpm<uintptr_t>(persistentLevel + 0xa0); // Offsets::Classes::UWorld::ULevel::UNetConnection::OwningActor
		actorsCount = squad->rpm<int>(persistentLevel + 0xa8); // Offsets::Classes::UWorld::ULevel::UNetConnection::MaxPacket

		for (int i = 0; i < actorsCount; i++) 
		{
			uintptr_t actor = squad->rpm<uintptr_t>(actorsArray + i * 0x8);

			if (!actor)
				continue;

			UPlayer player;

			player.instance = actor;
			player.objectId = squad->rpm<int>(actor + 0x18); // offset_actor_id

			if (GetNameById(player.objectId).find("BP_Soldier") != std::string::npos)
			{
				player.instigator = squad->rpm<uintptr_t>(player.instance + 0x148); // AActor::Instigator 
				player.PlayerState = squad->rpm<uintptr_t>(player.instigator + 0x268); // APawn::PlayerState
				player.team = squad->rpm<int>(player.PlayerState + 0x420); // ASQPlayerState::Teamid
				player.root_component = squad->rpm<uintptr_t>(player.instance + 0x160); // AActor::RootComponent
				player.origin = squad->rpm<Vector3>(player.root_component + 0x144); // USceneComponent::RelativeLocation

				// Distance 
				float dist = cameraCache.POV.Location.DistTo(player.origin); 
				float DistM = ToMeters(dist);

				if (DistM > distanceMax)
					continue;

				player.health = squad->rpm<float>(actor + 0x1c18); //AActor::Uobject::ASQSolider::Health 0x1c18

				// i know, its ugly.
				std::string healthT = std::to_string(int(player.health));
				const char* healthText = healthT.c_str();
				std::string distT = std::to_string(int(DistM));
				const char* DistT = distT.c_str();

				//EnnemieESP
				if (player.health > 0 && player.team != LocalTeamId.TeamId && EnnemiESp)
				{
					Vector3 Screen;
					if (WorldToScreenX(player.origin, cameraCache.POV, Screen))
					{

						if(espLine)
							g_overlay->draw_line(ScreenCenterX, 1080, Screen.x, Screen.y, D2D1::ColorF(255, 0, 0, 255));
						if(HealthEsp)
							g_overlay->draw_text(Screen.x - 15, Screen.y, D2D1::ColorF(0, 255, 0, 255), healthText);
						if(distanceESp)
							g_overlay->draw_text(Screen.x - 15, Screen.y - 10, D2D1::ColorF(255, 0, 0, 255), DistT);
					}
				}

				//TeamESP
				if (player.team == LocalTeamId.TeamId && player.health > 0 && TeamEsp)
				{
					Vector3 Screen;
					if (WorldToScreenX(player.origin, cameraCache.POV, Screen))
					{

						if (espLine)
							g_overlay->draw_line(ScreenCenterX, 1080, Screen.x, Screen.y, D2D1::ColorF(0, 0, 255, 255));
						if (HealthEsp)
							g_overlay->draw_text(Screen.x - 15, Screen.y, D2D1::ColorF(0, 255, 0, 255), healthText);
						if (distanceESp)
							g_overlay->draw_text(Screen.x - 15, Screen.y - 10, D2D1::ColorF(0, 0, 255, 255), DistT);
					}
				}
			}
			else
				continue;
		}

	}
	return 0;
}