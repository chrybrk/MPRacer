#include "raylib.h"
#include "raymath.h"
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

Car players[4];

// -- update player movements
void UpdatePlayer(Car *player, float dt);

// -- i haven't decided which camera movement o choose between so i have all the important ones.
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Car *player, float delta, int width, int height);
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Car *player, float delta, int width, int height);
void UpdateCameraCenter(Camera2D *camera, Car *player, float delta, int width, int height);

int main(void)
{
	InitWindow(800, 600, "title");
	SetTargetFPS(60);

	HandleMenu();
	WaitForPlayers();

	// -- setup 2d camera
	Camera2D camera = { 0 };
	camera.offset = (Vector2){ 0.0f, 0.0f };
	camera.rotation = 0.0f;
	camera.target = (Vector2){ 0, 0 };
	camera.zoom = 1.0f;

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

		float dt = GetFrameTime();

		UpdatePlayer(&players[ID], dt);
		UpdateCameraCenterSmoothFollow(&camera, &players[ID], dt, WINDOW_WIDTH, WINDOW_HEIGHT);

		packet_T packet = { ID, UPDATE, SET_POSITION, { players[ID].position.x, players[ID].position.y }, players[ID].rotation };
		net_send(&Client, &packet, sizeof(packet_T), Server.addr);

		BeginDrawing();
		{
			BeginMode2D(camera);
			{
				for (int i = 0; i < MaxPlayer; ++i)
				{
					Rectangle playerRect = { players[i].position.x, players[i].position.y, players[i].rectSize.x, players[i].rectSize.y };
					DrawRectanglePro(playerRect, players[i].origin, players[i].rotation, RED);
				}
			}
			EndMode2D();

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

void UpdatePlayer(Car *player, float dt)
{
	if (IsKeyDown(KEY_W))
		player->velocity.y += -player->acceleration * dt;
	else if (IsKeyDown(KEY_S))
		player->velocity.y += player->braking * dt;
	else
		player->velocity = Vector2Scale(player->velocity, player->friction);

	if (IsKeyDown(KEY_A))
		player->rotation += player->turnSpeed * (player->velocity.y / player->maxSpeed) * dt;
	else if (IsKeyDown(KEY_D))
		player->rotation -= player->turnSpeed * (player->velocity.y / player->maxSpeed) * dt;

	player->velocity = Vector2ClampValue(player->velocity, 0, player->maxSpeed);
	player->position.x += player->velocity.y * cosf(player->rotation * DEG2RAD);
	player->position.y += player->velocity.y * sinf(player->rotation * DEG2RAD);
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Car *player, float delta, int width, int height)
{
	static float minSpeed = 30;
	static float minEffectLength = 10;
	static float fractionSpeed = 4;

	float halfW = width / 2.0f;
	float halfH = height / 2.0f;
	float halfPW = player->rectSize.x / 2.0f;
	float halfPH = player->rectSize.y / 2.0f;

	camera->offset = (Vector2){ halfW - halfPW, halfH - halfPH };

	Vector2 diff = Vector2Subtract(player->position, camera->target);
	float length = Vector2Length(diff);

	if (length > minEffectLength)
	{
		float speed = fmaxf(fractionSpeed*length, minSpeed);
		camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
	}
}

void UpdateCameraPlayerBoundsPush(Camera2D *camera, Car *player, float delta, int width, int height)
{
    static Vector2 bbox = { 0.2f, 0.2f };

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = (Vector2){ (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (player->position.x < bboxWorldMin.x) camera->target.x = player->position.x;
    if (player->position.y < bboxWorldMin.y) camera->target.y = player->position.y;
    if (player->position.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x);
    if (player->position.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}

void UpdateCameraCenter(Camera2D *camera, Car *player, float delta, int width, int height)
{
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    camera->target = player->position;
}
