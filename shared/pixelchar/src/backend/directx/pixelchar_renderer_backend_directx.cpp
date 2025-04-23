#include "pixelchar_internal.h"

uint32_t pixelchar_renderer_backend_directx_init(
	struct pixelchar_renderer* pcr,
	uint8_t* vertex_shader_custom_source,
	size_t vertex_shader_custom_source_size,
	uint8_t* fragment_shader_custom_source,
	size_t fragment_shader_custom_source_size
)
{
	pcr->backends_initialized |= PIXELCHAR_BACKEND_DIRECTX_BIT;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) if (pcr->fonts[i] != NULL)
		_pixelchar_font_backend_directx_reference_add(pcr->fonts[i], pcr, i);

	return PIXELCHAR_SUCCESS;
}

void pixelchar_renderer_backend_directx_deinit(struct pixelchar_renderer* pcr)
{
	if (pcr->backends_initialized & PIXELCHAR_BACKEND_DIRECTX_BIT == 0) _DEBUG_CALLBACK_ERROR_RETURN_VOID((uint8_t*)"pixelchar_renderer_backend_directx_deinit: directx backend not initialized");

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONTS; i++) if (pcr->fonts[i] != NULL)
		_pixelchar_font_backend_directx_reference_subtract(pcr->fonts[i]);

	pcr->backends_initialized &= (~PIXELCHAR_BACKEND_DIRECTX_BIT);
}

void pixelchar_renderer_backend_directx_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height, float shadow_devisor_r, float shadow_devisor_g, float shadow_devisor_b, float shadow_devisor_a)
{
	if (pcr->backends_initialized & PIXELCHAR_BACKEND_DIRECTX_BIT == 0) _DEBUG_CALLBACK_ERROR_RETURN_VOID((uint8_t*)"pixelchar_renderer_backend_directx_render: directx backend not initialized");

	if (pcr->char_count == 0) return;

	_pixelchar_renderer_render_convert_to_internal_characters(pcr);

	pcr->char_count = 0;
}

