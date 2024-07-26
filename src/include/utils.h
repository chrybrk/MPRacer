#ifndef __UTILS_H__
#define __UTILS_H__

#include "raylib.h"

typedef struct {
	float x, y, w, h;
	int fontSize;
	char *buffer;
	int maxChar;
	int index;
} InputBox;

InputBox InitInputBox(int x, int y, int w, int h, int fontSize, int maxChar);
void DrawInputBox(InputBox *ib, bool line, Color bg, Color fg);

bool CheckTouchCollisionRect(Rectangle _rect);
bool CheckMouseCollisionRect(Rectangle _rect);
bool CheckMouseAndTouchCollisionRect(Rectangle _rect);
bool CheckMouseCollideAndClicked(Rectangle _rect, int key);
bool CheckMouseOrTouchClicked(Rectangle _rect, int key);

void ShowKeyboard();
void HideKeyboard();
void DrawKeyboard(Vector2 startPos);
bool IsKeyboardActive();

#endif
