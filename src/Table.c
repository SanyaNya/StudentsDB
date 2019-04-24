#include <stdlib.h>
#include "Table.h"

//Возвращает длину строки, end - символ конца строки
size_t strLengthCustom(char* str, char end)
{
	size_t len = 0;

	for (; str[len] != end && str[len] != 0; len++) {}

	return len;
}

//Возвращает размер строки
struct Size strSize(char* str)
{
	struct Size size;
	size.width = 0;
	size.height = 1;

	for (size_t x = 0, prevPos = -1; ; x++)
	{
		if (str[x] == '\n')
		{
			size_t width = x - prevPos - 1;
			if (width > size.width) size.width = width;
			size.height++;
			prevPos = x;
		}
		else if (str[x] == 0)
		{
			size_t width = x - prevPos - 1;
			if (width > size.width) size.width = width;
			break;
		}
	}

	return size;
}


//Заполняет строку таблицы пустыми символами
static void _ClearTableString(struct TableString* tableString_ptr, char empty)
{
	for (size_t x = 0; x < tableString_ptr->length; x++)
		tableString_ptr->tableString[x] = empty;
}

//Выделяет память под строку таблицы
static void _AllocTableString(struct TableString* tableString_ptr, struct Table* table_ptr, struct Size* textWithIndentsSizes)
{
	tableString_ptr->width = table_ptr->cellsWidth + 2;
	tableString_ptr->height = table_ptr->cellsHeight + 1;

	for (size_t x = 0; x < table_ptr->cellsWidth; x++)
		tableString_ptr->width += textWithIndentsSizes[x].width;

	for (size_t y = 0; y < table_ptr->cellsLength; y += table_ptr->cellsWidth)
		tableString_ptr->height += textWithIndentsSizes[y].height;

	tableString_ptr->length = tableString_ptr->width * tableString_ptr->height;

	tableString_ptr->tableString = (char*)malloc(sizeof(char) * tableString_ptr->length);

	_ClearTableString(tableString_ptr, table_ptr->borders_ptr->empty);
}


//Возвращает размер текста ячвейки с отступами
static void _SetBorderlessCellSize(struct Size* size_ptr, struct Cell* cell_ptr)
{
	(*size_ptr) = strSize(cell_ptr->text);

	(*size_ptr).width += cell_ptr->style.indentLeft + cell_ptr->style.indentRight;
	(*size_ptr).height += cell_ptr->style.indentUp + cell_ptr->style.indentDown;
}

//Возвращает массив размеров текста с отступами, соответствующий ячейкам
//Под результат выделяется память
static struct Size* _BorderlessCellSizes(struct Table* table_ptr)
{
	struct Size* result = (struct Size*)malloc(table_ptr->cellsLength * sizeof(struct Size));

	for (size_t x = 0; x < table_ptr->cellsLength; x++)
		_SetBorderlessCellSize(result + x, table_ptr->cells + x);

	return result;
}


//Увеличивает ширину ячейки до maxWidth с помощью увеличения отступов
//Размер меняется
static void _AlignCellWidth(struct Cell* cell_ptr, struct Size* size_ptr, size_t maxWidth)
{
	size_t r = maxWidth - size_ptr->width;

	cell_ptr->style.indentLeft += r / 2;
	cell_ptr->style.indentRight += r - r / 2;

	/* switch(_CELL_PTR_BORDERS_MODE(cell_ptr))
	{
	case NoRemoveBorders:
	_CELL_PTR_STYLE(cell_ptr).indentLeft += r / 2;
	_CELL_PTR_STYLE(cell_ptr).indentRight += r - r / 2;
	break;

	case RemoveLeftBorder:
	_CELL_PTR_STYLE(cell_ptr).indentRight += r + 1;
	break;

	case RemoveLeftUpBorder:
	_CELL_PTR_STYLE(cell_ptr).indentRight += r + 1;
	break;
	}*/

	size_ptr->width = maxWidth;
}

