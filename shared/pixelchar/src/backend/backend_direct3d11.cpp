#include <pixelchar/pixelchar.h>
#include <pixelchar/backend/backend_direct3d11.h>

extern "C" {

PixelcharResult _font_backend_direct3d11_add_reference(PixelcharRenderer renderer, uint32_t font_index)
{
	return PIXELCHAR_SUCCESS;
}

void _font_backend_direct3d11_sub_reference(PixelcharRenderer renderer, uint32_t font_index)
{

}

PixelcharResult pixelcharRendererBackendDirect3D11Initialize(PixelcharRenderer renderer)
{
	return PIXELCHAR_SUCCESS;
}

void pixelcharRendererBackendDirect3D11Deinitialize(PixelcharRenderer renderer)
{

}

PixelcharResult pixelcharRendererBackendDirect3D11Render(
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