#include <stdio.h>
#include "include/menu.h"
#include "include/utils.h"
#include "include/global.h"
#include "include/server.h"
#include "include/client.h"

bool pressed = true;

void HandleEvent(int selected_menu);
void CreateEvent();
void JoinEventHandle();

#define MAX_CHAR 32
char buffer[MAX_CHAR];

void Menu(
	char *menu_items[],
	size_t n,
	int *selected_menu,
	Vector2 start_pos,
	Color background,
	Color text
)
{
	Vector2 boxSize = { 150, 40 };
	DrawText("Racer", 270, 150, 85, RED);
	for (int i = 0; i < n; i++)
	{
		Rectangle box = { 
					start_pos.x - 10, 
					start_pos.y + (i * 40) - 5, 
					boxSize.x, 
					boxSize.y
		};

		if (CheckCollisionPointRec(GetMousePosition(), box))
			*selected_menu = i;

		if (IsKeyDown(KEY_DOWN) && pressed)
		{
			(*selected_menu)++;
			*selected_menu = *selected_menu % n;
			pressed = false;
		}

		if (IsKeyDown(KEY_UP) && pressed)
		{
			(*selected_menu)--;
			*selected_menu = *selected_menu % n;
			pressed = false;
		}

		if (IsKeyUp(KEY_DOWN) && IsKeyUp(KEY_UP))
			pressed = true;

		if (*selected_menu == i)
		{
			DrawText(menu_items[i], start_pos.x, start_pos.y + (i * 40), 35, background);
			DrawRectangleRoundedLines(
					box,
					0.3f, 
					10,
					0.3f,
					background
			);
		}
		else
			DrawText(menu_items[i], start_pos.x, start_pos.y + (i * 40), 35, text);

		if (*selected_menu >= 0 && IsKeyDown(KEY_ENTER) || (CheckMouseOrTouchClicked(box, MOUSE_LEFT_BUTTON)))
		{
			HandleEvent(*selected_menu);
			return;
		}
	}
}

void HandleEvent(int selected_menu)
{
	switch (selected_menu)
	{
		case 0:
			CreateEvent();
			break;

		case 1:
			NotCreatedButJoinEventCalled = true;
			return;

		case 2:
			exit(0);
			break;
	}
	
	return;
}

void CreateEvent()
{
	Server = init_server(8080, NULL);
	Client = init_client();

	server_bind(&Server);
	client_bind(&Client);

	pthread_create(&ServerLoop, NULL, server_loop, &Server);
	pthread_create(&ServerSend, NULL, server_tick, &Server);

	net_send(
			&Client, 
			(void*)&(packet_T){ ID, JOIN, 0, { 0, 0 } }, 
			sizeof(packet_T), 
			Server.addr
	);

	packet_T *packet = malloc(sizeof(packet_T));
	net_recv(&Client, packet, sizeof(packet_T));

	pthread_create(&ClientLoop, NULL, client_loop, &Client);
	// pthread_create(&ClientRecv, NULL, client_recv, &Client);
	Created = packet->kind == SUCCESS ? true : false;

	if (Created)
		ID = packet->id;

	return;
}

void JoinMenu()
{
	Rectangle button = { 300, 350, 200, 60 };
	DrawText("Enter IP: ", 150, 200, 35, RAYWHITE);
	DrawInputBox(&IPaddrIB, true, true, RAYWHITE, RAYWHITE);

#if __ANDROID__
	ShowKeyboard();
	DrawKeyboard((Vector2){ 100, 250 });

	if (CustomLastKeyInput > 0)
	{
		if (CustomLastKeyInput == KEY_BACKSPACE)
		{
			IPaddrIB.index--;
			if (IPaddrIB.index < 0) IPaddrIB.index = 0;

			IPaddrIB.buffer[IPaddrIB.index] = '\0';
		}
		else if (CustomLastKeyInput == KEY_ENTER)
		{
			NotCreatedButJoinEventCalled = false;
			return;
		}
		else
		{
			IPaddrIB.buffer[IPaddrIB.index] = CustomLastKeyCharInput;
			IPaddrIB.index++;
		}
	}

#else
	DrawRectangleRounded(button, .3f, 10, RED);
	DrawText("Join", 360, 365, 35, RAYWHITE);

	if ((CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) && IPaddrIB.index > 0)
	{
		NotCreatedButJoinEventCalled = false;
		return;
	}
#endif
}
