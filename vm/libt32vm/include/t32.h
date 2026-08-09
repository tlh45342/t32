#ifndef T32_H
#define T32_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "t32_opcodes.h"

#define T32_REGISTER_COUNT 16u
#define T32_DEFAULT_MEMORY_SIZE (1024u * 1024u)

#define T32_VIDEO_BASE UINT32_C(0x90000000)
#define T32_VIDEO_COLUMNS 80u
#define T32_VIDEO_ROWS 25u
#define T32_VIDEO_SIZE (T32_VIDEO_COLUMNS * T32_VIDEO_ROWS)

/* Keyboard MMIO: polling ASCII input, no IRQ in the first contract. */
#define T32_KEYBOARD_BASE UINT32_C(0x90002000)
#define T32_KEYBOARD_MMIO_SIZE UINT32_C(0x0000000C)
#define T32_KEYBOARD_ID UINT32_C(0x54334B31) /* "T3K1" */
#define T32_KEYBOARD_REG_ID UINT32_C(0x00)
#define T32_KEYBOARD_REG_STATUS UINT32_C(0x04)
#define T32_KEYBOARD_REG_DATA UINT32_C(0x08)
#define T32_KEYBOARD_STATUS_DATA_READY UINT32_C(0x00000001)
#define T32_KEYBOARD_QUEUE_SIZE 64u

/* Platform-control MMIO: guest-requested VM lifecycle actions. */
#define T32_PLATFORM_BASE UINT32_C(0x90004000)
#define T32_PLATFORM_MMIO_SIZE UINT32_C(0x00000010)
#define T32_PLATFORM_ID UINT32_C(0x54335031) /* "T3P1" */
#define T32_PLATFORM_REG_ID UINT32_C(0x00)
#define T32_PLATFORM_REG_STATUS UINT32_C(0x04)
#define T32_PLATFORM_REG_CONTROL UINT32_C(0x08)
#define T32_PLATFORM_REG_RAM_SIZE UINT32_C(0x0C)

#define T32_PLATFORM_STATUS_POWER_REQUESTED UINT32_C(0x00000001)
#define T32_PLATFORM_CONTROL_POWER_OFF UINT32_C(0x00000001)
#define T32_PLATFORM_CONTROL_RESET UINT32_C(0x00000002)



#define T32_DISK_BASE UINT32_C(0x90001000)
#define T32_DISK_ID UINT32_C(0x54334431) /* "T3D1" */
#define T32_DISK_SECTOR_SIZE 512u
#define T32_DISK_DATA_OFFSET UINT32_C(0x00000100)
#define T32_DISK_DATA_BASE (T32_DISK_BASE + T32_DISK_DATA_OFFSET)
#define T32_DISK_MMIO_SIZE (T32_DISK_DATA_OFFSET + T32_DISK_SECTOR_SIZE)

#define T32_DISK_REG_ID UINT32_C(0x00)
#define T32_DISK_REG_STATUS UINT32_C(0x04)
#define T32_DISK_REG_COMMAND UINT32_C(0x08)
#define T32_DISK_REG_LBA UINT32_C(0x0c)
#define T32_DISK_REG_ERROR UINT32_C(0x10)
#define T32_DISK_REG_SECTOR_SIZE UINT32_C(0x14)
#define T32_DISK_REG_SECTOR_COUNT UINT32_C(0x18)

#define T32_DISK_STATUS_ATTACHED UINT32_C(0x00000001)
#define T32_DISK_STATUS_READY UINT32_C(0x00000002)
#define T32_DISK_STATUS_ERROR UINT32_C(0x00000004)

#define T32_DISK_COMMAND_READ UINT32_C(1)
#define T32_DISK_COMMAND_WRITE UINT32_C(2)

#define T32_DISK_ERROR_NONE UINT32_C(0)
#define T32_DISK_ERROR_NO_MEDIA UINT32_C(1)
#define T32_DISK_ERROR_RANGE UINT32_C(2)
#define T32_DISK_ERROR_IO UINT32_C(3)
#define T32_DISK_ERROR_BAD_COMMAND UINT32_C(4)

typedef enum {
    T32_STATE_STOPPED = 0,
    T32_STATE_RUNNING,
    T32_STATE_HALTED,
    T32_STATE_ERROR
} t32_state_t;

typedef enum {
    T32_STEP_OK = 0,
    T32_STEP_HALTED,
    T32_STEP_FAULT
} t32_step_result_t;

typedef struct {
    bool carry;
    bool zero;
    bool negative;
    bool overflow;
} t32_flags_t;

typedef struct t32_machine t32_machine_t;

t32_machine_t *t32_create(size_t memory_size);
void t32_destroy(t32_machine_t *machine);
void t32_reset(t32_machine_t *machine);

bool t32_load_file(
    t32_machine_t *machine,
    const char *path,
    uint32_t address
);

bool t32_read_memory(
    const t32_machine_t *machine,
    uint32_t address,
    void *buffer,
    size_t length
);

bool t32_write_memory(
    t32_machine_t *machine,
    uint32_t address,
    const void *buffer,
    size_t length
);


bool t32_disk_attach(t32_machine_t *machine, const char *path);
void t32_disk_detach(t32_machine_t *machine);
bool t32_disk_is_attached(const t32_machine_t *machine);
uint32_t t32_disk_sector_count(const t32_machine_t *machine);

bool t32_video_is_dirty(const t32_machine_t *machine);
bool t32_keyboard_push(t32_machine_t *machine, uint8_t character);
size_t t32_keyboard_pending(const t32_machine_t *machine);

bool t32_power_off_requested(const t32_machine_t *machine);
bool t32_reset_requested(const t32_machine_t *machine);
void t32_clear_platform_requests(t32_machine_t *machine);
void t32_video_clear_dirty(t32_machine_t *machine);

bool t32_set_register(
    t32_machine_t *machine,
    unsigned register_number,
    uint32_t value
);

uint32_t t32_get_register(
    const t32_machine_t *machine,
    unsigned register_number
);

void t32_set_pc(t32_machine_t *machine, uint32_t pc);
uint32_t t32_get_pc(const t32_machine_t *machine);

t32_flags_t t32_get_flags(const t32_machine_t *machine);
t32_state_t t32_get_state(const t32_machine_t *machine);
const char *t32_state_name(t32_state_t state);
const char *t32_get_halt_reason(const t32_machine_t *machine);
uint64_t t32_get_instruction_count(const t32_machine_t *machine);

void t32_clear_halt(t32_machine_t *machine);

t32_step_result_t t32_step(t32_machine_t *machine);
t32_step_result_t t32_run(
    t32_machine_t *machine,
    uint64_t instruction_limit
);

#endif
