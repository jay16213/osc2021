#ifndef _LCD_SHELL_H_
#define _LCD_SHELL_H_

void shell_welcome_message();
void shell_start();

void command_controller(char *cmd);

void command_help();
void command_hello();
void command_reboot();

void command_cpio_info();
void command_ls();

void command_allocate();
void command_free();
void command_meminfo();

void command_not_found();

#endif