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

//������ ������� ���������
struct StudySubjects
{
	uint32_t count; //���������� ������� ���������

	char** names;   //������ �������� ������� ���������
};

//���������� � ��������
struct StudentInfo
{
	char* firstName;  //���
	char* secondName; //�������
	char* lastName;   //��������

	char* group; //�������� ������

	uint8_t* studySubjectRatings; //������ ������, ������ � ���������� ������� ���������
};

void Read_StudySubjects(FILE* stream, uint32_t size, va_list args);  //�������, ����������� �� ������ ��������� StudySubjects, ��� �������� � DB_Read
void Write_StudySubjects(FILE* stream, uint32_t size, va_list args); //�������, ������������ � ����� ��������� StudySubjects, ��� �������� � DB_Add ��� DB_Set

void Read_StudentInfo(FILE* stream, uint32_t size, va_list args);  //�������, ����������� �� ������ ��������� StudentInfo, ��� �������� � DB_Read
void Write_StudentInfo(FILE* stream, uint32_t size, va_list args); //�������, ������������ � ����� ��������� StudentInfo, ��� �������� � DB_Add ��� DB_Set