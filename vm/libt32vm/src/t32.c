/*
 * t32.c
 *
 * Reference execution core for the canonical T32 ISA.
 *
 * Instruction word:
 *   bits 31..24  opcode
 *   bits 23..20  destination register (rd)
 *   bits 19..16  source/address register A (ra)
 *   bits 15..12  source/value register B (rb)
 *   bits 11..0   reserved
 *
 * Immediate and target forms consume one additional little-endian word.
 */

#include "t32.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define T32_STACK_POINTER 15u
#define T32_CPUID_VALUE 0x54333201u /* "T32", implementation revision 1 */

struct t32_machine {
    uint32_t registers[T32_REGISTER_COUNT];
    uint32_t pc;
    t32_flags_t flags;

    uint8_t *memory;
    size_t memory_size;

    uint8_t video_memory[T32_VIDEO_SIZE];
    bool video_dirty;

    uint8_t keyboard_queue[T32_KEYBOARD_QUEUE_SIZE];
    size_t keyboard_head;
    size_t keyboard_tail;
    size_t keyboard_count;

    bool platform_power_off_requested;
    bool platform_reset_requested;

    FILE *disk_file;
    uint32_t disk_sector_count;
    uint32_t disk_lba;
    uint32_t disk_error;
    uint8_t disk_buffer[T32_DISK_SECTOR_SIZE];

    t32_state_t state;
    char halt_reason[128];
    uint64_t instruction_count;
};

static bool ram_range_valid(const t32_machine_t *machine,
                            uint32_t address, size_t length)
{
    size_t start = (size_t)address;

    if (!machine || !machine->memory)
        return false;
    if (start > machine->memory_size)
        return false;
    return length <= machine->memory_size - start;
}

static bool video_range_valid(uint32_t address, size_t length,
                              size_t *offset)
{
    uint64_t start = address;
    uint64_t base = T32_VIDEO_BASE;
    uint64_t end = start + (uint64_t)length;
    uint64_t video_end = base + (uint64_t)T32_VIDEO_SIZE;

    if (start < base || end < start || end > video_end)
        return false;

    if (offset)
        *offset = (size_t)(start - base);
    return true;
}

static bool disk_range_valid(uint32_t address, size_t length, size_t *offset)
{
    uint64_t start = address;
    uint64_t base = T32_DISK_BASE;
    uint64_t end = start + (uint64_t)length;
    uint64_t disk_end = base + (uint64_t)T32_DISK_MMIO_SIZE;

    if (start < base || end < start || end > disk_end)
        return false;

    if (offset)
        *offset = (size_t)(start - base);
    return true;
}

static uint32_t disk_status(const t32_machine_t *machine)
{
    uint32_t status = 0;

    if (machine->disk_file)
        status |= T32_DISK_STATUS_ATTACHED | T32_DISK_STATUS_READY;
    if (machine->disk_error != T32_DISK_ERROR_NONE)
        status |= T32_DISK_STATUS_ERROR;
    return status;
}

static void encode_u32_le(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static uint32_t decode_u32_le(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0]) |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static bool disk_execute(t32_machine_t *machine, uint32_t command)
{
    long offset;
    size_t transferred;

    machine->disk_error = T32_DISK_ERROR_NONE;

    if (!machine->disk_file) {
        machine->disk_error = T32_DISK_ERROR_NO_MEDIA;
        return true;
    }
    if (machine->disk_lba >= machine->disk_sector_count) {
        machine->disk_error = T32_DISK_ERROR_RANGE;
        return true;
    }
    if (machine->disk_lba > (uint32_t)(LONG_MAX / (long)T32_DISK_SECTOR_SIZE)) {
        machine->disk_error = T32_DISK_ERROR_RANGE;
        return true;
    }

    offset = (long)machine->disk_lba * (long)T32_DISK_SECTOR_SIZE;
    if (fseek(machine->disk_file, offset, SEEK_SET) != 0) {
        machine->disk_error = T32_DISK_ERROR_IO;
        return true;
    }

    if (command == T32_DISK_COMMAND_READ) {
        transferred = fread(machine->disk_buffer, 1, T32_DISK_SECTOR_SIZE,
                            machine->disk_file);
        if (transferred != T32_DISK_SECTOR_SIZE) {
            clearerr(machine->disk_file);
            machine->disk_error = T32_DISK_ERROR_IO;
        }
        return true;
    }

    if (command == T32_DISK_COMMAND_WRITE) {
        transferred = fwrite(machine->disk_buffer, 1, T32_DISK_SECTOR_SIZE,
                             machine->disk_file);
        if (transferred != T32_DISK_SECTOR_SIZE ||
            fflush(machine->disk_file) != 0) {
            clearerr(machine->disk_file);
            machine->disk_error = T32_DISK_ERROR_IO;
        }
        return true;
    }

    machine->disk_error = T32_DISK_ERROR_BAD_COMMAND;
    return true;
}

