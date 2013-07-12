#ifndef _WINDOW_H
#define _WINDOW_H

#include "Region.h"
#include "GraphicsContext.h"

class Event;

class Window {
public:

	Window(int id, int eventPort, Renderer *renderer = 0);
	~Window();
	void AddChild(const Rect& frame, Window *window);
	void RemoveChild(Window *window);
	void MoveToFront();
	inline int ID() const;

	Window *ChildAtPoint(int x, int y);

	Rect LocalToScreen(const Rect&) const;
	Rect ScreenToLocal(const Rect&) const;

	void SetVisibleRegion(const Region&);
	inline const Rect& Frame() const;
	inline Rect Bounds() const;

	const Region& InvalidRegion() const;
	const Region& ClipRegion() const;
	void Invalidate(const Region&);
	void Invalidate(const Rect&);
	void BeginPaint(Rect &out_invalidRect);
	void EndPaint();
	GraphicsContext& GC();
	void ResetGC();
	bool IsVisible() const;
	char Color() const;
	void SetColor(char);
	void Show();
	void Hide();
	void PostEvent(Event*);

	void DumpChildList(int level = 0);

private:
	void UpdateClipRegion();
	void MoveTo(long, long);
	void ResizeTo(long, long);

	int fID;
	Window *fNextSibling;
	Window **fPreviousSibling;
	Window *fChildList;
	Window *fParent;

	Region fInvalidRegion;
	Region fCurrentRedrawRegion;

	// The visible region represents what part of this window is not
	// obscured by siblings of my parent.  I maintain this
	// when recomputing clipping for my children.
	Region fVisibleRegion;

	// The clip region is the visible region, minus parts of my window
	// that are obscured by my children.
	Region fClipRegion;

	Rect fFrame;
	GraphicsContext fGC;
	bool fIsVisible;
	bool fInRedraw;
	int fEventPort;
	bool fPaintMsgSent;
	char fColor;
};

inline int Window::ID() const
{
	return fID;
}

inline const Rect& Window::Frame() const
{
	return fFrame;
}

inline Rect Window::Bounds() const
{
	Rect rect(fFrame);
	rect.OffsetTo(0, 0);
	return rect;
}

inline const Region& Window::InvalidRegion() const
{
	return fInvalidRegion;
}




#endif

