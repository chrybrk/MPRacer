#include "raylib.h"
#include "include/global.h"
#include "include/client.h"
#include "include/menu.h"

/*
 * Task for Thursday
 *
 * - [x] After you join game, you've a window -> list of players
 * - [x] After everyone joined or more than one person, user can start the game.
 * - [x] It should locate all the players on the map.
 *
 * - [ ] When users event, it should send each req to server (Create a buffer store req in buffer, and create thread to act).
 * - [ ] Resolve eventi on each player side
*/

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CLEAR_COLOR (Color){ 23, 23, 23 }

int ID = -1;

bool IsServer = true;

bool Created = false;
bool NotCreatedButJoinEventCalled = false;
bool HasStarted = false;

unsigned int MaxPlayer = 1;

InputBox IPaddrIB;
pthread_t ServerLoop, ClientLoop;
network_T Server, Client;

void HandleMenu();
void WaitForPlayers();

Rectangle players[4];

int main(void)
{
	InitWindow(800, 600, "title");
	SetTargetFPS(60);

	HandleMenu();
	WaitForPlayers();

	while (!WindowShouldClose())
	{
		ClearBackground(CLEAR_COLOR);
	
		/*
		int key = GetKeyPressed();
		if (key > 0)
		{
			packet_T packet = { ID, SET_KEY, key, { 0, 0 } };
			net_send(&Client, &packet, sizeof(packet_T), Server.addr);
		}
		*/

		if (IsKeyDown(KEY_W))
			players[ID].y += 10;

		if (IsKeyDown(KEY_S))
			players[ID].y -= 10;

		if (IsKeyDown(KEY_A))
			players[ID].x += 10;

		if (IsKeyDown(KEY_D))
			players[ID].x -= 10;

		packet_T packet = { 
			ID, 
			UPDATE, 
			SET_POSITION, 
			{ players[ID].x, players[ID].y }
		};
		net_send(&Client, &packet, sizeof(packet_T), Server.addr);

		BeginDrawing();
		{
			for (int i = 0; i < MaxPlayer; ++i)
			{
				DrawRectangleRec(players[i], RED);
			}

			DrawFPS(20, 20);
		}
		EndDrawing();
	}

	pthread_cancel(ClientLoop);
	if (IsServer) pthread_cancel(ServerLoop);

	return 0;
}

void HandleMenu()
{
	char *menu[] = {
		"Create",
		"Join",
		"Exit"
	};

	int selected_menu = -1;

	IPaddrIB = InitInputBox(
			340, 200,
			240, 50,
			35,
			32
	);

	while ((!Created && !NotCreatedButJoinEventCalled) && !WindowShouldClose())
	{
		ClearBackground(CLEAR_COLOR);
		BeginDrawing();
		{
			Menu(
					menu, 
					sizeof(menu) / 8, 
					&selected_menu, 
					(Vector2){ 150, 400 }, 
					RED, RAYWHITE
			);
		}
		EndDrawing();
	}

	while ((NotCreatedButJoinEventCalled && !Created) && !WindowShouldClose())
	{
		ClearBackground(CLEAR_COLOR);
		BeginDrawing();
		{
			JoinMenu();
		}
		EndDrawing();
	}

	if (!NotCreatedButJoinEventCalled && !Created)
	{
		IsServer = false;

		create_network(8080, IPaddrIB.buffer);
	}

	printf("ID = %d\n", ID);
	MaxPlayer = ID + 1;
}

void WaitForPlayers()
{
	while (!HasStarted && !WindowShouldClose())
	{
		ClearBackground(CLEAR_COLOR);

		DrawText("Waiting for player...", 250, 50, 35, RAYWHITE);

		Vector2 start_pos = { 300, 180 };
		Vector2 box_size = { 200, 50 };

		BeginDrawing();
		{
			for (int i = 0; i < MaxPlayer; i++)
			{
				Rectangle box = { 
							start_pos.x - 10, 
							start_pos.y + (i * 60), 
							box_size.x, 
							box_size.y
				};

				DrawRectangleRounded(box, .3f, 10, RED);
				DrawRectangleRoundedLines(
						(Rectangle){ start_pos.x - 100, start_pos.y - 50, box_size.x * 2, box_size.y * 6.5 },
						.10f, 10, 2.8f, RAYWHITE
				);

				DrawText(TextFormat("Player %d", i + 1), box.x + 20, box.y + 10, 35, RAYWHITE);

				if (ID == 0)
				{
					Rectangle button = { 290, 500, 200, 50 };
					DrawRectangleRounded(button, .3f, 10, RED);
					DrawText("Start", button.x + 50, button.y + 10, 35, RAYWHITE);

					if (MaxPlayer > 1 && CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON))
					{
						packet_T packet = { ID, UPDATE, GAME_HAS_STARTED, { 0, 0 } };
						net_send(&Client, &packet, sizeof(packet_T), Server.addr);
					}
				}
			}
		}
		EndDrawing();
	}
}
