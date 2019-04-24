#pragma once

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

//Список учебный предметов
struct StudySubjects
{
	uint32_t count; //Количество учебных предметов

	char** names;   //Массив названий учебных предметов
};

//Информация о студенте
struct StudentInfo
{
	char* firstName;  //Имя
	char* secondName; //Фамилия
	char* lastName;   //Отчество

	char* group; //Название группы

	uint8_t* studySubjectRatings; //Массив оценок, длиною в количество учебных предметов
};

void Read_StudySubjects(FILE* stream, uint32_t size, va_list args);  //Функция, считывающая из потока структуру StudySubjects, для передачи в DB_Read
void Write_StudySubjects(FILE* stream, uint32_t size, va_list args); //Функция, записывающая в поток структуру StudySubjects, для передачи в DB_Add или DB_Set

void Read_StudentInfo(FILE* stream, uint32_t size, va_list args);  //Функция, считывающая из потока структуру StudentInfo, для передачи в DB_Read
void Write_StudentInfo(FILE* stream, uint32_t size, va_list args); //Функция, записывающая в поток структуру StudentInfo, для передачи в DB_Add или DB_Set