#ifndef COMMANDS_H
#define COMMANDS_H

struct Command
{
	const char* name; //Íàçâàíèå êîìàíäû
	const int argsCount; //Êîëè÷åñòâî àðãóìåíòîâ
	const char** argFormats; //Ìàññèâ ôîðìàòîâ àðãóìåíòîâ
	void(*func)(void*); //Óêàçàòåëü íà èñïîëíÿåìóþ ôóíêöèþ, â êîòîðóþ ïåðåäàåòñÿ argsBuffer, ñîäåðæàùèé â ñåáå àðãóìåíòû
	const char* funcDescription; //Îïèñàíèå ôóíêöèè
	const char** argsDescription; //Ìàññèâ îïèñàíèé àðãóìåíòîâ
};

extern size_t commandsCount;
extern struct Command* commands;
extern int exitFlag;

//Âîçâðàùàåò èíäåêñ êîìàíäû â ñïèñêå êîìàíä
//Â ñëó÷àå åñëè êöîìàíäà íå âõîäèò â ñïèñîê êîìàíä âîçâðàùàåòñÿ SIZE_MAX
size_t FindCommand(const char* name);

//Èíèöèàëèçàöèÿ êîììàíä
//cmnds - ìàññèâ êîìàíä
//cmndsCount - êîëè÷åñòâî êîìàíä
//nameBuffer - áóôôåð äëÿ ñ÷èòûâàíèÿ íàçâàíèÿ êîìàíäû
//argsBuffer - áóôôåð äëÿ ñ÷èòûâàíèÿ àðãóìåíòîâ êîìàíäû
void InitCommands(struct Command* cmnds, size_t cmndsCount, char* nameBuffer, char* argsBuffer);

//Ñ÷èòûâàåò êîìàíäó è âûçûâàåò ñâÿçàííóþ ñ íåé ôóíêöèþ
int ScanAndRunCommand();

void ConsoleLoop();

#endif
