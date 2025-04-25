#pragma once

#ifndef PIXELCHAR_BACKEND_DIRECT3D12_H
#define PIXELCHAR_BACKEND_DIRECT3D12_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

PixelcharResult pixelcharRendererBackendDirect3D12Initialize(PixelcharRenderer renderer);

void pixelcharRendererBackendDirect3D12Deinitialize(PixelcharRenderer renderer);

PixelcharResult pixelcharRendererBackendDirect3D12Render(
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
