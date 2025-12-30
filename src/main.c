#include <assert.h>
#include "raylib.h"
#include "raymath.h"

#include "CCFuncs.h"

#define NAND_WIDTH 120
#define NAND_HEIGHT 40
#define PIN_RADIUS 8

typedef struct Nand Nand;
typedef struct Pin Pin;

struct Pin {
    Vector2 pos;
    Nand *parent;

    struct {
        Pin *items;
        size_t count;
        size_t capacity;
    } targets;
};

struct Nand {
    Vector2 pos;

    Pin inputs[2];
    Pin output;
};

typedef struct {
    struct {
        Vector2 *items;
        size_t count;
        size_t capacity;
    } points;
} Wire;

typedef struct {
    struct {
        Nand *items;
        size_t count;
        size_t capacity;
    } nands;

    struct {
        Wire *items;
        size_t count;
        size_t capacity;
    } wires;
} State;

State state = {0};

void DrawPin(Pin pin) {
    assert(pin.parent != NULL);
    Vector2 pos = Vector2Add(pin.pos, pin.parent->pos);
    DrawCircleV(pos, PIN_RADIUS, GRAY);
}

void DrawNand(Nand *nand) {
    Rectangle rec = {
        .x = nand->pos.x,
        .y = nand->pos.y,
        .width = NAND_WIDTH,
        .height = NAND_HEIGHT,
    };

    DrawPin(nand->inputs[0]);
    DrawPin(nand->inputs[1]);
    DrawPin(nand->output);

    DrawRectangleRec(rec, RED);
}

Pin CreatePin(Vector2 pos, Nand *parent) {
    Pin pin = {
        .pos = pos,
        .parent = parent,
    };

    da_init(&pin.targets, 1);

    return pin;
}

Nand *CreateNand(float x, float y) {
    da_append(&state.nands, (Nand){0});
    Nand *nand = &state.nands.items[state.nands.count - 1];

    nand->pos.x = x;
    nand->pos.y = y;

    nand->inputs[0] = CreatePin((Vector2){0, PIN_RADIUS}, nand);
    nand->inputs[1] = CreatePin((Vector2){0, NAND_HEIGHT - PIN_RADIUS}, nand);

    nand->output = CreatePin((Vector2){NAND_WIDTH, NAND_HEIGHT / 2}, nand);

    return nand;
}

int main(void) {
    InitWindow(1280, 720, "Logic Simulator");
    SetTargetFPS(60);

    Nand *nand = CreateNand(100, 100);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawNand(nand);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
