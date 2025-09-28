#include <cwindow/cwindow.h>
#include <pixelchar/pixelchar.h>
#include <utils.h>

#include <stdio.h>
#include <math.h>
#include <mutex.h>
#include <malloc.h>

cwindow_context* window_context;
cwindow* window;
uint32_t width;
uint32_t height;
bool leave;

PixelcharFont* default_font;

Pixelchar c[128];

void event_callback(cwindow* window, const cwindow_event* event, struct app* app)
{
	switch (event->type)
	{

	case CWINDOW_EVENT_SIZE: {
		printf(
			"New window dimensions:\n  width: %d\n  height: %d\n  position x: %d\n  position y: %d\n\n",
			event->info.size.width,
			event->info.size.height
		);

		width = event->info.size.width;
		height = event->info.size.height;

	} break;

	case CWINDOW_EVENT_DESTROY: {
		leave = true;
	} break;

	}
}

void pixelchar_callback(uint8_t* message, void* userparam)
{
	printf("%s\n", message);
}

void pixelchar_impl_init();
void pixelchar_impl_frame();
void pixelchar_impl_deinit();

int main()
{
	width = 200;
	height = 200;

	window_context = cwindow_context_create("context");
	window = cwindow_create(window_context, 100, 100, width, height, "pixelchar test", true, event_callback);

	size_t default_font_data_size;
	void* default_font_data = file_load("../../../../../shared/pixelchar/pixelfonts/debug_font/debug.pixelfont", &default_font_data_size);
	if (default_font_data == NULL) printf("failed to load pixelfont\n");

	PixelcharResult res = pixelcharFontLoadFromFileData(default_font_data, default_font_data_size, &default_font, pixelchar_callback, NULL);

	free(default_font_data);

	for (uint32_t i = 0; i < 128; i++)
	{
		pixelcharFill(
			(i % 32 == 0 ? 100 : c[i - 1].position[0] + c[i - 1].width),
			(i == 0 ? 100 : (i % 32 == 0 ? c[i - 1].position[1] + 200 : c[i - 1].position[1])),
			i,
			8,
			PIXELCHAR_MODIFIER_BACKGROUND_BIT | PIXELCHAR_MODIFIER_SHADOW_BIT | PIXELCHAR_MODIFIER_UNDERLINE_BIT,
			0xffdcdcdc,
			i % 2 ? 0xff00ffff : 0xff0000ff,
			default_font,
			&c[i]
		);
	}

	pixelchar_impl_init();

	leave = false;
	while (leave == false)
	{
		cwindow_handle_events(window);

		pixelchar_impl_frame();
	}

	pixelchar_impl_deinit();

	free(default_font);

	cwindow_destroy(window);
	cwindow_context_destroy(window_context);
}