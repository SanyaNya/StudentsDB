#pragma once

struct Command
{
	const char* name; //Название команды
	const int argsCount; //Количество аргументов
	const char** argFormats; //Массив форматов аргументов
	void(*func)(void*); //Указатель на исполняемую функцию, в которую передается argsBuffer, содержащий в себе аргументы
	const char* funcDescription; //Описание функции
	const char** argsDescription; //Массив описаний аргументов
};

extern size_t commandsCount;
extern struct Command* commands;
extern int exitFlag;

//Возвращает индекс команды в списке команд
//В случае если кцоманда не входит в список команд возвращается SIZE_MAX
size_t FindCommand(const char* name);

//Инициализация комманд
//cmnds - массив команд
//cmndsCount - количество команд
//nameBuffer - буффер для считывания названия команды
//argsBuffer - буффер для считывания аргументов команды
void InitCommands(struct Command* cmnds, size_t cmndsCount, char* nameBuffer, char* argsBuffer);

//Считывает команду и вызывает связанную с ней функцию
int ScanAndRunCommand();

void ConsoleLoop();