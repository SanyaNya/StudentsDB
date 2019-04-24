#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "Commands.h"

size_t commandsCount;
struct Command* commands;
int exitFlag = 0;

static char* _funcNameBuffer;
static char* _funcArgsBuffer;

static size_t _GetSizeOfFormat(char* buf, const char* format)
{
	if (format[0] != '%') return 0;

	switch (format[1])
	{
	case 'd':
	case 'i':
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	case 'a':
	case 'o':
	case 'x':
	case 'X':
	case 'u':
	case 'b':
		return sizeof(int);
		break;

	case 'c':
		return sizeof(char);
		break;

	case 's':
		return strlen(buf) + 1;
		break;

	case 'p':
		return sizeof(void*);
		break;
	}

	return 0;
}

size_t FindCommand(const char* name)
{
	for (size_t x = 0; x < commandsCount; x++)
		if (strcmp(name, commands[x].name) == 0)
			return x;

	return SIZE_MAX;
}

void InitCommands(struct Command* cmnds, size_t cmndsCount, char* nameBuffer, char* argsBuffer)
{
	commands = cmnds;
	commandsCount = cmndsCount;
	_funcNameBuffer = nameBuffer;
	_funcArgsBuffer = argsBuffer;
}

int ScanAndRunCommand()
{
	scanf("%s", _funcNameBuffer);

	size_t commandId = FindCommand(_funcNameBuffer);

	if (commandId == SIZE_MAX) return 1;

	if (commands[commandId].argsCount > 0)
	{
		size_t bufOffset = 0;

		for (int x = 0; x < commands[commandId].argsCount; x++)
		{
			scanf(commands[commandId].argFormats[x], _funcArgsBuffer + bufOffset);

			size_t size = _GetSizeOfFormat(_funcArgsBuffer + bufOffset, commands[commandId].argFormats[x]);

			if (size == 0) return 2;

			bufOffset += size;
		}

		commands[commandId].func(_funcArgsBuffer);
	}
	else
	{
		commands[commandId].func(NULL);
	}

	return 0;
}

void ConsoleLoop()
{
	while (!exitFlag)
	{
		printf("\n>");
		int res = ScanAndRunCommand();

		if (res == 1) printf("\nCommand does not exist");
		else if (res == 2) printf("\nInvalid arguments");
	}
}