#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "raylib.h"

typedef struct {
	float Acceleration;
	float Deceleration;
	float TurnSpeed;
	float MaxSpeed;
	float Friction;
} CarProperty;

typedef struct {
	CarProperty carProperty;
	Vector2 position;
	Vector2 velocity;
	Vector2 size;
	float angle;
	float angularVelocity;
} Player;

Player InitPlayer(Vector2 position, Vector2 size, CarProperty carProperty);

Vector2 RotateVector(Vector2 vector, float angle);
float GetVectorAngle(Vector2 vector);

void UpdatePlayerPhysics(Player *player, float DeltaTime);

#endif