//Увеличивает высоту ячейки до maxHeight с помощью увеличения отступов
//Размер меняется
static void _AlignCellHeight(struct Cell* cell_ptr, struct Size* size_ptr, size_t maxHeight)
{
	size_t r = maxHeight - size_ptr->height;

	cell_ptr->style.indentUp += r / 2;
	cell_ptr->style.indentDown += r - r / 2;

	/* switch(_CELL_PTR_BORDERS_MODE(cell_ptr))
	{
	case NoRemoveBorders:
	_CELL_PTR_STYLE(cell_ptr).indentUp += r / 2;
	_CELL_PTR_STYLE(cell_ptr).indentDown += r - r / 2;
	break;

	case RemoveUpBorder:
	_CELL_PTR_STYLE(cell_ptr).indentDown += r + 1;
	break;

	case RemoveLeftUpBorder:
	_CELL_PTR_STYLE(cell_ptr).indentDown += r + 1;
	break;
	}*/

	size_ptr->height = maxHeight;
}

//Находит ячейку с максимальной шириной и приравнивает к ней остальные ячейки столбца
//Размеры меняются
static void _AlignColumn(struct Table* table_ptr, struct Size* textWithIndentsSizes, size_t columnIndex)
{
	size_t maxWidth = 0;

	for (size_t strIndex = 0; strIndex < table_ptr->cellsHeight; strIndex++)
		if (textWithIndentsSizes[columnIndex + strIndex * table_ptr->cellsWidth].width > maxWidth) maxWidth = textWithIndentsSizes[columnIndex + strIndex * table_ptr->cellsWidth].width;

	for (size_t strIndex = 0; strIndex < table_ptr->cellsHeight; strIndex++)
		_AlignCellWidth(table_ptr->cells + columnIndex + strIndex * table_ptr->cellsWidth, textWithIndentsSizes + (columnIndex + strIndex * table_ptr->cellsWidth), maxWidth);
}

//Находит ячейку с максимальной высотой и приравнивает к ней остальные ячейки строки
//Размеры меняются
static void _AlignStr(struct Table* table_ptr, struct Size* textWithIndentsSizes, size_t strIndex)
{
	size_t maxHeight = 0;

	for (size_t columnIndex = 0; columnIndex < table_ptr->cellsWidth; columnIndex++)
		if (textWithIndentsSizes[columnIndex + strIndex * table_ptr->cellsWidth].height > maxHeight) maxHeight = textWithIndentsSizes[columnIndex + strIndex * table_ptr->cellsWidth].height;

	for (size_t columnIndex = 0; columnIndex < table_ptr->cellsWidth; columnIndex++)
		_AlignCellHeight(table_ptr->cells + (columnIndex + strIndex * table_ptr->cellsWidth), textWithIndentsSizes + (columnIndex + strIndex * table_ptr->cellsWidth), maxHeight);
}

//Выравнивает размеры ячеек по ширине и высоте
//Размеры меняются
static void _AlignCells(struct Table* table_ptr, struct Size* textWithIndentsSizes)
{
	for (size_t columnIndex = 0; columnIndex < table_ptr->cellsWidth; columnIndex++) _AlignColumn(table_ptr, textWithIndentsSizes, columnIndex);
	for (size_t strIndex = 0; strIndex < table_ptr->cellsHeight; strIndex++) _AlignStr(table_ptr, textWithIndentsSizes, strIndex);
}


#define _RLEFT(leftCell_ptr) (leftCell_ptr)->bordersMode == RemoveUpBorder || (leftCell_ptr)->bordersMode == RemoveLeftUpBorder
#define _RRIGHT(cell_ptr) (cell_ptr)->bordersMode == RemoveUpBorder || (cell_ptr)->bordersMode == RemoveLeftUpBorder
#define _RUP(upCell_ptr) (upCell_ptr)->bordersMode == RemoveLeftBorder || (upCell_ptr)->bordersMode == RemoveLeftUpBorder
#define _RDOWN(cell_ptr) (cell_ptr)->bordersMode == RemoveLeftBorder || (cell_ptr)->bordersMode == RemoveLeftUpBorder

