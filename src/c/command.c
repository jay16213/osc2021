#include "uart.h"
#include "string.h"
#include "cpio.h"
#include "printf.h"

void input_buffer_overflow_message(char cmd[])
{
    printf("Follow command: \"");
    printf(cmd);
    printf("\"... is too long to process.\n");

    printf("The maximum length of input is 64.");
}

void command_help()
{
    printf("\n");
    printf("Valid Command:\n");
    printf("\thelp:\t\tprint this help.\n");
    printf("\thello:\t\tprint \"Hello World!\".\n");
    printf("\treboot:\t\treboot Raspberry Pi.\n");
    printf("\tcpio:\t\tread initramfs.cpio on the SD card.\n");
    printf("\ttimer_on:\tturn on the core timer.\n");
    printf("\ttimer_off:\tturn off the core timer.\n");
    printf("\n");
}

void command_hello()
{
    printf("Hello World!\n");
}

void command_cpio()
{
    printf("\n");
    printf("Valid Option:\n");
    printf("\t1:\t\tGet the file list.\n");
    printf("\t2:\t\tSearch for a specific file.\n");
    printf("\t3:\t\tRun an executable.\n");
    printf("\n");
    printf("# ");

    char choice = uart_getc();
    printf("%c", choice);
    char new_line = uart_getc();
    printf("\n");

    switch (choice)
    {
    case '1':
        cpio_ls();
        break;

    case '2':
    {
        char file_name[100];
        char c;
        int counter = 0;

        printf("# ");

        while (1)
        {
            c = uart_getc();
            // delete
            if ((c == 127) && counter > 0)
            {
                counter--;
                printf("\b \b");
            }
            // new line
            else if ((c == 10) || (c == 13))
            {
                file_name[counter] = '\0';
                uart_send(c);
                break;
            }
            // regular input
            else if (counter < 100)
            {
                file_name[counter] = c;
                counter++;
                uart_send(c);
            }
        }

        cpio_find_file(file_name);
    }

    break;

    case '3':
    {
        char file_name[100];
        char c;
        int counter = 0;

        printf("# ");

        while (1)
        {
            c = uart_getc();
            // delete
            if ((c == 127) && counter > 0)
            {
                counter--;
                printf("\b \b");
            }
            // new line
            else if ((c == 10) || (c == 13))
            {
                file_name[counter] = '\0';
                printf("%c", c);
                break;
            }
            // regular input
            else if (counter < 100)
            {
                file_name[counter] = c;
                counter++;
                printf("%c", c);
            }
        }

        cpio_run_executable(file_name);
    }

    break;

    default:
        break;
    }
}

void command_timer_on()
{
    asm volatile("svc 1");
}

void command_timer_off()
{
    asm volatile("svc 2");
}

void command_not_found(char *s)
{
    printf("Err: command ");
    printf(s);
    printf(" not found, try <help>\n");
}

void command_reboot()
{
    printf("Start Rebooting...\n");

    *PM_WDOG = PM_PASSWORD | 100;
    *PM_RSTC = PM_PASSWORD | 0x20;

    // while(1);
}