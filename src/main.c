#include <assert.h>
#include "raylib.h"
#include "raymath.h"

#include "CCFuncs.h"

#define NAND_WIDTH 120
#define NAND_HEIGHT 40
#define PIN_RADIUS 8

typedef struct Nand Nand;

typedef struct {
    Vector2 pos;
    Nand *parent;
} Pin;

struct Nand {
    Vector2 pos;

    Pin inputs[2];
    Pin output;
};

typedef struct {
    struct {
        Nand *items;
        size_t count;
        size_t capacity;
    } nands;
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

Nand *CreateNand(float x, float y) {
    da_append(&state.nands, (Nand){0});
    Nand *nand = &state.nands.items[state.nands.count - 1];

    nand->pos.x = x;
    nand->pos.y = y;

    nand->inputs[0].pos = (Vector2){0, PIN_RADIUS};
    nand->inputs[0].parent = nand;
    nand->inputs[1].pos = (Vector2){0, NAND_HEIGHT - PIN_RADIUS};
    nand->inputs[1].parent = nand;

    nand->output.pos = (Vector2){NAND_WIDTH, NAND_HEIGHT / 2};
    nand->output.parent = nand;

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