static bool disk_mmio_read(const t32_machine_t *machine, uint32_t address,
                           void *buffer, size_t length)
{
    size_t offset;
    uint8_t bytes[4];
    uint32_t value;

    if (!disk_range_valid(address, length, &offset))
        return false;

    if (offset >= T32_DISK_DATA_OFFSET) {
        size_t data_offset = offset - T32_DISK_DATA_OFFSET;
        if (data_offset + length > T32_DISK_SECTOR_SIZE)
            return false;
        memcpy(buffer, machine->disk_buffer + data_offset, length);
        return true;
    }

    if (length != 4 || (offset & 3u) != 0)
        return false;

    switch ((uint32_t)offset) {
    case T32_DISK_REG_ID: value = T32_DISK_ID; break;
    case T32_DISK_REG_STATUS: value = disk_status(machine); break;
    case T32_DISK_REG_COMMAND: value = 0; break;
    case T32_DISK_REG_LBA: value = machine->disk_lba; break;
    case T32_DISK_REG_ERROR: value = machine->disk_error; break;
    case T32_DISK_REG_SECTOR_SIZE: value = T32_DISK_SECTOR_SIZE; break;
    case T32_DISK_REG_SECTOR_COUNT: value = machine->disk_sector_count; break;
    default: value = 0; break;
    }

    encode_u32_le(bytes, value);
    memcpy(buffer, bytes, 4);
    return true;
}

static bool disk_mmio_write(t32_machine_t *machine, uint32_t address,
                            const void *buffer, size_t length)
{
    size_t offset;
    uint32_t value;

    if (!disk_range_valid(address, length, &offset))
        return false;

    if (offset >= T32_DISK_DATA_OFFSET) {
        size_t data_offset = offset - T32_DISK_DATA_OFFSET;
        if (data_offset + length > T32_DISK_SECTOR_SIZE)
            return false;
        memcpy(machine->disk_buffer + data_offset, buffer, length);
        return true;
    }

    if (length != 4 || (offset & 3u) != 0)
        return false;

    value = decode_u32_le((const uint8_t *)buffer);
    switch ((uint32_t)offset) {
    case T32_DISK_REG_COMMAND:
        return disk_execute(machine, value);
    case T32_DISK_REG_LBA:
        machine->disk_lba = value;
        machine->disk_error = T32_DISK_ERROR_NONE;
        return true;
    case T32_DISK_REG_ERROR:
        if (value == 0)
            machine->disk_error = T32_DISK_ERROR_NONE;
        return true;
    default:
        return true;
    }
}



static bool rtc_range_valid(uint32_t address, size_t length, size_t *offset)
{
    uint64_t start = address;
    uint64_t base = T32_RTC_BASE;
    uint64_t end = start + (uint64_t)length;
    uint64_t rtc_end = base + (uint64_t)T32_RTC_MMIO_SIZE;

    if (start < base || end < start || end > rtc_end)
        return false;
    if (offset)
        *offset = (size_t)(start - base);
    return true;
}

static bool rtc_read(const t32_machine_t *machine, uint32_t address,
                     void *buffer, size_t length)
{
    size_t offset;
    uint32_t value = 0;
    uint8_t bytes[4];
    time_t now;

    (void)machine;

    if (!buffer || !rtc_range_valid(address, length, &offset))
        return false;
    if (length != 4 || (offset & 3u) != 0)
        return false;

    switch ((uint32_t)offset) {
    case T32_RTC_REG_ID:
        value = T32_RTC_ID;
        break;
    case T32_RTC_REG_STATUS:
        now = time(NULL);
        value = now >= 0 && (uint64_t)now <= UINT32_MAX
              ? T32_RTC_STATUS_VALID : 0;
        break;
    case T32_RTC_REG_EPOCH:
        now = time(NULL);
        if (now < 0 || (uint64_t)now > UINT32_MAX)
            return false;
        value = (uint32_t)now;
        break;
    default:
        return false;
    }

    encode_u32_le(bytes, value);
    memcpy(buffer, bytes, sizeof(bytes));
    return true;
}

static bool platform_range_valid(uint32_t address, size_t length,
                                 size_t *offset)
{
    uint64_t start = address;
    uint64_t base = T32_PLATFORM_BASE;
    uint64_t end = start + (uint64_t)length;
    uint64_t platform_end = base + (uint64_t)T32_PLATFORM_MMIO_SIZE;

    if (start < base || end < start || end > platform_end)
        return false;
    if (offset)
        *offset = (size_t)(start - base);
    return true;
}

static bool platform_read(t32_machine_t *machine, uint32_t address,
                          void *buffer, size_t length)
{
    size_t offset;
    uint32_t value = 0;
    uint8_t bytes[4];

    if (!machine || !buffer || !platform_range_valid(address, length, &offset))
        return false;
    if (length != 4 || (offset & 3u) != 0)
        return false;

    switch ((uint32_t)offset) {
    case T32_PLATFORM_REG_ID:
        value = T32_PLATFORM_ID;
        break;
    case T32_PLATFORM_REG_STATUS:
        if (machine->platform_power_off_requested ||
            machine->platform_reset_requested)
            value |= T32_PLATFORM_STATUS_POWER_REQUESTED;
        break;
    case T32_PLATFORM_REG_CONTROL:
        value = 0;
        break;
    case T32_PLATFORM_REG_RAM_SIZE:
        value = machine->memory_size > UINT32_MAX
              ? UINT32_MAX : (uint32_t)machine->memory_size;
        break;
    default:
        return false;
    }

    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
    memcpy(buffer, bytes, 4);
    return true;
}

