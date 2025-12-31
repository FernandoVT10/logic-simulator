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
    bool isInput;
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

typedef struct {
    Vector2 *items;
    size_t count;
    size_t capacity;
} Points;

struct Wire {
    WireConnection src;
    WireConnection target;

    // these are points between the src and target
    Points points;
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

    bool wiring;
    struct {
        Pin *firstPin;
        Points points;
    } wireData;
} State;

State state = {0};

Pin CreatePin(Vector2 pos, Nand *parent, bool isInput) {
    Pin pin = {
        .isInput = isInput,
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

    nand->inputs[0] = CreatePin((Vector2){0, PIN_RADIUS}, nand, true);
    nand->inputs[1] = CreatePin((Vector2){0, NAND_HEIGHT - PIN_RADIUS}, nand, true);

    nand->output = CreatePin((Vector2){NAND_WIDTH, NAND_HEIGHT / 2}, nand, false);
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


void HandleWiring(Pin *pin) {
    if(state.wiring) {
        if(pin->isInput && state.wireData.firstPin->isInput) return;

        Pin *srcPin, *targetPin;

        if(pin->isInput) {
            srcPin = state.wireData.firstPin;
            targetPin = pin;
        } else {
            srcPin = pin;
            targetPin = state.wireData.firstPin;
        }

        Wire wire = {
            .src = {
                .isPin = true,
                .pin = srcPin,
            },
            .target = {
                .isPin = true,
                .pin = targetPin,
            },
        };

        da_init(&wire.points, state.wireData.points.count);

        for(size_t i = 0; i < state.wireData.points.count; i++) {
            da_append(&wire.points, state.wireData.points.items[i]);
        }

        da_append(&state.wires, wire);

        state.wiring = false;
    } else {
        state.wiring = true;
        state.wireData.firstPin = pin;
        state.wireData.points.count = 0;
    }
}

void UpdatePin(Pin *pin) {
    Vector2 pos = GetPinPos(pin);
    Vector2 mousePos = GetMousePosition();

    if(CheckCollisionPointCircle(mousePos, pos, PIN_RADIUS) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        HandleWiring(pin);
    }
}

void DrawPin(Pin pin) {
    assert(pin.parent != NULL);
    Vector2 pos = GetPinPos(&pin);
    DrawCircleV(pos, PIN_RADIUS, GRAY);
}

void UpdateNand(Nand *nand) {
    Rectangle rec = {
        .x = nand->pos.x,
        .y = nand->pos.y,
        .width = NAND_WIDTH,
        .height = NAND_HEIGHT,
    };

    UpdatePin(&nand->inputs[0]);
    UpdatePin(&nand->inputs[1]);
    UpdatePin(&nand->output);

    DrawPin(nand->inputs[0]);
    DrawPin(nand->inputs[1]);
    DrawPin(nand->output);

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
    (void) nand_1;
    (void) nand_2;

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        for(size_t i = 0; i < state.wires.count; i++) {
            Wire wire = state.wires.items[i];
            DrawWire(wire);
        }

        if(state.wiring) {
            Vector2 curPoint = GetPinPos(state.wireData.firstPin);

            for(size_t i = 0; i < state.wireData.points.count; i++) {
                Vector2 point = state.wireData.points.items[i];
                DrawLineEx(curPoint, point, 3, BLUE);
                curPoint = point;
            }

            Vector2 mousePos = GetMousePosition();
            DrawLineEx(curPoint, mousePos, 3, BLUE);

            for(size_t i = 0; i < state.wireData.points.count; i++) {
                DrawCircleV(state.wireData.points.items[i], 5, DARKBLUE);
            }
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
            UpdateNand(nand);
        }

        // It's important that this is after the nands are updated
        // since we need to know if we're clicking above a pin
        if(state.wiring && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            da_append(&state.wireData.points, mousePos);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
