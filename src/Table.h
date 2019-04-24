#ifndef TABLE_H
#define TABLE_H

#define EMPTY_CELL { {1, 1}, NoRemoveBorders, {LeftAlignment, 0,0,0,0}, "" }
#define DEFAULT_CELL(WIDTH, HEIGHT, TEXT) { {WIDTH, HEIGHT}, NoRemoveBorders,{ CenterAlignment, 0,0,0,0 }, TEXT }
#define DEFAULT_CELL_A(WIDTH, HEIGHT, TEXT, ALIGNMENT) { {WIDTH, HEIGHT}, NoRemoveBorders,{ ALIGNMENT, 0,0,0,0 }, TEXT }
#define DEFAULT_BORDERS { ' ', 196, 179, 218, 191, 217, 192, 193, 194, 180, 195, 197 }

#define GET_CELL(TABLE, X, Y) (TABLE).cells[(X) + (Y) * ((TABLE).cellsWidth)]

struct Size
{
	size_t width;
	size_t height;
};

enum TextAlignment
{
	LeftAlignment,
	CenteredAlignment,
	RightAlignment,

	CenterAlignment
};

enum BordersMode
{
	NoRemoveBorders,

	RemoveLeftUpBorder,
	RemoveLeftBorder,
	RemoveUpBorder
};

struct CellStyle
{
	enum TextAlignment cellTextAlignment;

	size_t indentLeft;
	size_t indentRight;
	size_t indentUp;
	size_t indentDown;
};

struct TableBorders
{
	char empty;

	char hLine;
	char vLine;

	char angleRightDown;
	char angleLeftDown;
	char angleLeftUp;
	char angleRightUp;

	char hLineUp;
	char hLineDown;
	char vLineLeft;
	char vLineRight;

	char cross;
};

struct Cell
{
	struct Size size;

	enum BordersMode bordersMode;

	struct CellStyle style;

	char* text;
};

struct Table
{
	struct Cell* cells;

	size_t cellsLength;
	size_t cellsWidth, cellsHeight;

	struct TableBorders* borders_ptr;
};

struct TableString
{
	char* tableString;

	size_t length;
	size_t width, height;
};


//Âîçâðàùàåò òàáëèöó â âèäå ñòðîêè
//Ïîä TableString.tableString âûäåëÿåòñÿ ïàìÿòü
struct TableString TableToString(struct Table* table_ptr);

//Çàïèñûâàåò ÿ÷åéêó ïî îïðåäåëåííûì êîîðäèíàòàì è ðàññòàâëÿåò ïóñòûå ÿ÷åéêè, åñëè ýòî ìóëüòèÿ÷åéêà
void SetCell(struct Table* table_ptr, size_t x, size_t y, struct Cell cell);

#endif
