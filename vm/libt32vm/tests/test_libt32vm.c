#include "t32.h"

#include <stdint.h>
#include <stdio.h>

static int require_true(int condition, const char *name)
{
    if (!condition) {
        printf("  FAIL %s\n", name);
        return 0;
    }
    printf("  PASS %s\n", name);
    return 1;
}

static void put_u32le(uint8_t bytes[4], uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

int main(void)
{
    t32_machine_t *machine;
    uint8_t bytes[4];
    int ok = 1;

    printf("Running libt32vm smoke validation...\n");

    machine = t32_create(T32_DEFAULT_MEMORY_SIZE);
    ok &= require_true(machine != NULL, "machine created");
    if (!machine)
        return 1;

    ok &= require_true(t32_keyboard_push(machine, (uint8_t)'A'),
                       "keyboard byte accepted");
    ok &= require_true(t32_keyboard_pending(machine) == 1,
                       "keyboard pending count increments");

    ok &= require_true(t32_read_memory(machine, T32_KEYBOARD_BASE +
                       T32_KEYBOARD_REG_STATUS, bytes, sizeof(bytes)),
                       "keyboard STATUS readable");
    ok &= require_true((bytes[0] & 1u) != 0,
                       "keyboard STATUS reports data ready");

    ok &= require_true(t32_read_memory(machine, T32_KEYBOARD_BASE +
                       T32_KEYBOARD_REG_DATA, bytes, sizeof(bytes)),
                       "keyboard DATA readable");
    ok &= require_true(bytes[0] == (uint8_t)'A',
                       "keyboard DATA returns queued ASCII");
    ok &= require_true(t32_keyboard_pending(machine) == 0,
                       "keyboard DATA consumes queued byte");

    put_u32le(bytes, T32_PLATFORM_CONTROL_POWER_OFF);
    ok &= require_true(t32_write_memory(machine, T32_PLATFORM_BASE +
                       T32_PLATFORM_REG_CONTROL, bytes, sizeof(bytes)),
                       "platform POWER_OFF write accepted");
    ok &= require_true(t32_power_off_requested(machine),
                       "platform POWER_OFF request recorded");

    t32_clear_platform_requests(machine);
    ok &= require_true(!t32_power_off_requested(machine),
                       "platform requests clear");

    put_u32le(bytes, T32_PLATFORM_CONTROL_RESET);
    ok &= require_true(t32_write_memory(machine, T32_PLATFORM_BASE +
                       T32_PLATFORM_REG_CONTROL, bytes, sizeof(bytes)),
                       "platform RESET write accepted");
    ok &= require_true(t32_reset_requested(machine),
                       "platform RESET request recorded");

    ok &= require_true(t32_read_memory(machine, T32_PLATFORM_BASE +
                       T32_PLATFORM_REG_RAM_SIZE, bytes, sizeof(bytes)),
                       "platform RAM_SIZE readable");
    ok &= require_true(((uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
                        ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24))
                       == T32_DEFAULT_MEMORY_SIZE,
                       "platform RAM_SIZE reports configured RAM");

    t32_destroy(machine);

    if (!ok)
        return 1;

    printf("libt32vm: PASS (15/15 cases)\n");
    return 0;
}