static bool platform_write(t32_machine_t *machine, uint32_t address,
                           const void *buffer, size_t length)
{
    size_t offset;
    const uint8_t *bytes = (const uint8_t *)buffer;
    uint32_t value;

    if (!machine || !buffer || !platform_range_valid(address, length, &offset))
        return false;
    if (length != 4 || (offset & 3u) != 0)
        return false;

    value = (uint32_t)bytes[0] |
            ((uint32_t)bytes[1] << 8) |
            ((uint32_t)bytes[2] << 16) |
            ((uint32_t)bytes[3] << 24);

    if ((uint32_t)offset != T32_PLATFORM_REG_CONTROL)
        return false;

    if (value == T32_PLATFORM_CONTROL_POWER_OFF) {
        machine->platform_power_off_requested = true;
        return true;
    }

    if (value == T32_PLATFORM_CONTROL_RESET) {
        machine->platform_reset_requested = true;
        return true;
    }

    return false;
}

static bool keyboard_range_valid(uint32_t address, size_t length,
                                 size_t *offset)
{
    uint64_t start = address;
    uint64_t base = T32_KEYBOARD_BASE;
    uint64_t end = start + (uint64_t)length;
    uint64_t keyboard_end = base + (uint64_t)T32_KEYBOARD_MMIO_SIZE;

    if (start < base || end < start || end > keyboard_end)
        return false;
    if (offset)
        *offset = (size_t)(start - base);
    return true;
}

static uint32_t keyboard_status(const t32_machine_t *machine)
{
    return machine && machine->keyboard_count
         ? T32_KEYBOARD_STATUS_DATA_READY : 0;
}

static bool keyboard_read(t32_machine_t *machine, uint32_t address,
                          void *buffer, size_t length)
{
    size_t offset;
    uint32_t value = 0;
    uint8_t bytes[4];

    if (!machine || !buffer || !keyboard_range_valid(address, length, &offset))
        return false;

    if (length != 4 || (offset & 3u) != 0)
        return false;

    switch ((uint32_t)offset) {
    case T32_KEYBOARD_REG_ID:
        value = T32_KEYBOARD_ID;
        break;
    case T32_KEYBOARD_REG_STATUS:
        value = keyboard_status(machine);
        break;
    case T32_KEYBOARD_REG_DATA:
        if (machine->keyboard_count) {
            value = machine->keyboard_queue[machine->keyboard_head];
            machine->keyboard_head =
                (machine->keyboard_head + 1u) % T32_KEYBOARD_QUEUE_SIZE;
            machine->keyboard_count--;
        }
        break;
    default:
        return false;
    }

    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
    memcpy(buffer, bytes, 4);
    return true;
}

static bool fetch_u8(const t32_machine_t *machine, uint32_t address, uint8_t *value)
{
    return value && t32_read_memory(machine, address, value, 1);
}

