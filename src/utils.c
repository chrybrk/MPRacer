#include <stdlib.h>
#include <string.h>
#include "include/utils.h"

InputBox InitInputBox(int x, int y, int w, int h, int fontSize, int maxChar)
{
	InputBox ib = {
		x, y, w, h, fontSize,
		malloc(maxChar * sizeof(char)),
		maxChar,
		0
	};

	memset(ib.buffer, 0, maxChar);

	return ib;
}

void DrawInputBox(InputBox *ib, bool line, Color bg, Color fg)
{
	if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ ib->x - 10, ib->y - 10, ib->w, ib->h }))
	{
		SetMouseCursor(MOUSE_CURSOR_IBEAM);
		int key = GetCharPressed();

		while (key > 0)
		{
			if ((key >= 32) && (key <= 125) && (ib->index < ib->maxChar))
			{
				ib->buffer[ib->index] = (char)key;
				ib->buffer[ib->index+1] = '\0';
				ib->index++;
			}

			key = GetCharPressed();
		}

		if (IsKeyPressed(KEY_BACKSPACE))
		{
			ib->index--;
			if (ib->index < 0) ib->index = 0;
			ib->buffer[ib->index] = '\0';
		}
	}
	else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

	if (line)
		DrawRectangleRoundedLines((Rectangle) { ib->x - 10, ib->y - 10, ib->w, ib->h }, .3f, 10, .8f, fg);

	int frame = (int)ib->w % ib->fontSize;
	DrawText(ib->index > frame ? TextSubtext(ib->buffer, ib->index - frame, frame) : ib->buffer, ib->x, ib->y, ib->fontSize, bg);
}

bool CheckTouchCollisionRect(Rectangle _rect)
{
	for (int i = 0; i < GetTouchPointCount(); ++i)
	{
		if (CheckCollisionPointRec(GetTouchPosition(i), _rect))
			return true;
	}

	return false;
}

bool CheckMouseCollisionRect(Rectangle _rect)
{
	return CheckCollisionPointRec(GetMousePosition(), _rect);
}

bool CheckMouseAndTouchCollisionRect(Rectangle _rect)
{
	return CheckMouseCollisionRect(_rect) && CheckTouchCollisionRect(_rect);
}

bool CheckMouseCollideAndClicked(Rectangle _rect, int key)
{
	return CheckMouseCollisionRect(_rect) && IsMouseButtonDown(key);
}

bool CheckMouseOrTouchClicked(Rectangle _rect, int key)
{
	return CheckMouseCollideAndClicked(_rect, key) || CheckTouchCollisionRect(_rect);
}
