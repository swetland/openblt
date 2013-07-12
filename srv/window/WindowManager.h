#ifndef _WINDOW_MANAGER_H
#define _WINDOW_MANAGER_H

#include <blt/qsem.h>
#include "Rect.h"

class Renderer;
class Window;

const int kMaxWindows = 255;

class WindowManager {
public:

	WindowManager(Renderer *screenRenderer);
	~WindowManager();
	
private:

	Window* CreateWindow(Window *parent, const Rect &rect, int eventPort);
	void DestroyWindow(Window *window);
	Window* LookupWindow(int id);
	Window* WindowAtPoint(int x, int y);

	int RequestorPort() const;
	static int StartDispatchThread(void *windowManager);
	void DispatchThread();
	void ReadServicePort(void*, int);
	int ReadInt32();
	short ReadInt16();
	char ReadInt8();
	void Respond(void *, int);

	void SetCursorPos(int x, int y);
	static int StartMouseThread(void *_wm);
	void MouseThread();
	void LockCursor();
	void UnlockCursor();
	
	void InvalidateMouseBoundries();

	
	Window *fWindowArray[kMaxWindows];
	int fNextWindowID;

	int fServicePort;
	char *fReceiveBuffer;
	int fReceiveBufferSize;
	int fReceiveBufferPos;
	int fRequestorPort;
	qsem_t *fCursorLock;

	Rect fMouseBoundries;
	Window *fCurrentMouseFocus;
	bool fMouseFocusLocked;
	bool fInFocusLockedWindow;

	Renderer *fScreenRenderer;
};


#endif
