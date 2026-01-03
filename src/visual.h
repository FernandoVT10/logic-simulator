#ifndef VISUAL_H
#define VISUAL_H

#include "raylib.h"
#include "simulation.h"

typedef struct {
    bool isInput;
    Vector2 pos; // relative to parent chip pos
} VisualPin;

typedef struct {
    VisualPin *items;
    size_t count;
} VisualPinArr;

typedef struct {
    ChipType type;
    SimChip *chip;

    // Vector2 pos;
    Rectangle rec;

    VisualPinArr inputPins;
    VisualPinArr outputPins;
} VisualChip;

void VisualNandCreate(Vector2 pos);
void VisualUpdate(void);

#endif // VISUAL_H
