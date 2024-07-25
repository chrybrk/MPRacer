#ifndef __MENU_H__
#define __MENU_H__

#include <stdlib.h>
#include "raylib.h"

void Menu(
	char *menu_items[],
	size_t n,
	int *selected_menu,
	Vector2 start_pos,
	Color background,
	Color text
);

void JoinMenu();

#endif
