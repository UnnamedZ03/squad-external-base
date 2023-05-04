#pragma once
#include "overlay.h"
#include "driver.h"
#include "struct.h"
#include "d3dx9.h"

DRV* squad = new DRV();

bool Crosshair = false;
bool esp = false;
bool espLine = false;
bool TeamEsp = false;
bool EnnemiESp = false;
bool distanceESp = false;
bool HealthEsp = false;

bool showmenu = true;
bool rendering = true;
int frame = 0;
FOverlay* g_overlay;

float ScreenCenterX;
float ScreenCenterY;

float distanceMax = 1000;

uint64_t process_base = 0;
uint32_t process_id = 0;

