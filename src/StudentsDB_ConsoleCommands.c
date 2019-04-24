#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Commands.h"
#include "Table.h"
#include "StudentsDB.h"
#include "StudentsDB_ConsoleCommands.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NUMS_COUNT(A) (((size_t)log10(A)) + 1)

static struct StudySubjectSumRating
{
	const char* name;
	int32_t rating;
};

static size_t MemStrNumsSize(int64_t count)
{
	size_t c = (size_t)log10(count);
	size_t size = (count - (size_t)pow(10, c)) * (c + 2) + 2;

	for (size_t x = 1; x <= c; x++)
		size += 9 * (x + 1) * (size_t)pow(10, x - 1);

	return size;
}

static void FindStudent(void* name, struct StudentInfo* out_res)
{
	const char* firstName = (char*)name;
	const char* secondName = firstName + (strlen(firstName) + 1);
	const char* lastName = secondName + (strlen(secondName) + 1);

	uint32_t size;

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		DB_Get(&studentsDB, &size, Read_StudentInfo, x, out_res);

		if (strcmp(firstName, out_res->firstName) == 0 && strcmp(secondName, out_res->secondName) == 0 && strcmp(lastName, out_res->lastName) == 0)
			return;

		FREE_STUDENT_INFO(*out_res);
	}

	out_res->firstName = 0;
}

static int64_t FindStudentIndex(void* args)
{
	const char* firstName = (char*)args;
	const char* secondName = firstName + (strlen(firstName) + 1);
	const char* lastName = secondName + (strlen(secondName) + 1);

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		uint32_t size;
		struct StudentInfo student;

		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		int r = (strcmp(firstName, student.firstName) == 0) && (strcmp(secondName, student.secondName) == 0) && (strcmp(lastName, student.lastName) == 0);

		FREE_STUDENT_INFO(student);

		if (r) return x;
	}

	return -1;
}

static size_t FindStudySubjectIndex(const char* name)
{
	for (uint32_t x = 0; x < studySubjects.count; x++)
		if (strcmp(name, studySubjects.names[x]) == 0)
			return x;

	return SIZE_MAX;
}

static uint32_t GetDebtsCount(struct StudentInfo* student_ptr)
{
	uint32_t count = 0;

	for (uint32_t x = 0; x < studySubjects.count; x++)
		if (student_ptr->studySubjectRatings[x] == STUDY_SUBJECT_RATING_ACDEMIC_DEBT)
			count++;

	return count;
}

static int IsGoodStudent(struct StudentInfo* student_ptr)
{
	for (uint32_t x = 0; x < studySubjects.count; x++)
		if (student_ptr->studySubjectRatings[x] < 4) return 0;

	return 1;
}

static int Compare_StudySubjectSumRating(const void* r1, const void* r2)
{
	if (((struct StudySubjectSumRating*)r1)->rating < ((struct StudySubjectSumRating*)r2)->rating) return 1;
	if (((struct StudySubjectSumRating*)r1)->rating >((struct StudySubjectSumRating*)r2)->rating) return -1;
	return 0;
}

static int IsUniqueStr(const char* strs, size_t strsCount, const char* str)
{
	for (size_t x = 0, offset = 0; x < strsCount; x++, offset += strlen(strs + offset) + 1)
		if (strcmp(str, strs + offset) == 0) return 0;

	return 1;
}

static void Write_EmptyStudySubjects(FILE* stream, uint32_t size, va_list args)
{
	fseek(stream, size, SEEK_CUR);
}

static void Write_CountStudySubjects(FILE* stream, uint32_t size, va_list args)
{
	fwrite(&(*(struct StudySubjects*)args).count, sizeof(uint32_t), 1, stream);
}


