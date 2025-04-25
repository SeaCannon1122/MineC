#pragma once

#ifndef PIXELCHAR_BACKEND_METAL_H
#define PIXELCHAR_BACKEND_METAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

PixelcharResult pixelcharRendererBackendMetalInitialize(PixelcharRenderer renderer);

void pixelcharRendererBackendMetalDeinitialize(PixelcharRenderer renderer);

PixelcharResult pixelcharRendererBackendMetalRender(
	PixelcharRenderer renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);

#ifdef __cplusplus
}
#endif

#endif
