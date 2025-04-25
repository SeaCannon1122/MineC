#include <pixelchar/pixelchar.h>
#include <pixelchar/backend/backend_direct3d12.h>

extern "C" {

PixelcharResult _font_backend_direct3d12_add_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	return PIXELCHAR_SUCCESS;
}

void _font_backend_direct3d12_sub_reference(PixelcharRenderer renderer, uint32_t font_index)
{

}

PixelcharResult pixelcharRendererBackendDirect3D12Initialize(PixelcharRenderer renderer)
{

	return PIXELCHAR_SUCCESS;
}

void pixelcharRendererBackendDirect3D12Deinitialize(PixelcharRenderer renderer)
{

}

PixelcharResult pixelcharRendererBackendDirect3D12Render(
	PixelcharRenderer renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
)
{
	return PIXELCHAR_SUCCESS;
}

}