void Command_Help(void* args)
{
	struct Cell cells[2 * STUDENTSDB_MAIN_COMMANDS_COUNT];
	struct TableBorders borders = DEFAULT_BORDERS;
	struct Table table = { cells, 2 * STUDENTSDB_MAIN_COMMANDS_COUNT, 2, STUDENTSDB_MAIN_COMMANDS_COUNT, &borders };

	for (int y = 0; y < table.cellsHeight; y++)
	{
		struct Cell cell1 = DEFAULT_CELL(1, 1, (char*)commands[y + STUDENTSDB_BASE_COMMANDS_COUNT].name);
		SetCell(&table, 0, y, cell1);
		struct Cell cell2 = DEFAULT_CELL(1, 1, (char*)commands[y + STUDENTSDB_BASE_COMMANDS_COUNT].funcDescription);
		SetCell(&table, 1, y, cell2);
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
}

void Command_CommandHelp(void* args)
{
	size_t commandId = FindCommand((const char*)args);

	if (commandId == SIZE_MAX)
	{
		printf("Command does not exist");
		return;
	}

	struct Cell* cells = (struct Cell*)calloc(2 * (3 + commands[commandId].argsCount), sizeof(struct Cell));
	struct TableBorders borders = DEFAULT_BORDERS;
	struct Table table = { cells, 2 * (3 + commands[commandId].argsCount), 2, 3 + commands[commandId].argsCount, &borders };

	struct Cell cell1 = DEFAULT_CELL(2, 1, "Description");
	SetCell(&table, 0, 0, cell1);
	struct Cell cell2 = DEFAULT_CELL(2, 1, (char*)commands[commandId].funcDescription);
	SetCell(&table, 0, 1, cell2);
	struct Cell cell3 = DEFAULT_CELL(2, 1, "Args description");
	SetCell(&table, 0, 2, cell3);

	for (int y = 3; y < table.cellsHeight; y++)
	{
		struct Cell cell4 = DEFAULT_CELL(1, 1, "Argument");
		SetCell(&table, 0, y, cell4);
		struct Cell cell5 = DEFAULT_CELL(1, 1, (char*)commands[commandId].argsDescription[y - 3]);
		SetCell(&table, 1, y, cell5);
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(cells);
}

void Command_Exit(void* args)
{
	exitFlag = 1;
}


void Command_PrintAllStudents(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	int64_t studentsCount = DB_ObjectsCount(&studentsDB) - 1;
	uint32_t studentCellsHeight = MAX(3, studySubjects.count + 1);

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(7 * (studentsCount * studentCellsHeight + 1), sizeof(struct Cell));
	struct Table table = { cells, 7 * (studentsCount * studentCellsHeight + 1), 7, studentsCount * studentCellsHeight + 1, &borders };

	struct Cell cell1 = DEFAULT_CELL(7, 1, "STUDENTS");
	SetCell(&table, 0, 0, cell1);

	char* memStrNums = (char*)malloc(MemStrNumsSize(studentsCount + 1) + studentsCount * (studySubjects.count * 2));
	size_t memStrNumsOffset = 0;

	char** strPtrs = (char**)calloc(studentsCount, sizeof(char*));

	for (int64_t x = 0; x < studentsCount; x++)
	{
		uint32_t size;
		struct StudentInfo studentInfo;

		DB_Get(&studentsDB, &size, Read_StudentInfo, x + 1, &studentInfo);

		strPtrs[x] = studentInfo.firstName;

		struct Cell cell2 = DEFAULT_CELL_A(1, studentCellsHeight, itoa(x + 1, memStrNums + memStrNumsOffset, 10), CenterAlignment);
		SetCell(&table, 0, x * studentCellsHeight + 1, cell2);
		memStrNumsOffset += NUMS_COUNT(x + 1) + 1;

		struct Cell cell3 = DEFAULT_CELL(1, 1, "Group");
		SetCell(&table, 1, x * studentCellsHeight + 1, cell3);
		struct Cell cell4 = DEFAULT_CELL(1, studySubjects.count, studentInfo.group);
		SetCell(&table, 1, x * studentCellsHeight + 2, cell4);

		struct Cell cell5 = DEFAULT_CELL(3, 1, "Name");
		SetCell(&table, 2, x * studentCellsHeight + 1, cell5);

		struct Cell cell6 = DEFAULT_CELL(1, 1, "First Name");
		SetCell(&table, 2, x * studentCellsHeight + 2, cell6);
		struct Cell cell7 = DEFAULT_CELL(1, 1, "Second Name");
		SetCell(&table, 3, x * studentCellsHeight + 2, cell7);
		struct Cell cell8 = DEFAULT_CELL(1, 1, "Last Name");
		SetCell(&table, 4, x * studentCellsHeight + 2, cell8);

		struct Cell cell9 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentInfo.firstName, CenteredAlignment);
		SetCell(&table, 2, x * studentCellsHeight + 3, cell9);
		struct Cell cell10 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentInfo.secondName, CenteredAlignment);
		SetCell(&table, 3, x * studentCellsHeight + 3, cell10);
		struct Cell cell11 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentInfo.lastName, CenteredAlignment);
		SetCell(&table, 4, x * studentCellsHeight + 3, cell11);


		struct Cell cell12 = DEFAULT_CELL(2, 1, "Ratings");
		SetCell(&table, 5, x * studentCellsHeight + 1, cell12);

		for (int64_t y = 0; y < studySubjects.count; y++)
		{
			struct Cell cell13 = DEFAULT_CELL(1, 1, studySubjects.names[y]);
			SetCell(&table, 5, x * studentCellsHeight + 2 + y, cell13);
			struct Cell cell14 = DEFAULT_CELL(1, 1, itoa(studentInfo.studySubjectRatings[y], memStrNums + memStrNumsOffset, 10));
			SetCell(&table, 6, x * studentCellsHeight + 2 + y, cell14);
			memStrNumsOffset += NUMS_COUNT(studentInfo.studySubjectRatings[y]) + 1;
		}
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	for (int64_t x = studentsCount - 1; x >= 0; x--) free(strPtrs[x]);
	free(strPtrs);
	free(memStrNums);
	free(cells);
}

