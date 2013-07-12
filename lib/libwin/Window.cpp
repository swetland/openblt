#include <blt/namer.h>
#include <blt/syscall.h>
#include <win/Window.h>
#include <win/Canvas.h>
#include <win/Event.h>
#include <stdio.h>
#include "../../srv/window/protocol.h"
#include "Connection.h"

Window::Window(long left, long top, long right, long bottom)
	:	fShowLevel(0),
		fCanvasList(0)
{
 	int windowServerPort;
 	windowServerPort = namer_find ("window_server", 0);
 	if (windowServerPort <= 0) {
 		printf("couldn't connect to window server\n");
 		return;
 	}

	int localReplyPort = port_create(0, "client_syncport");
	fEventPort = port_create(0, "client_eventport");

 	fConnection = new Connection(windowServerPort, localReplyPort);
 	fConnection->WriteInt8(OP_CREATE_WINDOW);
 	fConnection->WriteInt32(0);			// Child of root window
 	fConnection->WriteInt32(left);
 	fConnection->WriteInt32(top);
 	fConnection->WriteInt32(right);
 	fConnection->WriteInt32(bottom);
 	fConnection->WriteInt32(fEventPort);
 	fConnection->Flush();
 	fID = fConnection->ReadInt32();

 	fLock = qsem_create(1);

	thr_create((void*) EventLoop, this, "win_thread");
}

Window::~Window()
{
	qsem_acquire(fLock);
	while (fCanvasList) {
		Canvas *child = fCanvasList;
		fCanvasList = fCanvasList->fWinListNext;
		fConnection->WriteInt8(OP_DESTROY_WINDOW);
		fConnection->WriteInt32(child->fID);
		fConnection->Flush();
		delete child;
	}

	fConnection->WriteInt8(OP_DESTROY_WINDOW);
	fConnection->WriteInt32(fID);
	fConnection->Flush();
	delete fConnection;

 	qsem_destroy(fLock);
}


void Window::MakeFocus()
{
	fConnection->WriteInt8(OP_MAKE_FOCUS);
	fConnection->WriteInt32(fID);
	fConnection->Flush();
}

void Window::Flush()
{
	fConnection->Flush();
}

void Window::WaitEvent(Event *event)
{
 	msg_hdr_t header;
 	header.src = 0;
 	header.dst = fEventPort;
 	header.data = event;
 	header.size = sizeof(Event);
 	old_port_recv(&header);
}

void Window::AddChild(Canvas *child, long left, long top, long right, long bottom)
{
	fConnection->WriteInt8(OP_CREATE_WINDOW);
	fConnection->WriteInt32(fID);					// My child
	fConnection->WriteInt32(left);
	fConnection->WriteInt32(top);
	fConnection->WriteInt32(right);
	fConnection->WriteInt32(bottom);
	fConnection->WriteInt32(fEventPort);
	fConnection->Flush();
	child->fID = fConnection->ReadInt32();
	child->fWindow = this;
	child->fLeft = 0;
	child->fTop = 0;
	child->fRight = right - left;
	child->fBottom = bottom - top;	
	child->Show();

	// Stick window in my canvas list
	child->fWinListNext = fCanvasList;
	child->fWinListPrev = &fCanvasList;
	if (fCanvasList)
		fCanvasList->fWinListPrev = &child->fWinListNext;
		
	fCanvasList = child;
}

void Window::RemoveChild(Canvas *child)
{
	Lock();
	fConnection->WriteInt8(OP_DESTROY_WINDOW);
	fConnection->WriteInt32(child->fID);
	fConnection->Flush();
	*child->fWinListPrev = child->fWinListNext;
	if (child->fWinListNext)
		child->fWinListNext->fWinListPrev = child->fWinListPrev;
		
	Unlock();
}

void Window::Lock()
{
	qsem_acquire(fLock);
}

void Window::Unlock()
{
	qsem_release(fLock);
}

void Window::Hide()
{
	if (--fShowLevel == 0) {
		fConnection->WriteInt8(OP_HIDE_WINDOW);
 		fConnection->WriteInt32(fID);
		fConnection->Flush();
	}
}

void Window::Show()
{
	if (++fShowLevel == 1) {
		fConnection->WriteInt8(OP_SHOW_WINDOW);
		fConnection->WriteInt32(fID);
		fConnection->Flush();
	}
}

Canvas* Window::FindChild(int id)
{
	for (Canvas *canvas = fCanvasList; canvas; canvas = canvas->fWinListNext)
		if (canvas->fID == id)
			return canvas;
	
	return 0;
}

void Window::DispatchEvent(Event *event)
{
	Canvas *canvas = FindChild(event->target);
	if (canvas)
		canvas->HandleEvent(event);
}

int Window::EventLoop(void *_window)
{
	Window *window = (Window*) _window;	
	while (true) {
		Event event;
		window->WaitEvent(&event);
		if (event.what == EVT_QUIT) {
			delete window;
			os_terminate(0);
		}

		window->Lock();
		window->DispatchEvent(&event);
		window->Unlock();
	}
}

void Window::Quit()
{
	Event event;
	event.what = EVT_QUIT;
	event.target = fID;

	msg_hdr_t header;
	header.src = fEventPort;	// May break someday
	header.dst = fEventPort;
	header.data = &event;
	header.size = sizeof(Event);
	old_port_send(&header);
}


