#ifndef VISUAL_H
#define VISUAL_H

#include "raylib.h"
#include "simulation.h"

typedef struct VisualChip VisualChip;

typedef struct {
    bool isInput;
    Vector2 pos; // relative to parent chip pos

    VisualChip *parentChip;
} VisualPin;

typedef struct {
    VisualPin *items;
    size_t count;
} VisualPinArr;

struct VisualChip {
    uint16_t id;
    ChipType type;
    SimChip *chip;

    // Vector2 pos;
    Rectangle rec;

    VisualPinArr inputPins;
    VisualPinArr outputPins;
};

void VisualNandCreate(Vector2 pos);
void VisualUpdate(void);

#endif // VISUAL_H