void Command_PrintStudent(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	struct StudentInfo student;
	FindStudent(args, &student);

	if (student.firstName == 0)
	{
		printf("Student does not exist");
		return;
	}

	uint32_t studentCellHeight = MAX(3, studySubjects.count + 1);

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(6 * studentCellHeight, sizeof(struct Cell));
	struct Table table = { cells, 6 * studentCellHeight, 6, studentCellHeight, &borders };

	char* memStrNums = (char*)malloc(2 * studySubjects.count);
	size_t memStrNumsOffset = 0;

	struct Cell cell1 = DEFAULT_CELL(1, 1, "Group");
	SetCell(&table, 0, 0, cell1);
	struct Cell cell2 = DEFAULT_CELL(1, studentCellHeight - 1, student.group);
	SetCell(&table, 0, 1, cell2);

	struct Cell cell3 = DEFAULT_CELL(3, 1, "Name");
	SetCell(&table, 1, 0, cell3);

	struct Cell cell4 = DEFAULT_CELL(1, 1, "First name");
	SetCell(&table, 1, 1, cell4);
	struct Cell cell5 = DEFAULT_CELL(1, 1, "Second name");
	SetCell(&table, 2, 1, cell5);
	struct Cell cell6 = DEFAULT_CELL(1, 1, "Last name");
	SetCell(&table, 3, 1, cell6);

	struct Cell cell7 = DEFAULT_CELL(1, studentCellHeight - 2, student.firstName);
	SetCell(&table, 1, 2, cell7);
	struct Cell cell8 = DEFAULT_CELL(1, studentCellHeight - 2, student.secondName);
	SetCell(&table, 2, 2, cell8);
	struct Cell cell9 = DEFAULT_CELL(1, studentCellHeight - 2, student.lastName);
	SetCell(&table, 3, 2, cell9);

	struct Cell cell10 = DEFAULT_CELL(2, 1, "Ratings");
	SetCell(&table, 4, 0, cell10);

	for (uint32_t x = 0; x < studySubjects.count; x++)
	{
		struct Cell cell11 = DEFAULT_CELL(1, 1, studySubjects.names[x]);
		SetCell(&table, 4, 1 + x, cell11);
		struct Cell cell12 = DEFAULT_CELL(1, 1, itoa(student.studySubjectRatings[x], memStrNums + memStrNumsOffset, 10));
		SetCell(&table, 5, 1 + x, cell12);
		memStrNumsOffset += 2;
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(memStrNums);
	free(cells);
	FREE_STUDENT_INFO(student);
}

void Command_PrintStudentsFromGroup(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	size_t studGroupCount = 0;
	struct StudentInfo studentsBuffer[STUDENTS_GROUP_BUFFER_LEN];

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		uint32_t size;
		DB_Get(&studentsDB, &size, Read_StudentInfo, x, studentsBuffer + studGroupCount);

		if (strcmp((const char*)args, studentsBuffer[studGroupCount].group) == 0)
			studGroupCount++;
		else
			FREE_STUDENT_INFO(studentsBuffer[studGroupCount]);
	}

	if (studGroupCount == 0)
	{
		printf("Group does not exist");
		return;
	}

	uint32_t studentCellsHeight = MAX(3, studySubjects.count + 1);

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(6 * (studGroupCount * studentCellsHeight + 1), sizeof(struct Cell));
	struct Table table = { cells, 6 * (studGroupCount * studentCellsHeight + 1), 6, studGroupCount * studentCellsHeight + 1, &borders };

	char* group = (char*)malloc(strlen((const char*)args) + 7);
	strcpy(group, "Group ");
	strcpy(group + 6, (const char*)args);

	struct Cell cell1 = DEFAULT_CELL(6, 1, group);
	SetCell(&table, 0, 0, cell1);

	char* memStrNums = (char*)malloc(MemStrNumsSize(studGroupCount + 1) + studGroupCount * (studySubjects.count * 2));
	size_t memStrNumsOffset = 0;

	for (size_t x = 0; x < studGroupCount; x++)
	{
		struct Cell cell2 = DEFAULT_CELL_A(1, studentCellsHeight, itoa(x + 1, memStrNums + memStrNumsOffset, 10), CenterAlignment);
		SetCell(&table, 0, x * studentCellsHeight + 1, cell2);
		memStrNumsOffset += NUMS_COUNT(x + 1) + 1;

		struct Cell cell3 = DEFAULT_CELL(3, 1, "Name");
		SetCell(&table, 1, x * studentCellsHeight + 1, cell3);

		struct Cell cell4 = DEFAULT_CELL(1, 1, "First Name");
		SetCell(&table, 1, x * studentCellsHeight + 2, cell4);
		struct Cell cell5 = DEFAULT_CELL(1, 1, "Second Name");
		SetCell(&table, 2, x * studentCellsHeight + 2, cell5);
		struct Cell cell6 = DEFAULT_CELL(1, 1, "Last Name");
		SetCell(&table, 3, x * studentCellsHeight + 2, cell6);

		struct Cell cell7 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentsBuffer[x].firstName, CenteredAlignment);
		SetCell(&table, 1, x * studentCellsHeight + 3, cell7);
		struct Cell cell8 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentsBuffer[x].secondName, CenteredAlignment);
		SetCell(&table, 2, x * studentCellsHeight + 3, cell8);
		struct Cell cell9 = DEFAULT_CELL_A(1, studentCellsHeight - 2, studentsBuffer[x].lastName, CenteredAlignment);
		SetCell(&table, 3, x * studentCellsHeight + 3, cell9);


		struct Cell cell10 = DEFAULT_CELL(2, 1, "Ratings");
		SetCell(&table, 4, x * studentCellsHeight + 1, cell10);

		for (int64_t y = 0; y < studySubjects.count; y++)
		{
			struct Cell cell11 = DEFAULT_CELL(1, 1, studySubjects.names[y]);
			SetCell(&table, 4, x * studentCellsHeight + 2 + y, cell11);
			struct Cell cell12 = DEFAULT_CELL(1, 1, itoa(studentsBuffer[x].studySubjectRatings[y], memStrNums + memStrNumsOffset, 10));
			SetCell(&table, 5, x * studentCellsHeight + 2 + y, cell12);
			memStrNumsOffset += NUMS_COUNT(studentsBuffer[x].studySubjectRatings[y]) + 1;
		}
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(memStrNums);
	free(group);
	free(cells);
	for (studGroupCount--; studGroupCount > 0; studGroupCount--)
		FREE_STUDENT_INFO(studentsBuffer[studGroupCount]);
	FREE_STUDENT_INFO(studentsBuffer[0]);
}