static bool fetch_u16(const t32_machine_t *machine, uint32_t address, uint16_t *value)
{
    uint8_t bytes[2];

    if (!value || !t32_read_memory(machine, address, bytes, sizeof(bytes)))
        return false;
    *value = (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
    return true;
}

static bool fetch_u32(const t32_machine_t *machine, uint32_t address, uint32_t *value)
{
    uint8_t bytes[4];

    if (!value || !t32_read_memory(machine, address, bytes, sizeof(bytes)))
        return false;
    *value = ((uint32_t)bytes[0]) |
             ((uint32_t)bytes[1] << 8) |
             ((uint32_t)bytes[2] << 16) |
             ((uint32_t)bytes[3] << 24);
    return true;
}

static bool store_u8(t32_machine_t *machine, uint32_t address, uint8_t value)
{
    return t32_write_memory(machine, address, &value, 1);
}

static bool store_u16(t32_machine_t *machine, uint32_t address, uint16_t value)
{
    uint8_t bytes[2] = {(uint8_t)value, (uint8_t)(value >> 8)};
    return t32_write_memory(machine, address, bytes, sizeof(bytes));
}

static bool store_u32(t32_machine_t *machine, uint32_t address, uint32_t value)
{
    uint8_t bytes[4] = {
        (uint8_t)value,
        (uint8_t)(value >> 8),
        (uint8_t)(value >> 16),
        (uint8_t)(value >> 24)
    };
    return t32_write_memory(machine, address, bytes, sizeof(bytes));
}

static void set_fault(t32_machine_t *machine, const char *reason)
{
    machine->state = T32_STATE_ERROR;
    snprintf(machine->halt_reason, sizeof(machine->halt_reason), "%s",
             reason ? reason : "execution fault");
}

static t32_step_result_t memory_fault(t32_machine_t *machine, const char *operation,
                                      uint32_t address)
{
    char reason[128];
    snprintf(reason, sizeof(reason), "%s outside memory at 0x%08x",
             operation, address);
    set_fault(machine, reason);
    return T32_STEP_FAULT;
}

static bool fetch_extension(t32_machine_t *machine, uint32_t *value,
                            const char *instruction_name)
{
    char reason[128];

    if (fetch_u32(machine, machine->pc, value)) {
        machine->pc += 4;
        return true;
    }

    snprintf(reason, sizeof(reason), "%s extension outside memory",
             instruction_name);
    set_fault(machine, reason);
    return false;
}

static void set_zn(t32_machine_t *machine, uint32_t result)
{
    machine->flags.zero = result == 0;
    machine->flags.negative = (result & 0x80000000u) != 0;
}

static uint32_t alu_add(t32_machine_t *machine, uint32_t left, uint32_t right)
{
    uint32_t result = left + right;
    uint64_t wide = (uint64_t)left + (uint64_t)right;

    set_zn(machine, result);
    machine->flags.carry = wide > UINT32_MAX;
    machine->flags.overflow =
        ((~(left ^ right) & (left ^ result)) & 0x80000000u) != 0;
    return result;
}

static uint32_t alu_sub(t32_machine_t *machine, uint32_t left, uint32_t right)
{
    uint32_t result = left - right;

    set_zn(machine, result);
    /* T32 follows the common no-borrow convention. */
    machine->flags.carry = left >= right;
    machine->flags.overflow =
        (((left ^ right) & (left ^ result)) & 0x80000000u) != 0;
    return result;
}

static bool stack_push(t32_machine_t *machine, uint32_t value)
{
    uint32_t sp = machine->registers[T32_STACK_POINTER];

    if (sp < 4)
        return false;
    sp -= 4;
    if (!store_u32(machine, sp, value))
        return false;
    machine->registers[T32_STACK_POINTER] = sp;
    return true;
}

static bool stack_pop(t32_machine_t *machine, uint32_t *value)
{
    uint32_t sp = machine->registers[T32_STACK_POINTER];

    if (!fetch_u32(machine, sp, value))
        return false;
    if (sp > UINT32_MAX - 4)
        return false;
    machine->registers[T32_STACK_POINTER] = sp + 4;
    return true;
}

const char *t32_opcode_name(uint8_t opcode)
{
    switch (opcode) {
    case T32_OPCODE_HALT: return "HALT";
    case T32_OPCODE_NOP: return "NOP";
    case T32_OPCODE_TRAP: return "TRAP";
    case T32_OPCODE_IRET: return "IRET";
    case T32_OPCODE_CPUID: return "CPUID";
    case T32_OPCODE_MOV: return "MOV";
    case T32_OPCODE_MOVI: return "MOVI";
    case T32_OPCODE_LDB: return "LDB";
    case T32_OPCODE_LDH: return "LDH";
    case T32_OPCODE_LDW: return "LDW";
    case T32_OPCODE_STB: return "STB";
    case T32_OPCODE_STH: return "STH";
    case T32_OPCODE_STW: return "STW";
    case T32_OPCODE_ADD: return "ADD";
    case T32_OPCODE_ADDI: return "ADDI";
    case T32_OPCODE_SUB: return "SUB";
    case T32_OPCODE_SUBI: return "SUBI";
    case T32_OPCODE_MUL: return "MUL";
    case T32_OPCODE_MULU: return "MULU";
    case T32_OPCODE_DIV: return "DIV";
    case T32_OPCODE_DIVU: return "DIVU";
    case T32_OPCODE_AND: return "AND";
    case T32_OPCODE_OR: return "OR";
    case T32_OPCODE_XOR: return "XOR";
    case T32_OPCODE_NOT: return "NOT";
    case T32_OPCODE_SHL: return "SHL";
    case T32_OPCODE_SHR: return "SHR";
    case T32_OPCODE_SAR: return "SAR";
    case T32_OPCODE_CMP: return "CMP";
    case T32_OPCODE_CMPI: return "CMPI";
    case T32_OPCODE_JMP: return "JMP";
    case T32_OPCODE_JZ: return "JZ";
    case T32_OPCODE_JNZ: return "JNZ";
    case T32_OPCODE_PUSH: return "PUSH";
    case T32_OPCODE_POP: return "POP";
    case T32_OPCODE_CALL: return "CALL";
    case T32_OPCODE_RET: return "RET";
    default: return "UNKNOWN";
    }
}

t32_machine_t *t32_create(size_t memory_size)
{
    t32_machine_t *machine;

    if (memory_size == 0)
        memory_size = T32_DEFAULT_MEMORY_SIZE;

    machine = (t32_machine_t *)calloc(1, sizeof(*machine));
    if (!machine)
        return NULL;

    machine->memory = (uint8_t *)calloc(1, memory_size);
    if (!machine->memory) {
        free(machine);
        return NULL;
    }

    machine->memory_size = memory_size;
    t32_reset(machine);
    return machine;
}

void t32_destroy(t32_machine_t *machine)
{
    if (!machine)
        return;
    t32_disk_detach(machine);
    free(machine->memory);
    machine->memory = NULL;
    free(machine);
}

void t32_reset(t32_machine_t *machine)
{
    if (!machine)
        return;
    memset(machine->registers, 0, sizeof(machine->registers));
    memset(&machine->flags, 0, sizeof(machine->flags));
    memset(machine->video_memory, ' ', sizeof(machine->video_memory));
    machine->video_dirty = true;
    machine->disk_lba = 0;
    machine->disk_error = T32_DISK_ERROR_NONE;
    memset(machine->disk_buffer, 0, sizeof(machine->disk_buffer));
    machine->pc = 0;
    machine->state = T32_STATE_STOPPED;
    machine->halt_reason[0] = '\0';
    machine->keyboard_head = 0;
    machine->keyboard_tail = 0;
    machine->keyboard_count = 0;
    machine->platform_power_off_requested = false;
    machine->platform_reset_requested = false;
    machine->instruction_count = 0;
}

bool t32_disk_attach(t32_machine_t *machine, const char *path)
{
    FILE *file;
    long size;

    if (!machine || !path)
        return false;

    file = fopen(path, "r+b");
    if (!file)
        return false;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0 ||
        (size % (long)T32_DISK_SECTOR_SIZE) != 0 ||
        (unsigned long)size / T32_DISK_SECTOR_SIZE > UINT32_MAX) {
        fclose(file);
        return false;
    }

    t32_disk_detach(machine);
    machine->disk_file = file;
    machine->disk_sector_count = (uint32_t)((unsigned long)size /
                                             T32_DISK_SECTOR_SIZE);
    machine->disk_lba = 0;
    machine->disk_error = T32_DISK_ERROR_NONE;
    memset(machine->disk_buffer, 0, sizeof(machine->disk_buffer));
    return true;
}

