#pragma once

#define STUDENTSDB_BASE_COMMANDS_COUNT 3
#define STUDENTSDB_MAIN_COMMANDS_COUNT 15
#define STUDENTSDB_COMMANDS_COUNT (STUDENTSDB_MAIN_COMMANDS_COUNT + STUDENTSDB_BASE_COMMANDS_COUNT)

#define DB_COMMAND_NAME_BUFFER_LEN 64
#define DB_COMMAND_ARGS_BUFFER_LEN 260

void Command_Help(void* args);
void Command_CommandHelp(void* args);
void Command_Exit(void* args);

void Command_PrintAllStudents(void* args);
void Command_PrintStudent(void* args);
void Command_PrintStudentsFromGroup(void* args);
void Command_RemoveStudent(void* args);
void Command_AddStudent(void* args);
void Command_SetStudentRating(void* args);
void Command_PrintStudentsWithDebts(void* args);
void Command_PrintGroupPerformance(void* args);
void Command_PrintBestStudySubjects(void* args);
void Command_PrintStudySubjects(void* args);
void Command_PrintGroups(void* args);
void Command_CreateDB(void* args);
void Command_OpenDB(void* args);
void Command_CloseDB(void* args);
void Command_CheckDB(void* args);