//Возвращает уголок в зависимости от наличия границ
static char _GetAngle(struct TableBorders* borders_ptr, unsigned char left, unsigned char right, unsigned char up, unsigned char down)
{
	switch ((left << 3) + (right << 2) + (up << 1) + down)
	{
	case 0: //0000
		return borders_ptr->cross;
		break;

	case 1: //0001
		return borders_ptr->hLineUp;
		break;

	case 2: //0010
		return borders_ptr->hLineDown;
		break;

	case 3: //0011
		return borders_ptr->hLine;
		break;

	case 4: //0100
		return borders_ptr->vLineLeft;
		break;

	case 5: //0101
		return borders_ptr->angleLeftUp;
		break;

	case 6: //0110
		return borders_ptr->angleLeftDown;
		break;

	case 7: //0111
		return borders_ptr->empty;
		break;

	case 8: //1000
		return borders_ptr->vLineRight;
		break;

	case 9: //1001
		return borders_ptr->angleRightUp;
		break;

	case 10: //1010
		return borders_ptr->angleRightDown;
		break;

	case 11: //1011
		return borders_ptr->empty;
		break;

	case 12: //1100
		return borders_ptr->vLine;
		break;

	case 13: //1101
		return borders_ptr->empty;
		break;

	case 14: //1110
		return borders_ptr->empty;
		break;

	case 15: //1111
		return borders_ptr->empty;
		break;
	}

	return 0;
}


//Заполняет левую верхнюю границу ячейки
static void _FillCellLeftUpBorder_Default(struct TableString* tableString_ptr, struct Cell* cell_ptr, struct TableBorders* borders_ptr, struct Size* cellTextSizeWithIndents_ptr, size_t* posInTableStr, char angle)
{
	tableString_ptr->tableString[(*posInTableStr)] = angle;

	for (size_t x = 0; x < cellTextSizeWithIndents_ptr->width; x++)
		tableString_ptr->tableString[(*posInTableStr) + 1 + x] = borders_ptr->hLine;

	for (size_t y = 1; y <= cellTextSizeWithIndents_ptr->height; y++)
		tableString_ptr->tableString[(*posInTableStr) + y * tableString_ptr->width] = borders_ptr->vLine;
}
static void _FillCellLeftUpBorder_RemoveLeftBorder(struct TableString* tableString_ptr, struct Cell* cell_ptr, struct TableBorders* borders_ptr, struct Size* cellTextSizeWithIndents_ptr, size_t* posInTableStr, char angle)
{
	tableString_ptr->tableString[(*posInTableStr)] = angle;

	for (size_t x = 0; x < cellTextSizeWithIndents_ptr->width; x++)
		tableString_ptr->tableString[(*posInTableStr) + 1 + x] = borders_ptr->hLine;
}
static void _FillCellLeftUpBorder_RemoveUpBorder(struct TableString* tableString_ptr, struct Cell* cell_ptr, struct TableBorders* borders_ptr, struct Size* cellTextSizeWithIndents_ptr, size_t* posInTableStr, char angle)
{
	tableString_ptr->tableString[(*posInTableStr)] = angle;

	for (size_t y = 1; y <= cellTextSizeWithIndents_ptr->height; y++)
		tableString_ptr->tableString[(*posInTableStr) + y * tableString_ptr->width] = borders_ptr->vLine;
}

static void _FillCellLeftUpBorder(struct TableString* tableString_ptr, struct Cell* cell_ptr, struct TableBorders* borders_ptr, struct Size* cellTextSizeWithIndents_ptr, size_t* posInTableStr, char angle)
{
	switch (cell_ptr->bordersMode)
	{
	case NoRemoveBorders:
		_FillCellLeftUpBorder_Default(tableString_ptr, cell_ptr, borders_ptr, cellTextSizeWithIndents_ptr, posInTableStr, angle);
		break;

	case RemoveLeftBorder:
		_FillCellLeftUpBorder_RemoveLeftBorder(tableString_ptr, cell_ptr, borders_ptr, cellTextSizeWithIndents_ptr, posInTableStr, angle);
		break;

	case RemoveUpBorder:
		_FillCellLeftUpBorder_RemoveUpBorder(tableString_ptr, cell_ptr, borders_ptr, cellTextSizeWithIndents_ptr, posInTableStr, angle);
		break;

	default: break;
	}

	(*posInTableStr) += tableString_ptr->width + 1;
}


