#include "raylib.h"
#include "raymath.h"
#include "include/global.h"
#include "include/client.h"
#include "include/menu.h"
#include "include/physics.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CLEAR_COLOR (Color){ 23, 23, 23 }

Color perPlayer[4] = {
	RED,
	BLUE,
	YELLOW,
	PINK
};

int ID = -1;

bool IsServer = true;

bool Created = false;
bool NotCreatedButJoinEventCalled = false;
bool HasStarted = false;

unsigned int MaxPlayer = 1;

InputBox IPaddrIB;
pthread_t ServerLoop, ServerSend, ClientLoop, ClientRecv;
network_T Server, Client;

void HandleMenu();
void WaitForPlayers();

Player players[4];

packet_T *recvpackets[64 * 1024 * 1024];
int recvpackets_index = 0;

// -- update player movements
void UpdatePlayer(Player *player, float dt);
void UpdateCar(Player *car, float deltaTime);

// -- i haven't decided which camera movement o choose between so i have all the important ones.
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, float delta, int width, int height);
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, float delta, int width, int height);
void UpdateCameraCenter(Camera2D *camera, Player *player, float delta, int width, int height);

void resolve_updates();

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
	
		float dt = GetFrameTime();

		UpdatePlayerPhysics(&players[ID], dt);
		UpdateCameraCenterSmoothFollow(&camera, &players[ID], dt, WINDOW_WIDTH, WINDOW_HEIGHT);

		packet_T packet = { ID, UPDATE, SET_POSITION, { players[ID].position.x, players[ID].position.y }, players[ID].angle };
		net_send(&Client, &packet, sizeof(packet_T), Server.addr);
		// usleep(60);

		resolve_updates();

		BeginDrawing();
		{
			BeginMode2D(camera);
			{
				for (int i = 0; i < MaxPlayer; ++i)
				{
					Player player = players[i];
					Vector2 carOrigin = { player.size.x / 2.0f, player.size.y / 2.0f };
					Rectangle carRect = { player.position.x - carOrigin.x, player.position.y - carOrigin.y, player.size.x, player.size.y };
					DrawRectanglePro(carRect, carOrigin, player.angle, perPlayer[i]);
				}
			}
			EndMode2D();

			DrawFPS(20, 20);
		}
		EndDrawing();
	}

#if __ANDROID__
	pthread_kill(ClientLoop, SIGUSR1);
	if (IsServer) pthread_kill(ServerLoop, SIGUSR1);
#else
	pthread_cancel(ClientLoop);
	if (IsServer) pthread_cancel(ServerLoop);
#endif

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

					if (MaxPlayer > 1 && (CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)))
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

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, float delta, int width, int height)
{
	static float minSpeed = 30;
	static float minEffectLength = 10;
	static float fractionSpeed = 4;

	float halfW = width / 2.0f;
	float halfH = height / 2.0f;
	float halfPW = player->size.x / 2.0f;
	float halfPH = player->size.y / 2.0f;

	camera->offset = (Vector2){ halfW - halfPW, halfH - halfPH };

	Vector2 diff = Vector2Subtract(player->position, camera->target);
	float length = Vector2Length(diff);

	if (length > minEffectLength)
	{
		float speed = fmaxf(fractionSpeed*length, minSpeed);
		camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
	}
}

void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, float delta, int width, int height)
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

void UpdateCameraCenter(Camera2D *camera, Player *player, float delta, int width, int height)
{
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    camera->target = player->position;
}

void resolve_updates()
{
	printf("CLIENT :: total_packets: %d\n", recvpackets_index);
	for (int i = 0; i < recvpackets_index; ++i)
	{
		packet_T *buffer = recvpackets[i];
		if (buffer)
		{
			if (buffer->id != ID)
			{
				players[buffer->id].position.x = buffer->position[0];
				players[buffer->id].position.y = buffer->position[1];
				players[buffer->id].angle = buffer->rotation;
			}
		}
	}

	recvpackets_index = 0;
}

void UpdateCar(Player *car, float deltaTime) {
    const float carAcceleration = 200.0f;
    const float carFriction = 0.02f;
    const float carHandbrakeFriction = 0.1f;
    const float carMaxSpeed = 200.0f;
    const float carMaxAngularVelocity = 300.0f;
    const float turningSpeed = 100.0f;

    Vector2 acceleration = { 0, 0 };

    if (IsKeyDown(KEY_UP)) {
        acceleration.x += cosf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
        acceleration.y += sinf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
    }
    if (IsKeyDown(KEY_DOWN)) {
        acceleration.x -= cosf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
        acceleration.y -= sinf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
    }
    if (IsKeyDown(KEY_LEFT)) {
        car->angularVelocity -= turningSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        car->angularVelocity += turningSpeed * deltaTime;
    }

    // Apply acceleration
    car->velocity.x += acceleration.x;
    car->velocity.y += acceleration.y;

    // Apply friction
    car->velocity.x *= (1 - carFriction);
    car->velocity.y *= (1 - carFriction);

    // Apply handbrake friction if handbrake is engaged
    if (IsKeyDown(KEY_SPACE)) {
        car->velocity.x *= (1 - carHandbrakeFriction);
        car->velocity.y *= (1 - carHandbrakeFriction);
        car->angularVelocity *= 1.1f;  // Increase angular velocity for dramatic handbrake turns
    } else {
        car->angularVelocity *= 0.9f;  // Natural angular deceleration
    }

    // Cap speeds
    if (Vector2Length(car->velocity) > carMaxSpeed) {
        Vector2Scale(car->velocity, carMaxSpeed / Vector2Length(car->velocity));
    }
    if (fabsf(car->angularVelocity) > carMaxAngularVelocity) {
        car->angularVelocity = carMaxAngularVelocity * (car->angularVelocity > 0 ? 1 : -1);
    }

    // Update car position and angle
    car->position.x += car->velocity.x * deltaTime;
    car->position.y += car->velocity.y * deltaTime;
    car->angle += car->angularVelocity * deltaTime;

		printf("%f, %f\n", car->position.x, car->position.y);
}
