#include <string.h>

#include "simulation.h"
#include "CCFuncs.h"

typedef struct {
    struct {
        SimChip *items;
        size_t count;
        size_t capacity;
    } chips;
} SimState;

static SimState state = {0};

static uint16_t GenerateId() {
    static uint16_t id = 1;
    return id++;
}

static SimPinArr AllocPinArr(size_t count) {
    size_t size = count * sizeof(SimPin);

    SimPin *items = malloc(size);
    bzero(items, size);

    return (SimPinArr) {
        .items = items,
        .count = count
    };
}

static SimPinArr CreateInputPinArr(size_t count, SimPinOnChange onChange, SimChip *parentChip) {
    SimPinArr arr = AllocPinArr(count);

    for(size_t i = 0; i < count; i++) {
        arr.items[i].isInput = true;
        arr.items[i].onChange = onChange;
        arr.items[i].parentChip = parentChip;
    }

    return arr;
}

static SimPinArr CreateOutputPinArr(size_t count, SimChip *parentChip) {
    SimPinArr arr = AllocPinArr(count);

    for(size_t i = 0; i < count; i++) {
        arr.items[i].parentChip = parentChip;
    }

    return arr;
}

static uint8_t GetInputState(SimChip *chip, size_t index) {
    assert(index < chip->inputs.count);
    return chip->inputs.items[index].state;
}

static void NandOnChange(SimChip *nand) {
    uint8_t state = !(GetInputState(nand, 0) && GetInputState(nand, 1));
    SimSetOutputPinState(nand, 0, state);
}

SimChip *SimNandCreate(void) {
    da_append(&state.chips, ((SimChip){0}));
    SimChip *nand = &state.chips.items[state.chips.count - 1];

    nand->id = GenerateId();
    nand->type = CHIP_NAND;
    nand->inputs = CreateInputPinArr(2, &NandOnChange, nand);
    nand->outputs = CreateOutputPinArr(1, nand);

    SimSetOutputPinState(nand, 0, SIM_PIN_ON);

    return nand;
}

SimChip *SimLedCreate(void) {
    da_append(&state.chips, ((SimChip){0}));
    SimChip *led = &state.chips.items[state.chips.count - 1];

    led->id = GenerateId();
    led->type = CHIP_LED;
    led->inputs = CreateInputPinArr(1, NULL, led);

    return led;
}

static void SetPinState(SimPin *pin, uint8_t state) {
    pin->state = state;

    if(pin->isInput) {
        if(pin->onChange != NULL) {
            assert(pin->parentChip != NULL);
            pin->onChange(pin->parentChip);
        }
    } else {
        for(size_t i = 0; i < pin->connectedTargets.count; i++) {
            SetPinState(pin->connectedTargets.items[i], state);
        }
    }
}

void SimSetInputPinState(SimChip *chip, size_t index, uint8_t state) {
    assert(index < chip->inputs.count);

    SetPinState(&chip->inputs.items[index], state);
}

void SimSetOutputPinState(SimChip *chip, size_t index, uint8_t state) {
    assert(index < chip->outputs.count);

    SetPinState(&chip->outputs.items[index], state);
}

SimPin *SimGetInputPin(SimChip *chip, size_t index) {
    assert(index < chip->inputs.count);
    return &chip->inputs.items[index];
}
SimPin *SimGetOutputPin(SimChip *chip, size_t index) {
    assert(index < chip->outputs.count);
    return &chip->outputs.items[index];
}

void SimAddPinConnection(SimPin *outPin, SimPin *inPin) {
    assert(!outPin->isInput && inPin->isInput);

    if(outPin->connectedTargets.capacity == 0) {
        // initialize array with a capacity of 1
        da_init(&outPin->connectedTargets, 1);
    }

    da_append(&outPin->connectedTargets, inPin);

    SetPinState(inPin, outPin->state);
}

void SimPrintChip(SimChip *chip) {
    const char *chipName = chip->type == CHIP_NAND ? "NAND" : "LED";

    printf("[%s] (#%d) {\n", chipName, chip->id);

    printf("  [Inputs] {\n");
    for(size_t i = 0; i < chip->inputs.count; i++) {
        printf("    [%lu] = %d\n", i, chip->inputs.items[i].state);
    }
    printf("  }\n");

    printf("  [Ouputs] {\n");
    for(size_t i = 0; i < chip->outputs.count; i++) {
        printf("    [%lu] = %d\n", i, chip->outputs.items[i].state);
    }
    printf("  }\n");

    printf("}\n");
}

void SimDestroy(void) {
    for(size_t i = 0; i < state.chips.count; i++) {
        SimChip chip = state.chips.items[i];

        if(chip.inputs.items != NULL) free(chip.inputs.items);
        if(chip.outputs.items != NULL) free(chip.outputs.items);
    }

    da_free(&state.chips);
}

// TODO: it could be a good idea to remove all asserts and instead print an error message