static void _FillCellText(struct TableString* tableString_ptr, char* text, size_t posInTableStr, enum TextAlignment textAlignment, size_t cellWidth, size_t cellHeight)
{
	size_t textPos = 0;

	size_t offset;

	struct Size textSize = strSize(text);

	size_t width = textSize.width;
	size_t height = textSize.height;

	size_t p;

	switch (textAlignment)
	{
	case LeftAlignment: break;
	case CenteredAlignment: posInTableStr += (cellWidth - width) / 2;  break;
	case RightAlignment: posInTableStr += (cellWidth - width); break;

	case CenterAlignment:
		posInTableStr += (cellWidth - width) / 2;
		posInTableStr += ((cellHeight - height) / 2) * tableString_ptr->width;
	}

	p = textAlignment != 3 ? textAlignment : 1;

	while (1)
	{
		offset = p * (width - strLengthCustom(text + textPos, '\n')) / 2;

		for (size_t x = 0; ; textPos++, x++)
		{
			if (text[textPos] == '\n') break;
			else if (text[textPos] == 0) return;

			tableString_ptr->tableString[posInTableStr + x + offset] = text[textPos];
		}
		posInTableStr += tableString_ptr->width;
		textPos++;
	}
}


//Заполняет левую и верхнюю границу ячейки и её текст
static void _FillCell(struct TableString* tableString_ptr, struct Cell* cell_ptr, struct TableBorders* borders_ptr, struct Size* cellTextSizeWithIndents_ptr, size_t posInTableStr, char angle)
{
	_FillCellLeftUpBorder(tableString_ptr, cell_ptr, borders_ptr, cellTextSizeWithIndents_ptr, &posInTableStr, angle);
	_FillCellText(tableString_ptr, cell_ptr->text, posInTableStr, cell_ptr->style.cellTextAlignment, cellTextSizeWithIndents_ptr->width, cellTextSizeWithIndents_ptr->height);
}


//Заполняет правую и нижнюю границу таблицы
static void _FillRightDownBorder(struct Table* table_ptr, struct TableString* tableString_ptr, struct Size* cellTextSizesWithIndents)
{
	tableString_ptr->tableString[tableString_ptr->width - 2] = table_ptr->borders_ptr->angleLeftDown;

	tableString_ptr->tableString[tableString_ptr->width * tableString_ptr->height - 2] = table_ptr->borders_ptr->angleLeftUp;

	tableString_ptr->tableString[tableString_ptr->width * (tableString_ptr->height - 1)] = table_ptr->borders_ptr->angleRightUp;

	size_t pos = tableString_ptr->width - 2 + tableString_ptr->width;

	for (size_t y = 0; y < table_ptr->cellsHeight - 1; y++)
	{
		for (size_t p = 0; p < cellTextSizesWithIndents[y * table_ptr->cellsWidth].height; p++, pos += tableString_ptr->width)
			tableString_ptr->tableString[pos] = table_ptr->borders_ptr->vLine;

		tableString_ptr->tableString[pos] = _GetAngle(table_ptr->borders_ptr, _RLEFT(table_ptr->cells + (table_ptr->cellsWidth - 1 + (y + 1) * table_ptr->cellsWidth)), 1, 0, 0);
		pos += tableString_ptr->width;
	}

	for (size_t p = 0; p < cellTextSizesWithIndents[(table_ptr->cellsHeight - 1) * table_ptr->cellsWidth].height; p++, pos += tableString_ptr->width)
		tableString_ptr->tableString[pos] = table_ptr->borders_ptr->vLine;

	pos = tableString_ptr->width * (tableString_ptr->height - 1) + 1;

	for (size_t x = 0; x < table_ptr->cellsWidth - 1; x++)
	{
		for (size_t p = 0; p < cellTextSizesWithIndents[x].width; p++, pos++)
			tableString_ptr->tableString[pos] = table_ptr->borders_ptr->hLine;

		tableString_ptr->tableString[pos] = _GetAngle(table_ptr->borders_ptr, 0, 0, _RUP(table_ptr->cells + (table_ptr->cellsWidth * (table_ptr->cellsHeight - 1) + x + 1)), 1);
		pos++;
	}

	for (size_t p = 0; p < cellTextSizesWithIndents[table_ptr->cellsWidth - 1].width; p++, pos++)
		tableString_ptr->tableString[pos] = table_ptr->borders_ptr->hLine;
}

