#pragma once

struct Command
{
	const char* name; //�������� �������
	const int argsCount; //���������� ����������
	const char** argFormats; //������ �������� ����������
	void(*func)(void*); //��������� �� ����������� �������, � ������� ���������� argsBuffer, ���������� � ���� ���������
	const char* funcDescription; //�������� �������
	const char** argsDescription; //������ �������� ����������
};

extern size_t commandsCount;
extern struct Command* commands;
extern int exitFlag;

//���������� ������ ������� � ������ ������
//� ������ ���� �������� �� ������ � ������ ������ ������������ SIZE_MAX
size_t FindCommand(const char* name);

//������������� �������
//cmnds - ������ ������
//cmndsCount - ���������� ������
//nameBuffer - ������ ��� ���������� �������� �������
//argsBuffer - ������ ��� ���������� ���������� �������
void InitCommands(struct Command* cmnds, size_t cmndsCount, char* nameBuffer, char* argsBuffer);

//��������� ������� � �������� ��������� � ��� �������
int ScanAndRunCommand();

void ConsoleLoop();