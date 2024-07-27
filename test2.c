#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float angle;  // Car's facing angle
    float angularVelocity;
    float steeringAngle;
} Car;

void UpdateCar(Car *car, float deltaTime);
float GetVectorAngle(Vector2 vector);
Vector2 RotateVector(Vector2 vector, float angle);

int main(void) {
    // Initialization
    InitWindow(WIDTH, HEIGHT, "Realistic 2D Car Physics with Raylib");
    SetTargetFPS(60);

    Car car = { (Vector2){ WIDTH / 2.0f, HEIGHT / 2.0f }, (Vector2){ 0, 0 }, 0, 0, 0 };
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

        // Calculate and display angles
        float movementAngle = GetVectorAngle(car.velocity);
        char angleText[50];
        snprintf(angleText, sizeof(angleText), "Facing Angle: %.2f", car.angle);
        DrawText(angleText, 10, 10, 20, BLACK);
        snprintf(angleText, sizeof(angleText), "Movement Angle: %.2f", movementAngle);
        DrawText(angleText, 10, 40, 20, BLACK);

        EndDrawing();
    }

    // De-Initialization
    CloseWindow();

    return 0;
}

void UpdateCar(Car *car, float deltaTime) {
    const float carAcceleration = 300.0f;
    const float carMaxSpeed = 500.0f;
    const float carFriction = 0.98f;
    const float turnSpeed = 5.0f;  // Steering sensitivity
    const float maxSteeringAngle = 30.0f; // Max steering angle in degrees
    const float carLength = 4.0f;  // Distance between front and rear axles

    Vector2 forward = { cosf(DEG2RAD * car->angle), sinf(DEG2RAD * car->angle) };
    Vector2 right = { -sinf(DEG2RAD * car->angle), cosf(DEG2RAD * car->angle) };
    Vector2 acceleration = { 0, 0 };

    // Get user input
    if (IsKeyDown(KEY_UP)) {
        acceleration = Vector2Scale(forward, carAcceleration);
    }
    if (IsKeyDown(KEY_DOWN)) {
        acceleration = Vector2Scale(forward, -carAcceleration);
    }
    if (IsKeyDown(KEY_LEFT)) {
        car->steeringAngle = -maxSteeringAngle;
    } else if (IsKeyDown(KEY_RIGHT)) {
        car->steeringAngle = maxSteeringAngle;
    } else {
        car->steeringAngle = 0;
    }

    // Apply acceleration
    car->velocity = Vector2Add(car->velocity, Vector2Scale(acceleration, GetFrameTime()));

    // Apply friction
    car->velocity.x *= carFriction;
    car->velocity.y *= carFriction;

    // Cap speed
    if (Vector2Length(car->velocity) > carMaxSpeed) {
        car->velocity = Vector2Scale(Vector2Normalize(car->velocity), carMaxSpeed);
    }

    // Calculate the car's new position
    car->position = Vector2Add(car->position, Vector2Scale(car->velocity, GetFrameTime()));

    // Calculate the car's new angle
    float steeringAngleRad = car->steeringAngle * DEG2RAD;
    float velocityMagnitude = Vector2Length(car->velocity);
    float turnRadius = carLength / tanf(steeringAngleRad);

    if (fabs(turnRadius) > 0.01f) {
        car->angle += (velocityMagnitude / turnRadius) * GetFrameTime();
    }

    // Reduce angular velocity gradually for more realistic handling
    car->angularVelocity *= 0.95f;
}

float GetVectorAngle(Vector2 vector) {
    return atan2f(vector.y, vector.x) * RAD2DEG;
}

Vector2 RotateVector(Vector2 vector, float angle) {
    float rad = angle * DEG2RAD;
    float cs = cosf(rad);
    float sn = sinf(rad);
    return (Vector2){ vector.x * cs - vector.y * sn, vector.x * sn + vector.y * cs };
}
