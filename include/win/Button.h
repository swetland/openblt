#ifndef _BUTTON_H
#define _BUTTON_H

#include "Canvas.h"

class Button : public Canvas {
public:
	Button(const char *text);
	virtual ~Button();
	virtual void Invoke();

private:
	virtual void Repaint(long, long, long, long);
	virtual void EventReceived(const Event*);

	char *fText;
	bool fMouseDown;
	bool fOverButton;
};

#endif