//Расставляет символы перехода на новую строку и конец строки
static void _FillHelpSymbols(struct TableString* tableString_ptr)
{
	for (size_t y = tableString_ptr->width - 1; y < tableString_ptr->width * tableString_ptr->height - 1; y += tableString_ptr->width)
		tableString_ptr->tableString[y] = '\n';

	tableString_ptr->tableString[tableString_ptr->width * tableString_ptr->height - 1] = 0;
}

//Заполнение ячеек
static void _FillTable_FirstStr(struct Table* table_ptr, struct TableString* tableString_ptr, struct Size* textWithIndentsSizes, size_t* pos)
{
	_FillCell(tableString_ptr, table_ptr->cells, table_ptr->borders_ptr, textWithIndentsSizes, (*pos), _GetAngle(table_ptr->borders_ptr, 1, _RRIGHT(table_ptr->cells), 1, _RDOWN(table_ptr->cells)));
	(*pos) += textWithIndentsSizes[0].width + 1;

	if (table_ptr->cellsWidth > 1)
	{
		for (size_t x = 1; x < table_ptr->cellsWidth; x++)
		{
			_FillCell(tableString_ptr, table_ptr->cells + x, table_ptr->borders_ptr, textWithIndentsSizes + x, (*pos), _GetAngle(table_ptr->borders_ptr, _RLEFT(table_ptr->cells + (x - 1)), _RRIGHT(table_ptr->cells + x), 1, _RDOWN(table_ptr->cells + x)));
			(*pos) += textWithIndentsSizes[x].width + 1;
		}
	}

	(*pos) = tableString_ptr->width * (textWithIndentsSizes[0].height + 1);
}
static void _FillTable_OtherStrs(struct Table* table_ptr, struct TableString* tableString_ptr, struct Size* textWithIndentsSizes, size_t pos)
{
	for (size_t y = 1; y < table_ptr->cellsHeight; y++)
	{
		size_t cellPos = table_ptr->cellsWidth * y;
		size_t fpos = pos;

		_FillCell(tableString_ptr, table_ptr->cells + cellPos, table_ptr->borders_ptr, textWithIndentsSizes + cellPos, pos, _GetAngle(table_ptr->borders_ptr, 1, _RRIGHT(table_ptr->cells + cellPos), _RUP(table_ptr->cells + (cellPos - table_ptr->cellsWidth)), _RDOWN(table_ptr->cells + cellPos)));
		pos += textWithIndentsSizes[cellPos].width + 1;

		if (table_ptr->cellsWidth > 1)
		{
			for (size_t x = 1; x < table_ptr->cellsWidth; x++)
			{
				_FillCell(tableString_ptr, table_ptr->cells + (cellPos + x), table_ptr->borders_ptr, textWithIndentsSizes + (cellPos + x), pos, _GetAngle(table_ptr->borders_ptr, _RLEFT(table_ptr->cells + (cellPos + x - 1)), _RRIGHT(table_ptr->cells + (cellPos + x)), _RUP(table_ptr->cells + (cellPos + x - table_ptr->cellsWidth)), _RDOWN(table_ptr->cells + (cellPos + x))));
				pos += textWithIndentsSizes[cellPos + x].width + 1;
			}
		}

		pos = fpos + tableString_ptr->width * (textWithIndentsSizes[cellPos].height + 1);
	}
}

static void _FillTable(struct Table* table_ptr, struct TableString* tableString_ptr, struct Size* textWithIndentsSizes)
{
	size_t pos = 0;

	_FillTable_FirstStr(table_ptr, tableString_ptr, textWithIndentsSizes, &pos);
	_FillTable_OtherStrs(table_ptr, tableString_ptr, textWithIndentsSizes, pos);

	_FillRightDownBorder(table_ptr, tableString_ptr, textWithIndentsSizes);

	_FillHelpSymbols(tableString_ptr);
}


static struct _MultiCell
{
	char* text;

	struct Cell* head_ptr;
	size_t headPosX, headPosY;

	size_t indentLeft, indentUp, indentRight, indentDown;
};


static void _ClearMultiCellIndents(size_t cellsWidth, struct _MultiCell* multiCell_ptr)
{
	for (size_t x = 0; x < multiCell_ptr->head_ptr->size.width; x++)
	{
		for (size_t y = 0; y < multiCell_ptr->head_ptr->size.height; y++)
		{
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentLeft = 0;
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentRight = 0;
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentUp = 0;
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentDown = 0;
		}
	}
}

