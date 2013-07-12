#include <win/Event.h>
#include <win/Button.h>
#include <stdlib.h>
#include <string.h>

Button::Button(const char *text)
	:	fMouseDown(false),
		fOverButton(false)
{
	fText = (char*) malloc(strlen(text) + 1);	
	memcpy(fText, text, strlen(text) + 1);
}

Button::~Button()
{
	free(fText);
}

void Button::Invoke()
{
}

void Button::Repaint(long, long, long, long)
{
	long left, top, right, bottom;
	GetBounds(&left, &top, &right, &bottom);

	if (fMouseDown && fOverButton) {
		left++;
		top++;
	} else {
		right--;
		bottom--;
	}

	SetPenColor(5);
	DrawLine(left, top, right, top);
	DrawLine(left, bottom, right, bottom);
	DrawLine(left, top, left, bottom);
	DrawLine(right, top, right, bottom);
	SetPenColor(10);
	FillRect(left + 1, top + 1, right - 1, bottom - 1);
	DrawString(left + 1, top + 1, fText);
}

void Button::EventReceived(const Event *event)
{
	switch (event->what) {
		case EVT_MOUSE_DOWN:
			LockMouseFocus();
			fMouseDown = true;
			fOverButton = true;
			Invalidate();
			break;
			
		case EVT_MOUSE_UP:
			if (fOverButton)
				Invoke();
				
			fMouseDown = false;
			fOverButton = false;
			Invalidate();
			break;
			
		case EVT_MOUSE_ENTER:
			if (fMouseDown) {
				fOverButton = true; 
				Invalidate();
			}
			
			break;
			
		case EVT_MOUSE_LEAVE:
			if (fMouseDown) {
				fOverButton = false;
				Invalidate();
			}
			
			break;

		default:
			Canvas::EventReceived(event);
	}
}

