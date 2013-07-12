#include <blt/os.h>
#include <blt/syscall.h>
#include <blt/qsem.h>
#include <win/Window.h>
#include <win/Canvas.h>
#include <win/Event.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int kCellWidth = 10;

class DrawingArea : public Canvas {
public:
	DrawingArea();
	virtual void EventReceived(const Event*);
private:
	int fLastX;
	int fLastY;
	bool fButtonDown;
};

class PaletteArea : public Canvas {
public:
	PaletteArea(DrawingArea *draw);
	virtual void EventReceived(const Event*);
	virtual void Repaint(long left, long top, long right, long bottom);
private:
	DrawingArea *fDrawingArea;
	int fCurrentColor;
};

DrawingArea::DrawingArea()
	:	fButtonDown(false)
{
}

void DrawingArea::EventReceived(const Event *event)
{
	switch (event->what) {
	case EVT_MOUSE_DOWN:
		LockMouseFocus();
		fButtonDown = true;
		fLastX = event->x;
		fLastY = event->y;
		break;
		
	case EVT_MOUSE_UP:
		fButtonDown = false;
		break;
		
	case EVT_MOUSE_MOVED:
		if (fButtonDown) {
			printf("DrawLine(%d, %d, %d, %d)\n", fLastX, fLastY, event->x, event->y);
			DrawLine(fLastX, fLastY, event->x, event->y);
			fLastX = event->x;
			fLastY = event->y;
			GetWindow()->Flush();
		}		

		break;
	
	default:
		Canvas::EventReceived(event);
	
	}
}

PaletteArea::PaletteArea(DrawingArea *draw)
	:	fDrawingArea(draw)
{
}

void PaletteArea::EventReceived(const Event *event)
{
	switch (event->what) {
	case EVT_MOUSE_DOWN:
		printf("PaletteArea: EventReceived\n");
		fDrawingArea->SetPenColor(event->x / kCellWidth * 2);
		Invalidate(fCurrentColor * kCellWidth, 0, fCurrentColor * kCellWidth + kCellWidth, 10);
		fCurrentColor = event->x / kCellWidth;
		Invalidate(fCurrentColor * kCellWidth, 0, fCurrentColor * kCellWidth + kCellWidth, 10);
		break;
	
	default:
		Canvas::EventReceived(event);
	}
}

void PaletteArea::Repaint(long left, long top, long right, long bottom)
{
	int x = (left / kCellWidth) * kCellWidth;
	while (x < right) {
		SetPenColor(x / kCellWidth * 2);
		FillRect(x, top, x + kCellWidth, bottom);
		if (x / kCellWidth == fCurrentColor) {
			SetPenColor(0);
			DrawLine(x, top, x + kCellWidth, top);
			DrawLine(x, bottom, x + kCellWidth, bottom);
			DrawLine(x, top, x, bottom);
			DrawLine(x + kCellWidth, top, x + kCellWidth, bottom);
		}
		
		x += kCellWidth;
	}
}

int main()
{
	Window *win = new Window(50, 30, 270, 180);
	win->Lock();
	win->Show();
	Canvas *border = new Canvas;
	win->AddChild(border, 0, 0, 250, 150);
	border->SetBackgroundColor(10);
	DrawingArea *draw = new DrawingArea;
	PaletteArea *palette = new PaletteArea(draw);
	border->AddChild(draw, 2, 2, 218, 139);
	border->AddChild(palette, 2, 140, 218, 148);
	draw->SetBackgroundColor(250);
	win->Unlock();
	return 0;
}
