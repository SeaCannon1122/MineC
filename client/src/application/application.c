#include "minec_client.h"

#include "string_allocator.h"

uint32_t application_create(struct application_state* app)
{
	uint32_t screen_width = get_screen_width();
	uint32_t screen_height = get_screen_height();

	uint32_t ret = application_window_create(
		&app->main_window,
		screen_width / 4,
		screen_height / 8,
		screen_width / 2,
		screen_height / 2,
		"MineC",
		1
	);

	if (ret != 0) return 1;

	app->string_allocator = string_allocator_new(4096);
}

uint32_t application_handle_events(struct application_state* app)
{
	return application_window_handle_events(&app->main_window);
}

uint32_t application_update_renderer_backends(struct application_state* app)
{

}

uint32_t application_destroy(struct application_state* app)
{
	string_allocator_delete(app->string_allocator);

	application_window_destroy(&app->main_window);

	return 0;
}