#include <grrlib.h>
#include <malloc.h>
#include <ogc/system.h>
#include <ogc/lwp_watchdog.h>
#include <stdio.h>
#include <stdint.h>

#include "debug.hpp"
#include "arial_ttf.h"

static GRRLIB_ttfFont *font = NULL;
static u64 lastTime = 0;
static int fps = 0;
static int frameCount = 0;
static bool debugVisible = true;

u64 GetMillisecs() {
    return gettime() / 60750;
}

void InitDebug() {
    font = GRRLIB_LoadTTF(Arial_ttf, Arial_ttf_size);
    lastTime = GetMillisecs();
}

void ToggleDebugVisibility() {
    debugVisible = !debugVisible;
}

void DrawDebugInfo() {
    if (!debugVisible) return;

    u64 currentTime = GetMillisecs();
    frameCount++;

    if (currentTime - lastTime >= 1000) {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }

    u32 memUsed = (u32)((uintptr_t)SYS_GetArena1Hi() - (uintptr_t)SYS_GetArena1Lo());

    char debugText[256];
    snprintf(debugText, sizeof(debugText),
             "FPS: %d Mem: %u KB", 
             fps, memUsed / 1024);

    GRRLIB_PrintfTTF(10, 10, font, debugText, 16, 0x000000FF);
}