void Command_RemoveStudent(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	int64_t id = FindStudentIndex(args);

	if (id == -1)
	{
		printf("Student does not exist");
		return;
	}

	DB_RemoveAt(&studentsDB, id);
}

void Command_AddStudent(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	struct StudentInfo student;
	student.firstName = (char*)args;
	student.secondName = student.firstName + (strlen(student.firstName) + 1);
	student.lastName = student.secondName + (strlen(student.secondName) + 1);
	student.group = student.lastName + (strlen(student.lastName) + 1);
	student.studySubjectRatings = (uint8_t*)malloc(studySubjects.count);

	memset(student.studySubjectRatings, STUDY_SUBJECT_RATING_NO_RATING_AVAILABLE, studySubjects.count);

	uint32_t size = ((student.group + strlen(student.group) + 1) - student.firstName) + studySubjects.count;

	DB_Add(&studentsDB, size, Write_StudentInfo, student, studySubjects.count);

	free(student.studySubjectRatings);
}

void Command_SetStudentRating(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	const char* firstName = (char*)args;
	const char* secondName = firstName + (strlen(firstName) + 1);
	const char* lastName = secondName + (strlen(secondName) + 1);

	int64_t x = 1;

	uint32_t size;
	struct StudentInfo student;

	for (; x < studentsDB.objectsCount; x++)
	{
		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		if ((strcmp(firstName, student.firstName) == 0) && (strcmp(secondName, student.secondName) == 0) && (strcmp(lastName, student.lastName) == 0))
			break;

		FREE_STUDENT_INFO(student);
	}

	if (x == studentsDB.objectsCount)
	{
		printf("Student does not exist");
		return;
	}

	const char* studySubjectName = lastName + (strlen(lastName) + 1);
	uint32_t studySubjectId = FindStudySubjectIndex(studySubjectName);

	if (studySubjectId == SIZE_MAX)
	{
		printf("Study subject does not exist");
		return;
	}

	student.studySubjectRatings[studySubjectId] = *((uint8_t*)(studySubjectName + (strlen(studySubjectName) + 1)));

	DB_Set(&studentsDB, size, Write_StudentInfo, x, student, studySubjects.count);

	FREE_STUDENT_INFO(student);
}

