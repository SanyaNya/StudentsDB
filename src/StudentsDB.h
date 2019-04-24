#ifndef STUDENTS_DB_H
#define STUDENTS_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "FileDatabase.h"

#define FREE_STUDY_SUBJECTS(SS) free((SS).names);
#define FREE_STUDENT_INFO(SI) free((SI).firstName);

#define STUDY_SUBJECTS_MAX_COUNT 512
#define STUDY_SUBJECT_NAME_MAX_LEN 128

#define STUDY_SUBJECT_RATING_NO_RATING_AVAILABLE 0
#define STUDY_SUBJECT_RATING_ACDEMIC_DEBT 1

#define STUDENTS_GROUP_BUFFER_LEN 256

extern struct DB_Info studentsDB;
extern struct StudySubjects studySubjects;

extern int dbIsOpen;

//Ñïèñîê ó÷åáíûé ïðåäìåòîâ
struct StudySubjects
{
	uint32_t count; //Êîëè÷åñòâî ó÷åáíûõ ïðåäìåòîâ

	char** names;   //Ìàññèâ íàçâàíèé ó÷åáíûõ ïðåäìåòîâ
};

//Èíôîðìàöèÿ î ñòóäåíòå
struct StudentInfo
{
	char* firstName;  //Èìÿ
	char* secondName; //Ôàìèëèÿ
	char* lastName;   //Îò÷åñòâî

	char* group; //Íàçâàíèå ãðóïïû

	uint8_t* studySubjectRatings; //Ìàññèâ îöåíîê, äëèíîþ â êîëè÷åñòâî ó÷åáíûõ ïðåäìåòîâ
};

void Read_StudySubjects(FILE* stream, uint32_t size, va_list args);  //Ôóíêöèÿ, ñ÷èòûâàþùàÿ èç ïîòîêà ñòðóêòóðó StudySubjects, äëÿ ïåðåäà÷è â DB_Read
void Write_StudySubjects(FILE* stream, uint32_t size, va_list args); //Ôóíêöèÿ, çàïèñûâàþùàÿ â ïîòîê ñòðóêòóðó StudySubjects, äëÿ ïåðåäà÷è â DB_Add èëè DB_Set

void Read_StudentInfo(FILE* stream, uint32_t size, va_list args);  //Ôóíêöèÿ, ñ÷èòûâàþùàÿ èç ïîòîêà ñòðóêòóðó StudentInfo, äëÿ ïåðåäà÷è â DB_Read
void Write_StudentInfo(FILE* stream, uint32_t size, va_list args); //Ôóíêöèÿ, çàïèñûâàþùàÿ â ïîòîê ñòðóêòóðó StudentInfo, äëÿ ïåðåäà÷è â DB_Add èëè DB_Set

#endif
