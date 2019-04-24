#include <stdio.h>
#include "StudentsDB.h"
#include "Commands.h"
#include "StudentsDB_ConsoleCommands.h"

static const char* argsDesrciption1[] = { "First name", "Second name", "Last name", "Study subject name", "Rating" };
static const char* argsDescription2[] = { "Group name" };
static const char* argsDescription3[] = { "First name", "Second name", "Last name", "Group name" };
static const char* argsDescription4[] = { "Database path" };
static const char* argsDescription5[] = { "Study subject name" };
static const char* argFormats[] = { "%s", "%s", "%s", "%s", "%i" };

static struct Command dbCommands[STUDENTSDB_COMMANDS_COUNT] =
{
	{ "Help",                   0, 0,          Command_Help,                   "",                                                                   0 },
	{ "CommandHelp",            1, argFormats, Command_CommandHelp,            "",                                                                   0 },
	{ "Exit",                   0, 0,          Command_Exit,                   "",                                                                   0 },

	{ "PrintAllStudents",       0, 0,          Command_PrintAllStudents,       "Displays all students",                                              0 },
	{ "PrintStudentsFromGroup", 1, argFormats, Command_PrintStudentsFromGroup, "Displays students from the selected group",                          argsDescription2 },
	{ "PrintStudent",           3, argFormats, Command_PrintStudent,           "Looks for a student by name and displays it",                        argsDesrciption1 },
	{ "RemoveStudent",          3, argFormats, Command_RemoveStudent,          "Deletes a student",                                                  argsDesrciption1 },
	{ "AddStudent",             4, argFormats, Command_AddStudent,             "Adds a student",                                                     argsDescription3 },
	{ "SetStudentRating",       5, argFormats, Command_SetStudentRating,       "Changes the student's grade\n for the selected study subject",       argsDesrciption1 },
	{ "PrintStudentsWithDebts", 0, 0,          Command_PrintStudentsWithDebts, "Displays students with academic debts",                              0 },
	{ "PrintGroupPerformance",  1, argFormats, Command_PrintGroupPerformance,  "Displays the percentage of students\n who passed exams for 4 and 5", argsDescription2 },
	{ "PrintBestStudySubjects", 0, 0,          Command_PrintBestStudySubjects, "Displays the title of the study subject\n that was best delivered",  0 },
	{ "PrintStudySubjects",     0, 0,          Command_PrintStudySubjects,     "Displays the names of the study subjects",                           0 },
	{ "PrintGroups",            0, 0,          Command_PrintGroups,            "Displays group names",                                               0 },
	{ "CreateDB",               1, argFormats, Command_CreateDB,               "Creates and opens a database",                                       argsDescription4 },
	{ "OpenDB",                 1, argFormats, Command_OpenDB,                 "Opens the database on the specified path",                           argsDescription4 },
	{ "CloseDB",                0, 0,          Command_CloseDB,                "Closes the last opened database",                                    0 },
	{ "CheckDB",                1, argFormats, Command_CheckDB,                "Checks the database for format errors",                              argsDescription4}
};

int main()
{
	printf("===============================================================================\n");
	printf("1)Write Help for information about commands.\n");
	printf("2)For more information about the command, write CommandHelp [command]\n");
	printf("3)To log out, type Exit\n");
	printf("===============================================================================\n");

	char nameBuffer[DB_COMMAND_NAME_BUFFER_LEN];
	char argsBuffer[DB_COMMAND_ARGS_BUFFER_LEN];
	InitCommands(dbCommands, STUDENTSDB_COMMANDS_COUNT, nameBuffer, argsBuffer);

	ConsoleLoop();

	if (dbIsOpen)
	{
		DB_Close(&studentsDB);

		if (studySubjects.count != 0) FREE_STUDY_SUBJECTS(studySubjects);
	}

	return 0;
}