void t32_disk_detach(t32_machine_t *machine)
{
    if (!machine)
        return;
    if (machine->disk_file)
        fclose(machine->disk_file);
    machine->disk_file = NULL;
    machine->disk_sector_count = 0;
    machine->disk_lba = 0;
    machine->disk_error = T32_DISK_ERROR_NONE;
    memset(machine->disk_buffer, 0, sizeof(machine->disk_buffer));
}

bool t32_disk_is_attached(const t32_machine_t *machine)
{
    return machine && machine->disk_file != NULL;
}

uint32_t t32_disk_sector_count(const t32_machine_t *machine)
{
    return machine ? machine->disk_sector_count : 0;
}

bool t32_load_file(t32_machine_t *machine, const char *path, uint32_t address)
{
    FILE *file;
    long file_size;
    uint8_t *buffer;
    size_t read_size;
    bool ok;

    if (!machine || !path)
        return false;
    file = fopen(path, "rb");
    if (!file)
        return false;
    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }
    buffer = (uint8_t *)malloc((size_t)file_size);
    if (!buffer && file_size != 0) {
        fclose(file);
        return false;
    }
    read_size = fread(buffer, 1, (size_t)file_size, file);
    fclose(file);
    if (read_size != (size_t)file_size) {
        free(buffer);
        return false;
    }
    ok = t32_write_memory(machine, address, buffer, read_size);
    free(buffer);
    return ok;
}

bool t32_read_memory(const t32_machine_t *machine, uint32_t address,
                     void *buffer, size_t length)
{
    size_t offset;

    if (!machine || !buffer)
        return false;

    if (video_range_valid(address, length, &offset)) {
        memcpy(buffer, machine->video_memory + offset, length);
        return true;
    }

    if (disk_range_valid(address, length, NULL))
        return disk_mmio_read(machine, address, buffer, length);

    if (keyboard_range_valid(address, length, NULL))
        return keyboard_read((t32_machine_t *)machine, address, buffer, length);

    if (rtc_range_valid(address, length, NULL))
        return rtc_read(machine, address, buffer, length);

    if (platform_range_valid(address, length, NULL))
        return platform_read((t32_machine_t *)machine, address, buffer, length);

    if (!ram_range_valid(machine, address, length))
        return false;

    memcpy(buffer, machine->memory + address, length);
    return true;
}

bool t32_write_memory(t32_machine_t *machine, uint32_t address,
                      const void *buffer, size_t length)
{
    size_t offset;

    if (!machine || !buffer)
        return false;

    if (video_range_valid(address, length, &offset)) {
        if (memcmp(machine->video_memory + offset, buffer, length) != 0) {
            memcpy(machine->video_memory + offset, buffer, length);
            machine->video_dirty = true;
        }
        return true;
    }

    if (disk_range_valid(address, length, NULL))
        return disk_mmio_write(machine, address, buffer, length);

    if (rtc_range_valid(address, length, NULL))
        return false;

    if (platform_range_valid(address, length, NULL))
        return platform_write(machine, address, buffer, length);

    if (!ram_range_valid(machine, address, length))
        return false;

    memcpy(machine->memory + address, buffer, length);
    return true;
}


bool t32_keyboard_push(t32_machine_t *machine, uint8_t character)
{
    if (!machine || machine->keyboard_count >= T32_KEYBOARD_QUEUE_SIZE)
        return false;

    machine->keyboard_queue[machine->keyboard_tail] = character;
    machine->keyboard_tail =
        (machine->keyboard_tail + 1u) % T32_KEYBOARD_QUEUE_SIZE;
    machine->keyboard_count++;
    return true;
}

