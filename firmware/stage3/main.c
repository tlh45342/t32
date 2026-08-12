/* T32 C STAGE3 0.0.13 - RTC time command. */

extern int puts(char *s);
extern int putchar(int ch);
extern int strcmp(char *left, char *right);
extern int t32_getchar(void);
extern int t32_rtc_epoch(void);



int print_prompt(void)
{
    putchar(116);
    putchar(51);
    putchar(50);
    putchar(62);
    putchar(32);
    return 0;
}

int print_uint(int value)
{
    char digits[11];
    int count = 0;
    int quotient;
    int digit;

    if (value == 0) {
        putchar(48);
        return 0;
    }

    while (value > 0) {
        quotient = value / 10;
        digit = value - quotient * 10;
        digits[count] = digit + 48;
        count = count + 1;
        value = quotient;
    }

    while (count > 0) {
        count = count - 1;
        putchar(digits[count]);
    }

    return 0;
}

int command_time(void)
{
    int epoch;

    epoch = t32_rtc_epoch();
    puts("RTC epoch:");
    print_uint(epoch);
    putchar(10);
    return 0;
}

int command_help(void)
{
    puts("help     show commands");
    puts("version  show monitor version");
    puts("bootinfo show boot handoff status");
    puts("mem      show current memory contract");
    puts("time     show RTC UTC epoch seconds");
    puts("halt     leave the monitor");
    return 0;
}

int main(void)
{
    char line[64];
    int done = 0;
    int length;
    int ch;

    puts("T32 Stage3 Monitor 0.0.13");
    puts("Type 'help' for commands.");

    while (done == 0) {
        print_prompt();
        length = 0;

        while (length < 63) {
            ch = t32_getchar();

            if (ch == 13) {
                putchar(10);
                break;
            } else if (ch == 10) {
                putchar(10);
                break;
            } else if (ch == 8) {
                if (length > 0) {
                    length = length - 1;
                    putchar(8);
                }
            } else if (ch == 127) {
                if (length > 0) {
                    length = length - 1;
                    putchar(8);
                }
            } else if (ch >= 32) {
                if (ch <= 126) {
                    line[length] = ch;
                    length = length + 1;
                    putchar(ch);
                }
            }
        }

        line[length] = 0;

        if (length == 0) {
            /* Blank line: simply reprompt. */
        } else if (strcmp(line, "help") == 0) {
            command_help();
        } else if (strcmp(line, "version") == 0) {
            puts("T32 Stage3 Monitor 0.0.13");
        } else if (strcmp(line, "time") == 0) {
            command_time();
        } else if (strcmp(line, "bootinfo") == 0) {
            puts("Bootinfo v0.2 handoff OK");
        } else if (strcmp(line, "mem") == 0) {
            puts("Stage3 load address 0x00020000");
            puts("Stage3 stack top    0x0000E000");
        } else if (strcmp(line, "halt") == 0) {
            done = 1;
        } else {
            puts("Unknown command");
        }
    }

    return 42;
}
