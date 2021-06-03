#ifndef COMMAND_H
#define COMMAND_H

void InputBufferOverflowMessage(char []);

void CommandInit();
void CommandHelp();
void CommandHello();
void CommandCpiols();
void CommandCpioexe(char *);
void CommandCpiocat(char *);
void CommandTimestamp();
void CommandBuddyInit();
void CommandBuddyLogList();
void CommandBuddyLogPool();
void CommandBuddyLogTable();
void CommandBuddyFree(int section);
void CommandBuddyFreePool(int pool, int index);
void CommandBuddyAlloc(int size);
void CommandThreadTest(int test_id);
void CommandNotFound(char *);
void CommandReboot();

#endif
