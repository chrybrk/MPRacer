#include <stdlib.h>
#include <string.h>
#include "include/global.h"
#include "include/utils.h"

bool IsCustomKeyboardActive = false;
char CustomLastKeyCharInput = '\0';
int CustomLastKeyInput = 0;
bool hasClickedOnKey = false;

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

void DrawInputBox(InputBox *ib, bool alreadyFocus, bool line, Color bg, Color fg)
{
	if (alreadyFocus || CheckCollisionPointRec(GetMousePosition(), (Rectangle){ ib->x - 10, ib->y - 10, ib->w, ib->h }))
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

void ShowKeyboard()
{
	IsCustomKeyboardActive = true;
}

void HideKeyboard()
{
	IsCustomKeyboardActive = false;
}

void DrawKeyboard(Vector2 startPos)
{
	CustomLastKeyInput = 0;
	CustomLastKeyCharInput = '\0';

	if (IsCustomKeyboardActive)
	{
		char keys[4][10] = {
			{ '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
			{ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' },
			{ 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';' },
			{ 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/' }
		};

		int raylib_key[4][10] = {
			{ KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, KEY_ZERO },
			{ KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P },
			{ KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON },
			{ KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH }
		};

		Vector2 buttonSize = { 50, 50 };

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				Rectangle button = { 
					startPos.x + ((buttonSize.x + 10) * j), 
					startPos.y + ((buttonSize.y + 10) * i), 
					buttonSize.x, 
					buttonSize.y
				};

				DrawRectangleRounded(button, .3f, 10, RED);
				DrawText(TextFormat("%c", keys[i][j]), button.x + 10, button.y + 10, 35, RAYWHITE);

				if (CheckMouseOrTouchClicked(button, MOUSE_BUTTON_LEFT) && !hasClickedOnKey)
					CustomLastKeyInput = raylib_key[i][j], CustomLastKeyCharInput = keys[i][j], hasClickedOnKey = true;
			}
		}

		char *special_keys[] = { "Enter", "Backpace" };
		int raylib_special_keys[] = { KEY_ENTER, KEY_BACKSPACE };

		Vector2 startPosSK = { startPos.x, startPos.y+ (60 * 4) };
		Vector2 buttonSizeSK = { 200, 50 };

		for (int i = 0; i < sizeof(special_keys) / 8; i++)
		{
				Rectangle Sbutton = {
					startPosSK.x + ((buttonSizeSK.x + 10) * i), 
					startPosSK.y, 
					buttonSizeSK.x, 
					buttonSizeSK.y
				};

				DrawRectangleRounded(Sbutton, .3f, 10, RED);
				DrawText(special_keys[i], Sbutton.x + 10, Sbutton.y + 10, 35, RAYWHITE);

				if (CheckMouseOrTouchClicked(Sbutton, MOUSE_BUTTON_LEFT) && !hasClickedOnKey)
					CustomLastKeyInput = raylib_special_keys[i], hasClickedOnKey = true;
		}
	}

	if (hasClickedOnKey)
	{
		if (IsMouseButtonUp(MOUSE_LEFT_BUTTON) && GetTouchPointCount() == 0)
			hasClickedOnKey = false;
	}
}

bool IsKeyboardActive()
{
	return IsCustomKeyboardActive;
}
