#include "include/physics.h"
#include "raymath.h"
#include <math.h>

Player InitPlayer(Vector2 position, Vector2 size, CarProperty carProperty)
{
	Player player = {
		carProperty,
		position,
		{ 0, 0 },
		size,
		0,
		0	
	};

	return player;
}

float GetVectorAngle(Vector2 vector) {
    return atan2f(vector.y, vector.x) * RAD2DEG;
}

void UpdatePlayerPhysics(Player *player, float deltaTime)
{
	Vector2 acceleration = { 0, 0 };

	if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
			acceleration.x = cosf(DEG2RAD * player->angle) * player->carProperty.Acceleration * deltaTime;
			acceleration.y = sinf(DEG2RAD * player->angle) * player->carProperty.Acceleration * deltaTime;
	}
	if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
			acceleration.x = -cosf(DEG2RAD * player->angle) * player->carProperty.Deceleration * deltaTime;
			acceleration.y = -sinf(DEG2RAD * player->angle) * player->carProperty.Deceleration * deltaTime;
	}

	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
			player->angularVelocity -= player->carProperty.TurnSpeed * deltaTime;
	}
	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
			player->angularVelocity += player->carProperty.TurnSpeed * deltaTime;
	}

#if __ANDROID__
	int buttonSize = 75;
	Vector2 firstSideButtonStartingPosition = { 20, WINDOW_HEIGHT - (buttonSize + 40) };
	char *firstSideButtonText[] = { "Left", "Right" };

	for (int i = 0; i < sizeof(firstSideButtonText) / 8; ++i)
	{
		Rectangle button = { 
			firstSideButtonStartingPosition.x + ((buttonSize + 10) * i), 
			firstSideButtonStartingPosition.y,
			buttonSize, buttonSize
		};

		DrawRectangleRounded(button, .2f, 10, BLACK);
		DrawText(firstSideButtonText[i], button.x + 25, button.y + 30, 15, RAYWHITE);

		if (CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) && i == 0)
			player->angularVelocity -= player->carProperty.TurnSpeed * deltaTime;

		if (CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) && i == 1)
			player->angularVelocity += player->carProperty.TurnSpeed * deltaTime;
	}

	Vector2 secondSideButtonStartingPosition = { WINDOW_WIDTH - 210, WINDOW_HEIGHT - (buttonSize + 40) };
	char *secondSideButtonText[] = { "Down", "Up" };

	for (int i = 0; i < sizeof(secondSideButtonText) / 8; ++i)
	{
		Rectangle button = { 
			secondSideButtonStartingPosition.x + ((buttonSize + 10) * i), 
			secondSideButtonStartingPosition.y,
			buttonSize, buttonSize
		};

		DrawRectangleRounded(button, .2f, 10, BLACK);
		DrawText(secondSideButtonText[i], button.x + 25, button.y + 30, 15, RAYWHITE);

		if (CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) && i == 0)
		{
			acceleration.x = -cosf(DEG2RAD * player->angle) * player->carProperty.Deceleration * deltaTime;
			acceleration.y = -sinf(DEG2RAD * player->angle) * player->carProperty.Deceleration * deltaTime;
		}

		if (CheckMouseOrTouchClicked(button, MOUSE_LEFT_BUTTON) && i == 1)
		{
			acceleration.x = cosf(DEG2RAD * player->angle) * player->carProperty.Acceleration * deltaTime;
			acceleration.y = sinf(DEG2RAD * player->angle) * player->carProperty.Acceleration * deltaTime;
		}
	}
#endif

	// Apply acceleration
	player->velocity.x += acceleration.x;
	player->velocity.y += acceleration.y;

	// Apply friction
	player->velocity.x *= player->carProperty.Friction;
	player->velocity.y *= player->carProperty.Friction;

	// Cap speed
	if (Vector2Length(player->velocity) > player->carProperty.MaxSpeed) {
			player->velocity = Vector2Scale(player->velocity, player->carProperty.MaxSpeed / Vector2Length(player->velocity));
	}

	float speed = sqrt(pow(player->velocity.x, 2) + pow(player->velocity.y, 2));
	float deg = GetVectorAngle(player->velocity);

	// Update player->carProperty. position and angle
	player->position.x += player->velocity.x * deltaTime;
	player->position.y += player->velocity.y * deltaTime;
	player->angle += player->angularVelocity * (speed / player->carProperty.MaxSpeed) * deltaTime;

	// Reduce angular velocity gradually for more realistic handling
	player->angularVelocity *= 0.95f;
}
