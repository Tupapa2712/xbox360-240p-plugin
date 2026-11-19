#pragma once

#include <const/constant.h>

static const enum videoScale { FULLSCREEN=0, SCALE1X, SCALE2X, SCALE2X_ADV,
	SCALE_HQ2X_ALT, 
	SCALE_XBRZ_2X, SCALE_XBRZ_2X_TH, SCALE3X, SCALE3X_ADV, 
	SCALE_HQ3X_ALT, 
	SCALE_XBRZ_3X, SCALE_XBRZ_3X_TH, SCALE4X, 
	SCALE4X_ADV, SCALE_XBRZ_4X, NO_VIDEO, TOTAL_VIDEO_SCALE
};

static const enum videoScaleIntegerType {SCALE_INT_NATIVE=0, SCALE_INT_INCREASE, SCALE_INT_1X, SCALE_INT_2X, SCALE_INT_3X, SCALE_INT_4X, SCALE_INT_5X, TOTAL_INT_SCALE};


static const enum aspectRatio { RATIO_CORE=0, RATIO_4_3, RATIO_3_2, RATIO_8_7, RATIO_10_9,
	RATIO_1_1, RATIO_5_4, RATIO_16_9, RATIO_16_10, RATIO_FILL, TOTAL_VIDEO_RATIO
};

// Resoluciones CRT 240p nativas y super-resoluciones para juegos en Xbox 360.
// El menu frontend (default.xex) arranca siempre en 640x480 (480i legible).
struct t_screen_res { int w, h; };
static const t_screen_res g_screenResolutions[] = {
#ifdef WIN
	{3840, 2160}, {2560, 1440}, {1920, 1080},
#endif
	{320, 240}, {640, 240}, {1280, 240}, {1920, 240}
};
#define TOTAL_SCREEN_RES (int)(sizeof(g_screenResolutions)/sizeof(g_screenResolutions[0]))

/* La lista de shaders ya no es un enum fijo: se descubre en assets\shaders al
 * arrancar (ver src/video/shaderpreset.h). Para recorrerla usa
 * ShaderRegistry::instance()->count() / displayName(i) / idAt(i). */

static const enum ANIM_BACKGROUNDS {BG_TILES, BG_IMAGE, BG_HLSL, BG_HLSL2, BG_HLSL3, BG_NONE, BG_MAX};
static const enum syncOptions {OPT_SYNC_AUDIO = SYNC_TO_AUDIO, OPT_SYNC_VIDEO = SYNC_TO_VIDEO, OPT_SYNC_VIDEO_ADAPT = SYNC_TO_VIDEO_ADAPT, OPT_SYNC_NONE = SYNC_NONE, TOTAL_VIDEO_SYNC};
static const enum SCRAP_GAMES {SCRAP_ALL = 0, SCRAP_NO_METADATA, SCRAP_NO_SCREENSHOT, SCRAP_NO_TITLE, SCRAP_NO_BOX, TOTAL_SCRAP_GAMES};
enum SCRAP_FROM{SC_SCREENCSRAPER, SC_THEGAMESDB, SC_MAX};