void Command_PrintStudentsWithDebts(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(2 * studentsDB.objectsCount, sizeof(struct Cell));
	struct Table table = { cells, 2, 2, 1, &borders };

	char* memStrNums = (char*)malloc((NUMS_COUNT(studySubjects.count) + 1) * (studentsDB.objectsCount - 1));
	size_t memStrNumsOffset = 0;

	struct Cell cell1 = DEFAULT_CELL(1, 1, "Student full name");
	SetCell(&table, 0, 0, cell1);
	struct Cell cell2 = DEFAULT_CELL(1, 1, "Debts count");
	SetCell(&table, 1, 0, cell2);

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		uint32_t size;
		struct StudentInfo student;

		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		uint32_t debtsCount = GetDebtsCount(&student);

		if (debtsCount == 0)
		{
			FREE_STUDENT_INFO(student);
		}
		else
		{
			*(student.secondName - 1) = ' ';
			*(student.lastName - 1) = ' ';

			struct Cell cell3 = DEFAULT_CELL(1, 1, student.firstName);
			SetCell(&table, 0, table.cellsHeight, cell3);
			struct Cell cell4 = DEFAULT_CELL(1, 1, itoa(debtsCount, memStrNums + memStrNumsOffset, 10));
			SetCell(&table, 1, table.cellsHeight, cell4);

			memStrNumsOffset += strlen(memStrNums + memStrNumsOffset) + 1;

			table.cellsHeight++;
			table.cellsLength += 2;
		}
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	for (size_t y = table.cellsHeight - 1; y > 0; y--) free(GET_CELL(table, 0, y).text);
	free(memStrNums);
	free(cells);
}