size_t t32_keyboard_pending(const t32_machine_t *machine)
{
    return machine ? machine->keyboard_count : 0;
}


bool t32_power_off_requested(const t32_machine_t *machine)
{
    return machine && machine->platform_power_off_requested;
}

bool t32_reset_requested(const t32_machine_t *machine)
{
    return machine && machine->platform_reset_requested;
}

void t32_clear_platform_requests(t32_machine_t *machine)
{
    if (!machine)
        return;
    machine->platform_power_off_requested = false;
    machine->platform_reset_requested = false;
}

bool t32_video_is_dirty(const t32_machine_t *machine)
{
    return machine ? machine->video_dirty : false;
}

void t32_video_clear_dirty(t32_machine_t *machine)
{
    if (machine)
        machine->video_dirty = false;
}

bool t32_set_register(t32_machine_t *machine, unsigned register_number,
                      uint32_t value)
{
    if (!machine || register_number >= T32_REGISTER_COUNT)
        return false;
    machine->registers[register_number] = value;
    return true;
}

uint32_t t32_get_register(const t32_machine_t *machine, unsigned register_number)
{
    if (!machine || register_number >= T32_REGISTER_COUNT)
        return 0;
    return machine->registers[register_number];
}

void t32_set_pc(t32_machine_t *machine, uint32_t pc)
{
    if (machine)
        machine->pc = pc;
}

uint32_t t32_get_pc(const t32_machine_t *machine)
{
    return machine ? machine->pc : 0;
}

t32_flags_t t32_get_flags(const t32_machine_t *machine)
{
    t32_flags_t empty = {false, false, false, false};
    return machine ? machine->flags : empty;
}

t32_state_t t32_get_state(const t32_machine_t *machine)
{
    return machine ? machine->state : T32_STATE_ERROR;
}

const char *t32_state_name(t32_state_t state)
{
    switch (state) {
    case T32_STATE_STOPPED: return "stopped";
    case T32_STATE_RUNNING: return "running";
    case T32_STATE_HALTED: return "halted";
    case T32_STATE_ERROR: return "error";
    default: return "unknown";
    }
}

const char *t32_get_halt_reason(const t32_machine_t *machine)
{
    return (!machine || !machine->halt_reason[0]) ? "" : machine->halt_reason;
}

uint64_t t32_get_instruction_count(const t32_machine_t *machine)
{
    return machine ? machine->instruction_count : 0;
}

void t32_clear_halt(t32_machine_t *machine)
{
    if (machine && machine->state == T32_STATE_HALTED) {
        machine->state = T32_STATE_STOPPED;
        machine->halt_reason[0] = '\0';
    }
}


static bool instruction_encoding_mask(uint8_t opcode, uint32_t *mask)
{
    uint32_t allowed;

    switch (opcode) {
    case T32_OPCODE_HALT:
    case T32_OPCODE_NOP:
    case T32_OPCODE_TRAP:
    case T32_OPCODE_IRET:
    case T32_OPCODE_JMP:
    case T32_OPCODE_RET:
    case T32_OPCODE_CALL:
        allowed = 0xff000000u;
        break;

    case T32_OPCODE_CPUID:
    case T32_OPCODE_MOVI:
    case T32_OPCODE_POP:
        allowed = 0xfff00000u; /* opcode + rd */
        break;

    case T32_OPCODE_CMPI:
    case T32_OPCODE_JZ:
    case T32_OPCODE_JNZ:
    case T32_OPCODE_PUSH:
        allowed = 0xff0f0000u; /* opcode + ra */
        break;

    case T32_OPCODE_MOV:
    case T32_OPCODE_LDB:
    case T32_OPCODE_LDH:
    case T32_OPCODE_LDW:
    case T32_OPCODE_ADDI:
    case T32_OPCODE_SUBI:
    case T32_OPCODE_NOT:
        allowed = 0xffff0000u; /* opcode + rd + ra */
        break;

    case T32_OPCODE_STB:
    case T32_OPCODE_STH:
    case T32_OPCODE_STW:
    case T32_OPCODE_CMP:
        allowed = 0xff0ff000u; /* opcode + ra + rb */
        break;

    case T32_OPCODE_ADD:
    case T32_OPCODE_SUB:
    case T32_OPCODE_MUL:
    case T32_OPCODE_MULU:
    case T32_OPCODE_DIV:
    case T32_OPCODE_DIVU:
    case T32_OPCODE_AND:
    case T32_OPCODE_OR:
    case T32_OPCODE_XOR:
    case T32_OPCODE_SHL:
    case T32_OPCODE_SHR:
    case T32_OPCODE_SAR:
        allowed = 0xfffff000u; /* opcode + rd + ra + rb */
        break;

    default:
        return false;
    }

    if (mask)
        *mask = allowed;
    return true;
}

