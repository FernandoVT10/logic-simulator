#include "visual.h"
#include "CCFuncs.h"

#define VISUAL_PIN_RADIUS 10
#define VISUAL_CHIP_PADDING 20
#define VISUAL_CHIP_FONT_SIZE 30

typedef enum {
    ACTION_NONE,
    ACTION_DRAGGING,
} ActionType;

typedef struct {
    struct {
        VisualChip *items;
        size_t count;
        size_t capacity;
    } chips;

    struct {
        ActionType type;
        uint16_t chipId;
    } action;
} VisualState;

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

static void DrawPinArr(VisualPinArr arr) {
    if(arr.count == 0) return;

    Rectangle chipRec = arr.items[0].parentChip->rec;

    for(size_t i = 0; i < arr.count; i++) {
        VisualPin pin = arr.items[i];
        Vector2 pos = {
            .x = chipRec.x + pin.pos.x,
            .y = chipRec.y + pin.pos.y,
        };
        DrawCircleV(pos, VISUAL_PIN_RADIUS, DARKGRAY);
    }
}

static void UpdateNand(VisualChip *chip) {
    bool dragging = HandleDragging(chip);

    int pad = VISUAL_CHIP_PADDING;
    int fontSize = VISUAL_CHIP_FONT_SIZE;

    DrawPinArr(chip->inputPins);
    DrawPinArr(chip->outputPins);

    Color color = dragging ? (Color){180, 0, 0, 255} : RED;
    DrawRectangleRec(chip->rec, color);
    DrawText("NAND", chip->rec.x + pad, chip->rec.y + pad, fontSize, WHITE);
}

void VisualUpdate(void) {
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
    }
}
