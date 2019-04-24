#include <stdlib.h>
#include <string.h>
#include "StudentsDB.h"

struct DB_Info studentsDB;
struct StudySubjects studySubjects;

int dbIsOpen = 0;

void Read_StudySubjects(FILE* stream, uint32_t size, va_list args)
{
	fread(&((*(struct StudySubjects**)args)->count), sizeof(uint32_t), 1, stream);

	size_t strsSize = size - sizeof(uint32_t);
	size_t ptrsSize = sizeof(char*) * (**(struct StudySubjects**)args).count;

	char* mem = (char*)malloc(ptrsSize + strsSize);

	fread(mem + ptrsSize, strsSize, 1, stream);

	for (size_t x = ptrsSize, y = 0; x < ptrsSize + strsSize; x++, y++)
	{
		((char**)mem)[y] = mem + x;

		for (; mem[x] != 0; x++) {}
	}

	(*(struct StudySubjects**)args)->names = (char**)mem;
}
void Write_StudySubjects(FILE* stream, uint32_t size, va_list args)
{
	fwrite(&(*(struct StudySubjects*)args).count, sizeof(uint32_t), 1, stream);

	for (uint32_t x = 0; x < (*(struct StudySubjects*)args).count; x++)
	{
		for (uint32_t y = 0; (*(struct StudySubjects*)args).names[x][y] != 0; y++)
			fputc((*(struct StudySubjects*)args).names[x][y], stream);
		fputc(0, stream);
	}
}

void Read_StudentInfo(FILE* stream, uint32_t size, va_list args)
{
	char* mem = (char*)malloc(size);

	fread(mem, size, 1, stream);

	(*(struct StudentInfo**)args)->firstName = mem;
	mem += strlen((*(struct StudentInfo**)args)->firstName) + 1;

	(*(struct StudentInfo**)args)->secondName = mem;
	mem += strlen((*(struct StudentInfo**)args)->secondName) + 1;

	(*(struct StudentInfo**)args)->lastName = mem;
	mem += strlen((*(struct StudentInfo**)args)->lastName) + 1;

	(*(struct StudentInfo**)args)->group = mem;
	mem += strlen((*(struct StudentInfo**)args)->group) + 1;

	(*(struct StudentInfo**)args)->studySubjectRatings = (uint8_t*)mem;
}
void Write_StudentInfo(FILE* stream, uint32_t size, va_list args)
{
	for (size_t x = 0; ((struct StudentInfo*)args)->firstName[x] != 0; x++)
		fputc(((struct StudentInfo*)args)->firstName[x], stream);
	fputc(0, stream);

	for (size_t x = 0; ((struct StudentInfo*)args)->secondName[x] != 0; x++)
		fputc(((struct StudentInfo*)args)->secondName[x], stream);
	fputc(0, stream);

	for (size_t x = 0; ((struct StudentInfo*)args)->lastName[x] != 0; x++)
		fputc(((struct StudentInfo*)args)->lastName[x], stream);
	fputc(0, stream);

	for (size_t x = 0; ((struct StudentInfo*)args)->group[x] != 0; x++)
		fputc(((struct StudentInfo*)args)->group[x], stream);
	fputc(0, stream);

	fwrite(((struct StudentInfo*)args)->studySubjectRatings, *((uint32_t*)(args + sizeof(struct StudentInfo))), 1, stream);
}