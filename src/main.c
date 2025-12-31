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
        Pin **items;
        size_t count;
        size_t capacity;
    } targets;
};

struct Nand {
    Vector2 pos;

    Pin inputs[2];
    Pin output;
};

typedef struct Wire Wire;

typedef struct {
    bool isPin;
    Pin *pin;

    // used to know where the connection start if it's connected to a wire
    Vector2 wireConnectionPoint;
} WireConnection;

struct Wire {
    WireConnection src;
    WireConnection target;

    // these are points between the src and target
    struct {
        Vector2 *items;
        size_t count;
        size_t capacity;
    } points;
};

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

Pin CreatePin(Vector2 pos, Nand *parent) {
    Pin pin = {
        .pos = pos,
        .parent = parent,
    };

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

Vector2 GetPinPos(Pin *pin) {
    return Vector2Add(pin->pos, pin->parent->pos);
}

void DrawWire(Wire wire) {
    Vector2 curPoint;

    if(wire.src.isPin) {
        curPoint = GetPinPos(wire.src.pin);
    } else {
        curPoint = wire.src.wireConnectionPoint;
    }

    for(size_t j = 0; j < wire.points.count; j++) {
        Vector2 point = wire.points.items[j];
        DrawLineEx(curPoint, point, 3, BLUE);
        curPoint = point;
    }

    Vector2 endPoint;
    if(wire.target.isPin) {
        endPoint = GetPinPos(wire.target.pin);
    } else {
        endPoint = wire.target.wireConnectionPoint;
    }

    DrawLineEx(curPoint, endPoint, 3, BLUE);
}

void DrawPin(Pin pin) {
    assert(pin.parent != NULL);
    Vector2 pos = GetPinPos(&pin);
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

    Vector2 outPos = GetPinPos(&nand->output);
    for(size_t i = 0; i < nand->output.targets.count; i++) {
        Vector2 targetPos = GetPinPos(nand->output.targets.items[i]);
        DrawLineEx(outPos, targetPos, 3, PINK);
    }

    DrawRectangleRec(rec, RED);
}

void ConnectPin(Pin *outPin, Pin *inPin) {
    if(outPin->targets.capacity == 0) {
        da_init(&outPin->targets, 1);
    }

    da_append(&outPin->targets, inPin);
}

int main(void) {
    InitWindow(1280, 720, "Logic Simulator");
    SetTargetFPS(60);

    Nand *nand_1 = CreateNand(100, 100);
    Nand *nand_2 = CreateNand(300, 100);

    {
        Wire wire = {
            .src = {
                .isPin = true,
                .pin = &nand_1->output,
            },
            .target = {
                .isPin = true,
                .pin = &nand_2->inputs[0],
            },
        };

        da_init(&wire.points, 1);
        da_append(&wire.points, ((Vector2){100 + NAND_WIDTH + 40, 100 + NAND_HEIGHT / 2}));
        da_append(&wire.points, ((Vector2){100 + NAND_WIDTH + 40, 87 + NAND_HEIGHT / 2}));

        da_append(&state.wires, wire);
    }

    {
        Wire wire = {
            .src = {
                .isPin = false,
                .wireConnectionPoint = {100 + NAND_WIDTH + 40, 100 + NAND_HEIGHT / 2}
            },
            .target = {
                .isPin = true,
                .pin = &nand_2->inputs[1],
            },
        };

        da_init(&wire.points, 1);
        da_append(&wire.points, ((Vector2){100 + NAND_WIDTH + 40, 113 + NAND_HEIGHT / 2}));
        da_append(&state.wires, wire);
    }

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        for(size_t i = 0; i < state.wires.count; i++) {
            Wire wire = state.wires.items[i];
            DrawWire(wire);
        }

        // draw wire points
        for(size_t i = 0; i < state.wires.count; i++) {
            Wire wire = state.wires.items[i];

            for(size_t j = 0; j < wire.points.count; j++) {
                DrawCircleV(wire.points.items[j], 5, DARKBLUE);
            }
        }

        for(size_t i = 0; i < state.nands.count; i++) {
            Nand *nand = &state.nands.items[i];
            DrawNand(nand);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
