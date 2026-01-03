#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "types.h"

typedef struct SimPin SimPin;
typedef struct SimChip SimChip;

// this is a static array since "capacity" doesn't exist, needed for
// dynamic arrays
typedef struct {
    SimPin *items;
    size_t count;
} SimPinArr;

enum {
    SIM_PIN_OFF = 0,
    SIM_PIN_ON,
};

typedef void (*SimPinOnChange)(SimChip*);

struct SimPin {
    bool isInput;
    SimChip *parentChip;
    uint8_t state;

    // function that will be called on input pins when they are called.
    // Will be used by the chips to update themselves.
    SimPinOnChange onChange;

    struct {
        SimPin **items;
        size_t count;
        size_t capacity;
    } connectedTargets; // for output pin
};

struct SimChip {
    uint16_t id;
    ChipType type;
    SimPinArr inputs;
    SimPinArr outputs;
};

SimChip *SimNandCreate(void);
SimChip *SimLedCreate(void);

void SimSetInputPinState(SimChip *chip, size_t index, uint8_t state);
void SimSetOutputPinState(SimChip *chip, size_t index, uint8_t state);

SimPin *SimGetInputPin(SimChip *chip, size_t index);
SimPin *SimGetOutputPin(SimChip *chip, size_t index);

void SimAddPinConnection(SimPin *outPin, SimPin *inPin);

void SimPrintChip(SimChip *chip);

void SimDestroy(void);

#endif // SIMULATION_H
