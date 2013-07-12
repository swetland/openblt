#ifndef _RENDERER_H
#define _RENDERER_H

#include "Rect.h"

class Renderer {
public:

	inline Renderer(char *baseAddress, int width, int height, int bytesPerRow);
	virtual void DrawLine(int x1, int y1, int x2, int y2, char color) = 0;
	virtual void FillRect(int x1, int y1, int x2, int y2, char color) = 0;
	virtual void Blit(int x, int y, char image[], int image_width,
		int image_height, int img_bytes_per_row) = 0;
	virtual void StretchBlit(const Rect &imageRect, const Rect &displayRect, char image[],
		int imageBytesPerRow) = 0;
	virtual void CopyRect(const Rect &source, const Rect &dest) = 0;

	inline Rect Bounds() const;

	inline char *BufferBaseAddress() const;
	inline int BufferWidth() const;
	inline int BufferHeight() const;
	inline int BufferBytesPerRow() const;

private:
	char *fBufferBaseAddress;
	int fBufferWidth;
	int fBufferHeight;
	int fBufferBytesPerRow;	
};

inline Renderer::Renderer(char *baseAddress, int width, int height, int bytesPerRow)
	:	fBufferBaseAddress(baseAddress),
		fBufferWidth(width),
		fBufferHeight(height),
		fBufferBytesPerRow(bytesPerRow)
{
}

inline char* Renderer::BufferBaseAddress() const
{
	return fBufferBaseAddress;
}

inline int Renderer::BufferWidth() const
{
	return fBufferWidth;
}

inline int Renderer::BufferHeight() const
{
	return fBufferHeight;
}

inline int Renderer::BufferBytesPerRow() const
{
	return fBufferBytesPerRow;
}

inline Rect Renderer::Bounds() const
{
	return Rect(0, 0, fBufferWidth - 1, fBufferHeight - 1);
}



#endif
