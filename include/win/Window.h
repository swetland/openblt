#ifndef _WINDOW_H
#define _WINDOW_H

#include <blt/qsem.h>

class Canvas;
class Connection;
struct Event;

class Window {
public:

	Window(long, long, long, long);
	void Quit();
	void AddChild(Canvas *child, long left, long top, long right, long bottom);
	void RemoveChild(Canvas *child);
	void Flush();
	void MakeFocus();
	void Lock();
	void Unlock();
	void Show();
	void Hide();

private:

	~Window();
	static int EventLoop(void*);
	Canvas* FindChild(int id);
	void DispatchEvent(Event*);
	void WaitEvent(Event*);

	Connection *fConnection;
	int fID;
	int fShowLevel;
	int fEventPort;
	qsem_t *fLock;

	Canvas *fCanvasList;
	
	friend class Canvas;
};


#endif