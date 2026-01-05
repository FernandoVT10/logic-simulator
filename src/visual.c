#include "raymath.h"
#include "visual.h"
#include "CCFuncs.h"

#define VISUAL_PIN_RADIUS 10
#define VISUAL_CHIP_PADDING 20
#define VISUAL_CHIP_FONT_SIZE 30

static VisualState state = {0};

static uint16_t GenerateId() {
    static uint16_t id = 1;
    return id++;
}

static VisualPinArr AllocPinArr(size_t count) {
    size_t size = count * sizeof(VisualPin);

    VisualPin *items = malloc(size);
    bzero(items, size);

    return (VisualPinArr) {
        .items = items,
        .count = count
    };
}

static VisualPinArr CreatePins(size_t count, VisualChip *parent, bool isInput) {
    VisualPinArr arr = AllocPinArr(count);

    // complex math to just distribute each pin equally
    int numOfInputPins = arr.count;
    int pinRadius = VISUAL_PIN_RADIUS;
    int pinDiameter = pinRadius * 2;
    int separation = (parent->rec.height - pinDiameter * numOfInputPins) / (numOfInputPins + 1);

    for(size_t i = 0; i < arr.count; i++) {
        arr.items[i].isInput = isInput;
        arr.items[i].parentChip = parent;

        if(!isInput) {
            arr.items[i].pos.x = parent->rec.width;
        }

        arr.items[i].pos.y = separation * (i + 1) + pinRadius + pinDiameter * i;
    }

    return arr;
}

void VisualNandCreate(Vector2 pos) {
    int textWidth = MeasureText("NAND", VISUAL_CHIP_FONT_SIZE);

    da_append(&state.chips, ((VisualChip){0}));
    VisualChip *nand = &state.chips.items[state.chips.count - 1];

    nand->id = GenerateId();
    nand->type = CHIP_NAND;
    nand->rec = (Rectangle) {
        .x = pos.x,
        .y = pos.y,
        .width = textWidth + VISUAL_CHIP_PADDING * 2,
        .height = VISUAL_CHIP_FONT_SIZE + VISUAL_CHIP_PADDING * 2,
    };

    nand->inputPins = CreatePins(2, nand, true);
    nand->outputPins = CreatePins(1, nand, false);
}

// returns true if the chip is being dragged
static bool HandleDragging(VisualChip *chip) {
    if(state.action.type == ACTION_NONE) {
        Vector2 mousePos = GetMousePosition();

        if(CheckCollisionPointRec(mousePos, chip->rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            state.action.type = ACTION_DRAGGING;
            state.action.chipId = chip->id;
        }
    }

    bool dragging = state.action.type == ACTION_DRAGGING && state.action.chipId == chip->id;

    if(dragging) {
        Vector2 delta = GetMouseDelta();
        chip->rec.x += delta.x;
        chip->rec.y += delta.y;
    }

    return dragging;
}

static void UpdatePin(VisualPin *pin, Vector2 pinPos) {
    Vector2 mousePos = GetMousePosition();
    if(state.action.type == ACTION_NONE) {
        bool collision = CheckCollisionPointCircle(mousePos, pinPos, VISUAL_PIN_RADIUS);
        if(collision && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            state.action.type = ACTION_WIRING;
            state.action.wiring.points.count = 0;
            state.action.wiring.pin = pin;
        }
    }
}

static void UpdatePinArr(VisualPinArr arr) {
    if(arr.count == 0) return;

    assert(arr.items[0].parentChip != NULL);

    Rectangle chipRec = arr.items[0].parentChip->rec;
    Vector2 parentPos = {chipRec.x, chipRec.y};

    for(size_t i = 0; i < arr.count; i++) {
        VisualPin pin = arr.items[i];
        Vector2 pos = Vector2Add(pin.pos, parentPos);

        UpdatePin(&arr.items[i], pos);

        DrawCircleV(pos, VISUAL_PIN_RADIUS, DARKGRAY);
    }
}

static void UpdateNand(VisualChip *chip) {
    // before updating the pins and therefore being able to wire them
    // we want first to see if we can drag the chip first, this is done
    // since the chip rectangle overlaps the pins
    bool dragging = HandleDragging(chip);

    UpdatePinArr(chip->inputPins);
    UpdatePinArr(chip->outputPins);

    int pad = VISUAL_CHIP_PADDING;
    int fontSize = VISUAL_CHIP_FONT_SIZE;

    Color color = dragging ? (Color){180, 0, 0, 255} : RED;
    DrawRectangleRec(chip->rec, color);
    DrawText("NAND", chip->rec.x + pad, chip->rec.y + pad, fontSize, WHITE);
}

static Vector2 GetPinPos(VisualPin *pin) {
    assert(pin->parentChip != NULL);
    Rectangle chipRec = pin->parentChip->rec;

    return (Vector2){
        .x = pin->pos.x + chipRec.x,
        .y = pin->pos.y + chipRec.y,
    };
}

void VisualUpdate(void) {
    if(state.action.type == ACTION_WIRING) {
        // TODO: check if pin is not NULL
        Vector2 startPos = GetPinPos(state.action.wiring.pin);
        Vector2 mousePos = GetMousePosition();
        DrawLineEx(startPos, mousePos, 3, BLUE);
    }

    for(size_t i = 0; i < state.chips.count; i++) {
        VisualChip *chip = &state.chips.items[i];
        switch(chip->type) {
            case CHIP_NAND: UpdateNand(chip); break;
            case CHIP_LED: assert(false && "TODO");
        }
    }

    if(state.action.type == ACTION_DRAGGING) {
        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            state.action.type = ACTION_NONE;
            state.action.chipId = 0;
        }
    } else if(state.action.type == ACTION_WIRING) {
        // cancel wiring
        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            state.action.type = ACTION_NONE;
            state.action.wiring.pin = NULL;
            state.action.wiring.points.count = 0;
        }
    }
}
