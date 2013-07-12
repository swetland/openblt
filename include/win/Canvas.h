#ifndef _CANVAS_H
#define _CANVAS_H

class Window;
class Event;

class Canvas {
public:

	Canvas();
	virtual ~Canvas();
	
	void AddChild(Canvas *child, long left, long top, long right, long bottom);
	void RemoveChild(Canvas *child);
	void DrawLine(long, long, long, long);
	void FillRect(long, long, long, long);
	void SetBackgroundColor(char);
	void SetPenColor(char);
	void DrawString(long, long, const char*);
	void Hide();
	void Show();
	void CopyRect(long left, long top, long right, long bottom, long newLeft,
		long newTop);
	Window* GetWindow() const;
	void GetBounds(long *left, long *top, long *right, long *bottom);
	void Invalidate(long left = 0, long top = 0, long right = 100000, long bottom
		= 100000);
	void LockMouseFocus();
	
	virtual void Repaint(long left, long top, long right, long bottom);
	virtual void EventReceived(const Event*);

	
private:

	void HandleEvent(Event*);
	void BeginPaint(long *out_left, long *out_top, long *out_right, long *out_bottom);
	void EndPaint();

	int fID;
	Window *fWindow;
	bool fInPaint;
	int fShowLevel;
	Canvas *fWinListNext, **fWinListPrev;
	long fLeft, fTop, fRight, fBottom;

	friend class Window;
};


#endif