void Command_PrintGroupPerformance(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	float groupGoodStudentsCount = 0;
	float groupStudentsCount = 0;

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		uint32_t size;
		struct StudentInfo student;

		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		if (strcmp((const char*)args, student.group) == 0)
		{
			groupStudentsCount++;

			if (IsGoodStudent(&student)) groupGoodStudentsCount++;
		}

		FREE_STUDENT_INFO(student);
	}

	printf("Group %s performance: %.2f", (const char*)args, (groupGoodStudentsCount / groupStudentsCount * 100));
}

void Command_PrintBestStudySubjects(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	struct StudySubjectSumRating* sumRatings = (struct StudySubjectSumRating*)malloc(studySubjects.count * sizeof(struct StudySubjectSumRating));

	for (uint32_t x = 0; x < studySubjects.count; x++)
	{
		sumRatings[x].name = studySubjects.names[x];
		sumRatings[x].rating = 0;
	}

	for (int64_t x = 1; x < studentsDB.objectsCount; x++)
	{
		uint32_t size;
		struct StudentInfo student;

		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		for (uint32_t y = 0; y < studySubjects.count; y++)
			sumRatings[y].rating += student.studySubjectRatings[y];

		FREE_STUDENT_INFO(student);
	}

	qsort(sumRatings, studySubjects.count, sizeof(struct StudySubjectSumRating), Compare_StudySubjectSumRating);

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(2 * studySubjects.count + 2, sizeof(struct Cell));
	struct Table table = { cells, 2 * studySubjects.count + 2, 2, studySubjects.count + 1, &borders };

	char* memStrNums = (char*)malloc((NUMS_COUNT(UINT32_MAX) + 1) * studySubjects.count);
	size_t memStrNumsOffset = 0;

	struct Cell cell1 = DEFAULT_CELL(1, 1, "Study subject");
	SetCell(&table, 0, 0, cell1);
	struct Cell cell2 = DEFAULT_CELL(1, 1, "Summary rating");
	SetCell(&table, 1, 0, cell2);

	for (uint32_t x = 0; x < studySubjects.count; x++)
	{
		struct Cell cell3 = DEFAULT_CELL(1, 1, (char*)sumRatings[x].name);
		SetCell(&table, 0, x + 1, cell3);
		struct Cell cell4 = DEFAULT_CELL(1, 1, itoa(sumRatings[x].rating, memStrNums + memStrNumsOffset, 10));
		SetCell(&table, 1, x + 1, cell4);
		memStrNumsOffset += strlen(memStrNums + memStrNumsOffset) + 1;
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(memStrNums);
	free(cells);
	free(sumRatings);
}

void Command_PrintStudySubjects(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects");
		return;
	}

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(studySubjects.count + 1, sizeof(struct Cell));
	struct Table table = { cells, studySubjects.count + 1, 1, studySubjects.count + 1, &borders };

	struct Cell cell1 = DEFAULT_CELL(1, 1, "Study subjects");
	SetCell(&table, 0, 0, cell1);

	for (uint32_t x = 0; x < studySubjects.count; x++)
	{
		struct Cell cell2 = DEFAULT_CELL(1, 1, studySubjects.names[x]);
		SetCell(&table, 0, x + 1, cell2);
	}

	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(cells);
}

