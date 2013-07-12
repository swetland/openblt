#include <win/Canvas.h>
#include <win/Window.h>
#include <win/Event.h>
#include "Connection.h"
#include "../../srv/window/protocol.h"

Canvas::Canvas()
	:	fID(-1),
		fWindow(0),
		fInPaint(false),
		fShowLevel(0)
{
}

Canvas::~Canvas()
{
}

void Canvas::DrawLine(long x1, long y1, long x2, long y2)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_DRAW_LINE);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt32(x1);
	fWindow->fConnection->WriteInt32(y1);
	fWindow->fConnection->WriteInt32(x2);
	fWindow->fConnection->WriteInt32(y2);
	fWindow->fConnection->EndCommand();
}

void Canvas::FillRect(long x1, long y1, long x2, long y2)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_FILL_RECT);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt32(x1);
	fWindow->fConnection->WriteInt32(y1);
	fWindow->fConnection->WriteInt32(x2);
	fWindow->fConnection->WriteInt32(y2);
	fWindow->fConnection->EndCommand();
}

void Canvas::SetPenColor(char c)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_SET_PEN_COLOR);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt8(c);
	fWindow->fConnection->EndCommand();
}

void Canvas::SetBackgroundColor(char c)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_SET_BG_COLOR);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt8(c);
	fWindow->fConnection->EndCommand();
}

void Canvas::DrawString(long x, long y, const char *str)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_DRAW_STRING);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt32(x);
	fWindow->fConnection->WriteInt32(y);
	while (*str)
		fWindow->fConnection->WriteInt8(*str++);
		
	fWindow->fConnection->WriteInt8(0);
	fWindow->fConnection->EndCommand();
}

void Canvas::CopyRect(long left, long top, long right, long bottom,
	long newLeft, long newTop)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_COPY_RECT);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt32(left);
	fWindow->fConnection->WriteInt32(top);
	fWindow->fConnection->WriteInt32(right);
	fWindow->fConnection->WriteInt32(bottom);
	fWindow->fConnection->WriteInt32(newLeft);
	fWindow->fConnection->WriteInt32(newTop);
	fWindow->fConnection->EndCommand();
}


void Canvas::Hide()
{
	if (fWindow == 0)
		return;

	if (--fShowLevel == 0) {
		fWindow->fConnection->WriteInt8(OP_HIDE_WINDOW);
		fWindow->fConnection->WriteInt32(fID);
		fWindow->fConnection->Flush();
	}
}

void Canvas::Show()
{
	if (fWindow == 0)
		return;

	if (++fShowLevel == 1) {
		fWindow->fConnection->WriteInt8(OP_SHOW_WINDOW);
		fWindow->fConnection->WriteInt32(fID);
		fWindow->fConnection->Flush();
	}
}

void Canvas::BeginPaint(long *out_left, long *out_top, long *out_right, long *out_bottom)
{
	if (fInPaint)
		return;
		
	fInPaint = true;
	fWindow->fConnection->WriteInt8(OP_BEGIN_PAINT);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->Flush();
	
	*out_left = fWindow->fConnection->ReadInt32();
	*out_top = fWindow->fConnection->ReadInt32();
	*out_right = fWindow->fConnection->ReadInt32();
	*out_bottom = fWindow->fConnection->ReadInt32();
}

void Canvas::EndPaint()
{
	if (!fInPaint)
		return;
		
	fInPaint = false;
	fWindow->fConnection->WriteInt8(OP_END_PAINT);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->Flush();
}

void Canvas::AddChild(Canvas *child, long left, long top, long right, long bottom)
{
	if (fWindow == 0)
		return;

	fWindow->fConnection->WriteInt8(OP_CREATE_WINDOW);
	fWindow->fConnection->WriteInt32(fID);					// My child
	fWindow->fConnection->WriteInt32(left);
	fWindow->fConnection->WriteInt32(top);
	fWindow->fConnection->WriteInt32(right);
	fWindow->fConnection->WriteInt32(bottom);
	fWindow->fConnection->WriteInt32(fWindow->fEventPort);
	fWindow->fConnection->Flush();
	child->fID = fWindow->fConnection->ReadInt32();
	child->fWindow = fWindow;
	child->fLeft = 0;
	child->fTop = 0;
	child->fRight = right - left;
	child->fBottom = bottom - top;	

	// Stick window in the window's canvas list
	child->fWinListNext = fWindow->fCanvasList;
	child->fWinListPrev = &fWindow->fCanvasList;
	if (fWindow->fCanvasList)
		fWindow->fCanvasList->fWinListPrev = &child->fWinListNext;
		
	fWindow->fCanvasList = child;

	child->Show();
}

void Canvas::RemoveChild(Canvas *child)
{
	fWindow->fConnection->WriteInt8(OP_DESTROY_WINDOW);
	fWindow->fConnection->WriteInt32(child->fID);
	fWindow->fConnection->Flush();
	*child->fWinListPrev = child->fWinListNext;
}

void Canvas::HandleEvent(Event *event)
{
	switch (event->what) {
		case EVT_PAINT: {
			long left, top, right, bottom;
			BeginPaint(&left, &top, &right, &bottom);
			Repaint(left, top, right, bottom);
			EndPaint();
			break;
		}
	
		default:
			EventReceived(event);
	}
}

Window* Canvas::GetWindow() const
{
	return fWindow;
}

void Canvas::GetBounds(long *left, long *top, long *right, long *bottom)
{
	*left = fLeft;
	*top = fTop;
	*right = fRight;
	*bottom = fBottom;
}

void Canvas::Invalidate(long left, long top, long right, long bottom)
{
	fWindow->fConnection->WriteInt8(OP_INVALIDATE);
	fWindow->fConnection->WriteInt32(fID);
	fWindow->fConnection->WriteInt32(left);
	fWindow->fConnection->WriteInt32(top);
	fWindow->fConnection->WriteInt32(right);
	fWindow->fConnection->WriteInt32(bottom);
	fWindow->fConnection->Flush();
}

void Canvas::Repaint(long left, long top, long right, long bottom)
{
}

void Canvas::EventReceived(const Event *)
{
}

void Canvas::LockMouseFocus()
{
	fWindow->fConnection->WriteInt8(OP_LOCK_MOUSE_FOCUS);
	fWindow->fConnection->Flush();
}

