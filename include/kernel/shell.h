/*
 * KaiOS - Shell (Terminal) Header
 */

#ifndef KAIOS_SHELL_H
#define KAIOS_SHELL_H

#include "include/kernel/types.h"

// Shell constants
#define SHELL_MAX_INPUT    256
#define SHELL_MAX_ARGS     16
#define SHELL_HISTORY_SIZE 10

// Shell functions
void shell_init(void);
void shell_run(void);
void shell_process_command(char* input);

// Built-in commands
void cmd_help(int argc, char** argv);
void cmd_clear(int argc, char** argv);
void cmd_echo(int argc, char** argv);
void cmd_ls(int argc, char** argv);
void cmd_cd(int argc, char** argv);
void cmd_pwd(int argc, char** argv);
void cmd_mkdir(int argc, char** argv);
void cmd_rmdir(int argc, char** argv);
void cmd_touch(int argc, char** argv);
void cmd_rm(int argc, char** argv);
void cmd_cat(int argc, char** argv);
void cmd_write(int argc, char** argv);
void cmd_cp(int argc, char** argv);
void cmd_mv(int argc, char** argv);
void cmd_free(int argc, char** argv);
void cmd_uname(int argc, char** argv);
void cmd_date(int argc, char** argv);
void cmd_uptime(int argc, char** argv);
void cmd_reboot(int argc, char** argv);
void cmd_shutdown(int argc, char** argv);
void cmd_sync(int argc, char** argv);

#endif // KAIOS_SHELL_H