void Command_PrintGroups(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	if (studySubjects.count == 0)
	{
		printf("There are no study subjects in the database");
		return;
	}

	if (studentsDB.objectsCount < 2)
	{
		printf("There are no students in the database");
		return;
	}

	size_t memStrSize = 25600;
	size_t memStrOffset = 0;
	size_t strsCount = 0;

	char* memStr = (char*)malloc(memStrSize);

	uint32_t size;
	struct StudentInfo student;

	DB_Get(&studentsDB, &size, Read_StudentInfo, 1, &student);

	strcpy(memStr, student.group);

	memStrOffset += strlen(memStr) + 1;
	strsCount++;

	FREE_STUDENT_INFO(student);

	for (int64_t x = 2; x < studentsDB.objectsCount; x++)
	{
		DB_Get(&studentsDB, &size, Read_StudentInfo, x, &student);

		if (IsUniqueStr(memStr, strsCount, student.group))
		{
			strcpy(memStr + memStrOffset, student.group);

			memStrOffset += strlen(memStr + memStrOffset) + 1;
			strsCount++;

			if (memStrOffset >= memStrSize)
			{
				memStrSize += 25600;
				memStr = (char*)realloc(memStr, memStrSize);
			}
		}

		FREE_STUDENT_INFO(student);
	}

	memStr = (char*)realloc(memStr, memStrOffset);

	struct TableBorders borders = DEFAULT_BORDERS;
	struct Cell* cells = (struct Cell*)calloc(strsCount + 1, sizeof(struct Cell));
	struct Table table = { cells, strsCount + 1, 1, strsCount + 1, &borders };

	struct Cell cell1 = DEFAULT_CELL(1, 1, "Groups");
	SetCell(&table, 0, 0, cell1);

	memStrOffset = 0;

	for (size_t x = 0; x < strsCount; x++, memStrOffset += strlen(memStr + memStrOffset) + 1)
	{
		struct Cell cell2 = DEFAULT_CELL(1, 1, memStr + memStrOffset);
		SetCell(&table, 0, x + 1, cell2);
	}
	
	struct TableString tableString = TableToString(&table);

	printf("%s", tableString.tableString);

	free(tableString.tableString);
	free(cells);
	free(memStr);
}

void Command_CreateDB(void* args)
{
	if (dbIsOpen)
	{
		printf("There is already an open database");
		return;
	}

	int openRes = DB_Create(&studentsDB, (const char*)args);

	if (openRes == 1)
	{
		printf("Can not create database");
		return;
	}

	studySubjects.count = 0;

	uint32_t size = sizeof(uint32_t);

	long pos = ftell(studentsDB.stream);

	fseek(studentsDB.stream, 3 * sizeof(uint32_t), SEEK_CUR);

	printf("Print study subject names(to finish, enter end):\n");

	while (1)
	{
		char str[256];

		scanf("%s", str);

		if (strcmp("end", str) == 0) break;

		size_t len = strlen(str) + 1;

		fwrite(str, len, 1, studentsDB.stream);

		size += len;

		studySubjects.count++;
	}

	fseek(studentsDB.stream, pos, SEEK_SET);

	fwrite(&size, sizeof(uint32_t), 1, studentsDB.stream);
	fwrite(&size, sizeof(uint32_t), 1, studentsDB.stream);
	fwrite(&studySubjects.count, sizeof(uint32_t), 1, studentsDB.stream);

	fseek(studentsDB.stream, pos, SEEK_SET);

	studentsDB.fileSize += 2 * sizeof(uint32_t) + size;
	studentsDB.fakeFileSize += 2 * sizeof(uint32_t) + size;
	studentsDB.objectsCount++;
	studentsDB.readReady = 1;
	studentsDB.writeReady = 1;

	DB_Get(&studentsDB, &size, Read_StudySubjects, 0, &studySubjects);

	dbIsOpen = 1;
}

void Command_OpenDB(void* args)
{
	if (dbIsOpen)
	{
		printf("There is already an open database");
		return;
	}

	int openRes = DB_Open(&studentsDB, (const char*)args);

	if (openRes == 1)
	{
		printf("\nCan not open database\n");
		return;
	}

	uint32_t studySubjectsSize;
	DB_Get(&studentsDB, &studySubjectsSize, Read_StudySubjects, 0, &studySubjects);

	dbIsOpen = 1;
}

void Command_CloseDB(void* args)
{
	if (!dbIsOpen)
	{
		printf("No open database");
		return;
	}

	DB_Close(&studentsDB);
	FREE_STUDY_SUBJECTS(studySubjects);

	dbIsOpen = 0;
}

void Command_CheckDB(void* args)
{
	printf("%s", DB_FORMAT_ERROR_ToString(DB_IsInvalidFormat((const char*)args)));
}