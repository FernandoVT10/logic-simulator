#include "visual.h"
#include "CCFuncs.h"

#define VISUAL_PIN_RADIUS 8
#define VISUAL_CHIP_PADDING 20
#define VISUAL_CHIP_FONT_SIZE 40

typedef enum {
    ACTION_DRAGGING,
} ActionType;

typedef struct {
    struct {
        VisualChip *items;
        size_t count;
        size_t capacity;
    } chips;

    ActionType action;
} VisualState;

static VisualState state = {0};

void VisualNandCreate(Vector2 pos) {
    int textWidth = MeasureText("NAND", VISUAL_CHIP_FONT_SIZE);

    VisualChip nand = {
        .type = CHIP_NAND,
        .rec = {
            .x = pos.x,
            .y = pos.y,
            .width = textWidth + VISUAL_CHIP_PADDING * 2,
            .height = VISUAL_CHIP_FONT_SIZE + VISUAL_CHIP_PADDING * 2,
        },
    };

    da_append(&state.chips, nand);
}

static void UpdateNand(VisualChip *chip) {
    Vector2 mousePos = GetMousePosition();
    if(CheckCollisionPointRec(mousePos, chip->rec)) {
    }

    int pad = VISUAL_CHIP_PADDING;
    int fontSize = VISUAL_CHIP_FONT_SIZE;

    DrawRectangleRec(chip->rec, RED);
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
}
