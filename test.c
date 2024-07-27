#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float angularVelocity;
} Car;

void UpdateCar(Car *car, float deltaTime);
Vector2 RotateVector(Vector2 vector, float angle);
float GetVectorAngle(Vector2 vector);

int main(void) {
    // Initialization
    InitWindow(WIDTH, HEIGHT, "Arcade Style 2D Car Physics with Raylib");
    SetTargetFPS(60);

    Car car = { (Vector2){ WIDTH / 2.0f, HEIGHT / 2.0f }, (Vector2){ 0, 0 }, 0, 0 };
    float carWidth = 50;
    float carHeight = 30;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        UpdateCar(&car, deltaTime);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw car
        Vector2 carOrigin = { carWidth / 2.0f, carHeight / 2.0f };
        Rectangle carRect = { car.position.x - carOrigin.x, car.position.y - carOrigin.y, carWidth, carHeight };
        DrawRectanglePro(carRect, carOrigin, car.angle, BLUE);

        EndDrawing();
    }

    // De-Initialization
    CloseWindow();

    return 0;
}

void UpdateCar(Car *car, float deltaTime) {
    const float carAcceleration = 600.0f;
    const float carDeceleration = 300.0f;
    const float carTurnSpeed = 400.0f;
    const float carMaxSpeed = 300.0f;
    const float carFriction = 0.99f;

    Vector2 acceleration = { 0, 0 };

    // Get user input
    if (IsKeyDown(KEY_UP)) {
        acceleration.x = cosf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
        acceleration.y = sinf(DEG2RAD * car->angle) * carAcceleration * deltaTime;
    }
		if (IsKeyDown(KEY_DOWN)) {
        acceleration.x = -cosf(DEG2RAD * car->angle) * carDeceleration * deltaTime;
        acceleration.y = -sinf(DEG2RAD * car->angle) * carDeceleration * deltaTime;
    }

    if (IsKeyDown(KEY_LEFT)) {
        car->angularVelocity -= carTurnSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        car->angularVelocity += carTurnSpeed * deltaTime;
    }

    // Apply acceleration
    car->velocity.x += acceleration.x;
    car->velocity.y += acceleration.y;

    // Apply friction
    car->velocity.x *= carFriction;
    car->velocity.y *= carFriction;

    // Cap speed
    if (Vector2Length(car->velocity) > carMaxSpeed) {
        car->velocity = Vector2Scale(car->velocity, carMaxSpeed / Vector2Length(car->velocity));
    }

		float speed = sqrt(pow(car->velocity.x, 2) + pow(car->velocity.y, 2));
		float deg = GetVectorAngle(car->velocity);

    // Update car position and angle
    car->position.x += car->velocity.x * deltaTime;
    car->position.y += car->velocity.y * deltaTime;
    car->angle += car->angularVelocity * (speed / carMaxSpeed) * deltaTime;

		printf("velocity = %f, %f, speed = %f\n", car->velocity.x, car->velocity.y, speed);
		printf("angle = %f, deg = %f\n", car->angle, deg);

    // Reduce angular velocity gradually for more realistic handling
    car->angularVelocity *= 0.95f;
}

float GetVectorAngle(Vector2 vector) {
    return atan2f(vector.y, vector.x) * RAD2DEG;
}
