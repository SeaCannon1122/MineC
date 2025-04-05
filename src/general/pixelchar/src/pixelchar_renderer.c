#include "pixelchar_internal.h"

#define _CHAR_BUFFER_EXTENSION_SIZE 1024

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render)
{

	pcr->char_buffer = malloc(sizeof(struct pixelchar) * max_chars_to_render);
	if (pcr->char_buffer == NULL) _DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_renderer_create: malloc failed");

	pcr->char_buffer_length = max_chars_to_render;
	pcr->char_count = 0;
	pcr->backend_selected = PIXELCHAR_RENDERER_BACKEND_NONE;

	return PIXELCHAR_SUCCESS;
}

uint32_t pixelchar_renderer_add_font(struct pixelchar_renderer* pcr, struct pixelfont* font, uint32_t index)
{
	if (pcr->backend_selected == PIXELCHAR_RENDERER_BACKEND_NONE) _DEBUG_CALLBACK_CRITICAL_ERROR_RETURN("pixelchar_renderer_render: no backend initialized");


	return pixelchar_renderer_backend_add_font_functions[pcr->backend_selected](pcr, font, index);
}

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count)
{
	uint32_t chars_to_copy_count = 0;

	if (pcr->char_buffer_length - pcr->char_count < char_count)
	{
		_DEBUG_CALLBACK_ERROR("pixelchar_renderer_queue_pixelchars: total queued char count exceeds maximum");
		chars_to_copy_count = pcr->char_buffer_length - pcr->char_count;
	}
	else chars_to_copy_count = char_count;

	if (chars_to_copy_count > 0)
	{
		memcpy(&pcr->char_buffer[pcr->char_count], chars, sizeof(struct pixelchar) * chars_to_copy_count);
		pcr->char_count += chars_to_copy_count;
	}
}

void pixelchar_renderer_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height)
{
	if (pcr->backend_selected == PIXELCHAR_RENDERER_BACKEND_NONE) _DEBUG_CALLBACK_ERROR("pixelchar_renderer_render: no backend initialized");

	else if (pcr->char_count == 0) _DEBUG_CALLBACK_WARNING("pixelchar_renderer_render: queued char count is zero");

	else pixelchar_renderer_backend_render_functions[pcr->backend_selected](pcr, width, height);

	pcr->char_count = 0;
}

void pixelchar_renderer_backend_deinit(struct pixelchar_renderer* pcr)
{
	if (pcr->backend_selected == PIXELCHAR_RENDERER_BACKEND_NONE) _DEBUG_CALLBACK_ERROR("pixelchar_renderer_backend_deinit: no backend initialized");
	else
	{
		pixelchar_renderer_backend_deinit_functions[pcr->backend_selected](pcr);
		pcr->backend_selected = PIXELCHAR_RENDERER_BACKEND_NONE;
	}		
}

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr)
{
	if (pcr->backend_selected != PIXELCHAR_RENDERER_BACKEND_NONE)
	{
		_DEBUG_CALLBACK_WARNING("pixelchar_renderer_destroy: backend should be deinitialized by the user before destroying pixelchar_renderer");
		pixelchar_renderer_backend_deinit(pcr);
	}

	free(pcr->char_buffer);
}