t32_step_result_t t32_step(t32_machine_t *machine)
{
    uint32_t instruction;
    uint32_t extension;
    uint32_t left;
    uint32_t right;
    uint32_t result;
    uint32_t address;
    uint8_t opcode;
    unsigned rd;
    unsigned ra;
    unsigned rb;

    if (!machine)
        return T32_STEP_FAULT;
    if (machine->state == T32_STATE_HALTED)
        return T32_STEP_HALTED;
    if (machine->state == T32_STATE_ERROR)
        return T32_STEP_FAULT;

    machine->state = T32_STATE_RUNNING;
    if (!fetch_u32(machine, machine->pc, &instruction)) {
        set_fault(machine, "instruction fetch outside memory");
        return T32_STEP_FAULT;
    }

    machine->pc += 4;
    opcode = (uint8_t)(instruction >> 24);
    rd = (instruction >> 20) & 0x0fu;
    ra = (instruction >> 16) & 0x0fu;
    rb = (instruction >> 12) & 0x0fu;
    machine->instruction_count++;

    {
        uint32_t allowed_mask;
        if (instruction_encoding_mask(opcode, &allowed_mask) &&
            (instruction & ~allowed_mask) != 0) {
            char reason[128];
            snprintf(reason, sizeof(reason),
                     "invalid encoding for %s at 0x%08x: 0x%08x",
                     t32_opcode_name(opcode), machine->pc - 4, instruction);
            set_fault(machine, reason);
            return T32_STEP_FAULT;
        }
    }

    switch (opcode) {
    case T32_OPCODE_HALT:
        machine->state = T32_STATE_HALTED;
        snprintf(machine->halt_reason, sizeof(machine->halt_reason),
                 "HALT instruction");
        return T32_STEP_HALTED;

    case T32_OPCODE_NOP:
        break;

    case T32_OPCODE_TRAP:
        if (!fetch_extension(machine, &extension, "TRAP"))
            return T32_STEP_FAULT;
        machine->state = T32_STATE_HALTED;
        snprintf(machine->halt_reason, sizeof(machine->halt_reason),
                 "TRAP 0x%08x", extension);
        return T32_STEP_HALTED;

    case T32_OPCODE_IRET:
        if (!stack_pop(machine, &machine->pc))
            return memory_fault(machine, "IRET stack read",
                                machine->registers[T32_STACK_POINTER]);
        break;

    case T32_OPCODE_CPUID:
        machine->registers[rd] = T32_CPUID_VALUE;
        break;

    case T32_OPCODE_MOV:
        machine->registers[rd] = machine->registers[ra];
        break;

    case T32_OPCODE_MOVI:
        if (!fetch_extension(machine, &extension, "MOVI"))
            return T32_STEP_FAULT;
        machine->registers[rd] = extension;
        set_zn(machine, extension);
        break;

    case T32_OPCODE_LDB: {
        uint8_t value;
        address = machine->registers[ra];
        if (!fetch_u8(machine, address, &value))
            return memory_fault(machine, "LDB read", address);
        machine->registers[rd] = value;
        break;
    }

    case T32_OPCODE_LDH: {
        uint16_t value;
        address = machine->registers[ra];
        if (!fetch_u16(machine, address, &value))
            return memory_fault(machine, "LDH read", address);
        machine->registers[rd] = value;
        break;
    }

    case T32_OPCODE_LDW:
        address = machine->registers[ra];
        if (!fetch_u32(machine, address, &machine->registers[rd]))
            return memory_fault(machine, "LDW read", address);
        break;

    case T32_OPCODE_STB:
        address = machine->registers[ra];
        if (!store_u8(machine, address, (uint8_t)machine->registers[rb]))
            return memory_fault(machine, "STB write", address);
        break;

    case T32_OPCODE_STH:
        address = machine->registers[ra];
        if (!store_u16(machine, address, (uint16_t)machine->registers[rb]))
            return memory_fault(machine, "STH write", address);
        break;

    case T32_OPCODE_STW:
        address = machine->registers[ra];
        if (!store_u32(machine, address, machine->registers[rb]))
            return memory_fault(machine, "STW write", address);
        break;

    case T32_OPCODE_ADD:
        machine->registers[rd] =
            alu_add(machine, machine->registers[ra], machine->registers[rb]);
        break;

    case T32_OPCODE_ADDI:
        if (!fetch_extension(machine, &extension, "ADDI"))
            return T32_STEP_FAULT;
        machine->registers[rd] =
            alu_add(machine, machine->registers[ra], extension);
        break;

    case T32_OPCODE_SUB:
        machine->registers[rd] =
            alu_sub(machine, machine->registers[ra], machine->registers[rb]);
        break;

    case T32_OPCODE_SUBI:
        if (!fetch_extension(machine, &extension, "SUBI"))
            return T32_STEP_FAULT;
        machine->registers[rd] =
            alu_sub(machine, machine->registers[ra], extension);
        break;

    case T32_OPCODE_MUL: {
        int64_t product = (int64_t)(int32_t)machine->registers[ra] *
                          (int64_t)(int32_t)machine->registers[rb];
        result = (uint32_t)product;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = product > INT32_MAX || product < INT32_MIN;
        break;
    }

    case T32_OPCODE_MULU: {
        uint64_t product = (uint64_t)machine->registers[ra] *
                           (uint64_t)machine->registers[rb];
        result = (uint32_t)product;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = (product >> 32) != 0;
        machine->flags.overflow = false;
        break;
    }

    case T32_OPCODE_DIV: {
        int32_t dividend = (int32_t)machine->registers[ra];
        int32_t divisor = (int32_t)machine->registers[rb];
        if (divisor == 0) {
            set_fault(machine, "DIV by zero");
            return T32_STEP_FAULT;
        }
        if (dividend == INT32_MIN && divisor == -1) {
            set_fault(machine, "DIV signed overflow");
            return T32_STEP_FAULT;
        }
        result = (uint32_t)(dividend / divisor);
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;
    }

    case T32_OPCODE_DIVU:
        right = machine->registers[rb];
        if (right == 0) {
            set_fault(machine, "DIVU by zero");
            return T32_STEP_FAULT;
        }
        result = machine->registers[ra] / right;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;

    case T32_OPCODE_AND:
        result = machine->registers[ra] & machine->registers[rb];
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;

    case T32_OPCODE_OR:
        result = machine->registers[ra] | machine->registers[rb];
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;

    case T32_OPCODE_XOR:
        result = machine->registers[ra] ^ machine->registers[rb];
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;

    case T32_OPCODE_NOT:
        result = ~machine->registers[ra];
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = false;
        machine->flags.overflow = false;
        break;

    case T32_OPCODE_SHL: {
        unsigned count = machine->registers[rb] & 31u;
        left = machine->registers[ra];
        result = count ? left << count : left;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = count ? ((left >> (32u - count)) & 1u) != 0 : false;
        machine->flags.overflow = false;
        break;
    }

    case T32_OPCODE_SHR: {
        unsigned count = machine->registers[rb] & 31u;
        left = machine->registers[ra];
        result = count ? left >> count : left;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = count ? ((left >> (count - 1u)) & 1u) != 0 : false;
        machine->flags.overflow = false;
        break;
    }

    case T32_OPCODE_SAR: {
        unsigned count = machine->registers[rb] & 31u;
        left = machine->registers[ra];
        result = count ? (uint32_t)((int32_t)left >> count) : left;
        machine->registers[rd] = result;
        set_zn(machine, result);
        machine->flags.carry = count ? ((left >> (count - 1u)) & 1u) != 0 : false;
        machine->flags.overflow = false;
        break;
    }

    case T32_OPCODE_CMP:
        (void)alu_sub(machine, machine->registers[ra], machine->registers[rb]);
        break;

    case T32_OPCODE_CMPI:
        if (!fetch_extension(machine, &extension, "CMPI"))
            return T32_STEP_FAULT;
        (void)alu_sub(machine, machine->registers[ra], extension);
        break;

    case T32_OPCODE_JMP:
        if (!fetch_extension(machine, &extension, "JMP"))
            return T32_STEP_FAULT;
        machine->pc = extension;
        break;

    case T32_OPCODE_JZ:
        if (!fetch_extension(machine, &extension, "JZ"))
            return T32_STEP_FAULT;
        if (machine->registers[ra] == 0)
            machine->pc = extension;
        break;

    case T32_OPCODE_JNZ:
        if (!fetch_extension(machine, &extension, "JNZ"))
            return T32_STEP_FAULT;
        if (machine->registers[ra] != 0)
            machine->pc = extension;
        break;

    case T32_OPCODE_PUSH:
        if (!stack_push(machine, machine->registers[ra]))
            return memory_fault(machine, "PUSH stack write",
                                machine->registers[T32_STACK_POINTER]);
        break;

    case T32_OPCODE_POP:
        if (!stack_pop(machine, &machine->registers[rd]))
            return memory_fault(machine, "POP stack read",
                                machine->registers[T32_STACK_POINTER]);
        break;

    case T32_OPCODE_CALL:
        if (!fetch_extension(machine, &extension, "CALL"))
            return T32_STEP_FAULT;
        if (!stack_push(machine, machine->pc))
            return memory_fault(machine, "CALL stack write",
                                machine->registers[T32_STACK_POINTER]);
        machine->pc = extension;
        break;

    case T32_OPCODE_RET:
        if (!stack_pop(machine, &machine->pc))
            return memory_fault(machine, "RET stack read",
                                machine->registers[T32_STACK_POINTER]);
        break;

    default: {
        char reason[128];
        snprintf(reason, sizeof(reason),
                 "unknown opcode 0x%02x at 0x%08x",
                 opcode, machine->pc - 4);
        set_fault(machine, reason);
        return T32_STEP_FAULT;
    }
    }

    machine->state = T32_STATE_STOPPED;
    return T32_STEP_OK;
}

t32_step_result_t t32_run(t32_machine_t *machine, uint64_t instruction_limit)
{
    uint64_t executed = 0;

    if (!machine)
        return T32_STEP_FAULT;

    for (;;) {
        t32_step_result_t step_result;

        if (instruction_limit && executed >= instruction_limit) {
            machine->state = T32_STATE_STOPPED;
            snprintf(machine->halt_reason, sizeof(machine->halt_reason),
                     "instruction limit reached");
            return T32_STEP_OK;
        }

        step_result = t32_step(machine);
        executed++;
        if (step_result != T32_STEP_OK)
            return step_result;
    }
}