static void _SetMultiCellIndentsAndBorders_FirstStr(struct _MultiCell* multiCell_ptr, size_t indentLeft, size_t indentRight, size_t indentUp)
{
	multiCell_ptr->head_ptr[0].bordersMode = NoRemoveBorders;
	multiCell_ptr->head_ptr[0].style.indentLeft = indentLeft;
	multiCell_ptr->head_ptr[0].style.indentUp = indentUp;

	for (size_t x = 1; x < multiCell_ptr->head_ptr->size.width - 1; x++)
	{
		multiCell_ptr->head_ptr[x].bordersMode = RemoveLeftBorder;
		multiCell_ptr->head_ptr[x].style.indentUp = indentUp;
	}

	if (multiCell_ptr->head_ptr->size.width <= 1) return;

	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1].bordersMode = RemoveLeftBorder;
	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1].style.indentUp = indentUp;
	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1].style.indentRight = indentRight;
}
static void _SetMultiCellIndentsAndBorders_CenterStrs(size_t cellsWidth, struct _MultiCell* multiCell_ptr, size_t indentLeft, size_t indentRight)
{
	for (size_t y = 1; y < multiCell_ptr->head_ptr->size.height - 1; y++)
	{
		multiCell_ptr->head_ptr[y * cellsWidth].bordersMode = RemoveUpBorder;
		multiCell_ptr->head_ptr[y * cellsWidth].style.indentLeft = indentLeft;

		for (size_t x = 1; x < multiCell_ptr->head_ptr->size.width - 1; x++)
			multiCell_ptr->head_ptr[x + y * cellsWidth].bordersMode = RemoveLeftUpBorder;

		if (multiCell_ptr->head_ptr->size.width <= 1) continue;

		multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + y * cellsWidth].bordersMode = RemoveLeftUpBorder;
		multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + y * cellsWidth].style.indentRight = indentRight;
	}

}
static void _SetMultiCellIndentsAndBorders_LastStr(size_t cellsWidth, size_t cellsHeight, struct _MultiCell* multiCell_ptr, size_t indentLeft, size_t indentRight, size_t indentDown)
{
	if (multiCell_ptr->head_ptr->size.height <= 1) return;

	multiCell_ptr->head_ptr[(multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].bordersMode = RemoveUpBorder;
	multiCell_ptr->head_ptr[(multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentLeft = indentLeft;
	multiCell_ptr->head_ptr[(multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentDown = indentDown;

	for (size_t x = 1; x < multiCell_ptr->head_ptr->size.width - 1; x++)
	{
		multiCell_ptr->head_ptr[x + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].bordersMode = RemoveLeftUpBorder;
		multiCell_ptr->head_ptr[x + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentDown = indentDown;
	}

	if (multiCell_ptr->head_ptr->size.width <= 1) return;

	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].bordersMode = RemoveLeftUpBorder;
	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentRight = indentRight;
	multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentDown = indentDown;
}

static void _SetMultiCellIndentsAndBorders(struct Table* table_ptr, struct _MultiCell* multiCell_ptr)
{
	size_t indentLeft = multiCell_ptr->head_ptr->style.indentLeft;
	size_t indentRight = multiCell_ptr->head_ptr->style.indentRight;
	size_t indentUp = multiCell_ptr->head_ptr->style.indentUp;
	size_t indentDown = multiCell_ptr->head_ptr->style.indentDown;

	_ClearMultiCellIndents(table_ptr->cellsWidth, multiCell_ptr);

	_SetMultiCellIndentsAndBorders_FirstStr(multiCell_ptr, indentLeft, indentRight, indentUp);
	_SetMultiCellIndentsAndBorders_CenterStrs(table_ptr->cellsWidth, multiCell_ptr, indentLeft, indentRight);
	_SetMultiCellIndentsAndBorders_LastStr(table_ptr->cellsWidth, table_ptr->cellsHeight, multiCell_ptr, indentLeft, indentRight, indentDown);
}


static void _CreateMultiCell(struct Table* table_ptr, struct Cell* head_ptr, size_t headPosX, size_t headPosY, struct _MultiCell* multiCell_ptr)
{
	multiCell_ptr->text = head_ptr->text;

	head_ptr->text = "";

	multiCell_ptr->head_ptr = head_ptr;

	multiCell_ptr->headPosX = headPosX;
	multiCell_ptr->headPosY = headPosY;

	multiCell_ptr->indentLeft = multiCell_ptr->head_ptr->style.indentLeft;
	multiCell_ptr->indentUp = multiCell_ptr->head_ptr->style.indentUp;
	multiCell_ptr->indentRight = multiCell_ptr->head_ptr->style.indentRight;
	multiCell_ptr->indentDown = multiCell_ptr->head_ptr->style.indentDown;

	_SetMultiCellIndentsAndBorders(table_ptr, multiCell_ptr);
}

#define _IS_MULTICELL(CELL) ((CELL).size.width > 1 || (CELL).size.height > 1)

static size_t _MultiCellsCount(struct Table* table_ptr)
{
	size_t count = 0;

	for (size_t x = 0; x < table_ptr->cellsLength; x++)
		if (_IS_MULTICELL(table_ptr->cells[x])) count++;

	return count;
}

static struct _MultiCell* _GetMultiCells(struct Table* table_ptr, size_t* out_count)
{
	(*out_count) = _MultiCellsCount(table_ptr);

	struct _MultiCell* multiCells = (struct _MultiCell*)malloc(sizeof(struct _MultiCell) * (*out_count));

	size_t index = 0;

	for (size_t x = 0; x < table_ptr->cellsWidth; x++)
	{
		for (size_t y = 0; y < table_ptr->cellsHeight; y++)
		{
			if (_IS_MULTICELL(table_ptr->cells[x + y * table_ptr->cellsWidth]))
			{
				_CreateMultiCell(table_ptr, table_ptr->cells + (x + y * table_ptr->cellsWidth), x, y, multiCells + index);
				index++;
			}
		}
	}

	return multiCells;
}


static void _DistributeSize(size_t cellsWidth, struct _MultiCell* multiCell_ptr, struct Size* textSize_ptr)
{
	size_t cwidth = (*textSize_ptr).width / multiCell_ptr->head_ptr->size.width;
	size_t cheight = ((*textSize_ptr).height - 1) / multiCell_ptr->head_ptr->size.height;

	size_t cwidthRemainder = (*textSize_ptr).width % multiCell_ptr->head_ptr->size.width;
	size_t cheightRemainder = ((*textSize_ptr).height - 1) % multiCell_ptr->head_ptr->size.height;

	for (size_t x = 0; x < multiCell_ptr->head_ptr->size.width; x++)
	{
		for (size_t y = 0; y < multiCell_ptr->head_ptr->size.height; y++)
		{
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentRight += cwidth;
			multiCell_ptr->head_ptr[x + y * cellsWidth].style.indentDown += cheight;
		}
	}

	for (size_t x = 0; x < multiCell_ptr->head_ptr->size.width; x++)
		multiCell_ptr->head_ptr[x + (multiCell_ptr->head_ptr->size.height - 1) * cellsWidth].style.indentDown += cheightRemainder;

	for (size_t y = 0; y < multiCell_ptr->head_ptr->size.height; y++)
		multiCell_ptr->head_ptr[multiCell_ptr->head_ptr->size.width - 1 + y * cellsWidth].style.indentRight += cwidthRemainder;
}

static void _DistributeSizes(struct Table* table_ptr, struct _MultiCell* multiCells, struct Size* multiCellTextWithIndentSizes, size_t multiCellsCount)
{
	for (size_t x = 0; x < multiCellsCount; x++)
		_DistributeSize(table_ptr->cellsWidth, multiCells + x, multiCellTextWithIndentSizes + x);
}


static void _SetMultiCellTextWithIndentsSize(struct _MultiCell* multiCell_ptr, struct Size* size_ptr)
{
	(*size_ptr) = strSize(multiCell_ptr->text);

	(*size_ptr).width += multiCell_ptr->indentLeft + multiCell_ptr->indentRight;
	(*size_ptr).height += multiCell_ptr->indentUp + multiCell_ptr->indentDown;
}

static struct Size* _GetMultiCellTextWithIndentSizes(struct _MultiCell* multiCells, size_t multiCellsCount)
{
	struct Size* sizes = (struct Size*)malloc(multiCellsCount * sizeof(struct Size));

	for (size_t x = 0; x < multiCellsCount; x++)
		_SetMultiCellTextWithIndentsSize(multiCells + x, sizes + x);

	return sizes;
}


static size_t _GetMultiCellPosInTableString(struct Table* table_ptr, struct TableString* tableString_ptr, struct _MultiCell* multiCell_ptr, struct Size* cellTextSizesWithIndents)
{
	size_t posX = multiCell_ptr->headPosX + 1;
	size_t posY = multiCell_ptr->headPosY + 1;

	for (size_t x = 0; x < multiCell_ptr->headPosX; x++)
		posX += cellTextSizesWithIndents[x].width;

	for (size_t y = 0; y < multiCell_ptr->headPosY; y++)
		posY += cellTextSizesWithIndents[y * table_ptr->cellsWidth].height;

	posX += multiCell_ptr->indentLeft;
	posY += multiCell_ptr->indentUp;

	return posX + posY * tableString_ptr->width;
}

static size_t _MultiCellWidth(struct _MultiCell* multiCell_ptr, struct Size* textWithIndentSizes)
{
	size_t width = multiCell_ptr->head_ptr->size.width - 1;

	for (size_t x = 0; x < multiCell_ptr->head_ptr->size.width; x++)
		width += textWithIndentSizes[multiCell_ptr->headPosX + x].width;

	return width;
}

static size_t _MultiCellHeight(struct _MultiCell* multiCell_ptr, struct Size* textWithIndentSizes, size_t cellsWidth)
{
	size_t height = multiCell_ptr->head_ptr->size.height - 1;

	for (size_t y = 0; y < multiCell_ptr->head_ptr->size.height; y++)
		height += textWithIndentSizes[multiCell_ptr->headPosX + (multiCell_ptr->headPosY + y) * cellsWidth].height;

	return height;
}

static void _FillMultiCells(struct Table* table_ptr, struct TableString* tablestring_ptr, struct Size* textWithIndentSizes, struct _MultiCell* multiCells, struct Size* multiCellTextWithIndentSizes, size_t multiCellsCount)
{
	for (size_t x = 0; x < multiCellsCount; x++)
		_FillCellText(tablestring_ptr, multiCells[x].text, _GetMultiCellPosInTableString(table_ptr, tablestring_ptr, multiCells + x, textWithIndentSizes), multiCells[x].head_ptr->style.cellTextAlignment, _MultiCellWidth(multiCells + x, textWithIndentSizes), _MultiCellHeight(multiCells + x, textWithIndentSizes, table_ptr->cellsWidth));
}


struct TableString TableToString(struct Table* table_ptr)
{
	size_t multiCellsCount;

	struct _MultiCell* multiCells = _GetMultiCells(table_ptr, &multiCellsCount);

	struct Size* multiCellTextWithIndentSizes = _GetMultiCellTextWithIndentSizes(multiCells, multiCellsCount);

	_DistributeSizes(table_ptr, multiCells, multiCellTextWithIndentSizes, multiCellsCount);

	struct Size* textWithIndentsSizes = _BorderlessCellSizes(table_ptr);

	_AlignCells(table_ptr, textWithIndentsSizes);

	struct TableString tableString;

	_AllocTableString(&tableString, table_ptr, textWithIndentsSizes);

	_FillTable(table_ptr, &tableString, textWithIndentsSizes);

	_FillMultiCells(table_ptr, &tableString, textWithIndentsSizes, multiCells, multiCellTextWithIndentSizes, multiCellsCount);

	free(textWithIndentsSizes);
	free(multiCellTextWithIndentSizes);
	free(multiCells);

	return tableString;
}

void SetCell(struct Table* table_ptr, size_t x, size_t y, struct Cell cell)
{
	table_ptr->cells[x + y * table_ptr->cellsWidth] = cell;

	struct Cell emptyCell = EMPTY_CELL;
	
	for (size_t w = 1; w < cell.size.width; w++)
		table_ptr->cells[(x + w) + y * table_ptr->cellsWidth] = emptyCell;

	for (size_t h = 1; h < cell.size.height; h++)
		for (size_t w = 0; w < cell.size.width; w++)
			table_ptr->cells[(x + w) + (y + h) * table_ptr->cellsWidth] = emptyCell;
}
