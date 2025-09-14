#include <minec_client.h>

uint32_t menu_gui_create(struct minec_client* client, struct menu_gui_item_create_info* items, uint32_t item_count, struct menu_gui* menu)
{
	menu->item_count = item_count;
	for (uint32_t i = 0; i < item_count; i++);
}

void menu_gui_destroy(struct minec_client* client, struct menu_gui* menu)
{

}