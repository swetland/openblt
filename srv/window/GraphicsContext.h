#ifndef _GRAPHICS_CONTEXT_H
#define _GRAPHICS_CONTEXT_H

#include "Renderer.h"
#include "Region.h"

class Window;

class GraphicsContext {
public:
	
	GraphicsContext();
	Rect Bounds();

	inline const Region& ClipRegion();
	inline void SetClipRegion(const Region&);
	inline void SetRenderer(Renderer*);
	inline Renderer* GetRenderer();
	inline void SetOrigin(int x, int y);
	
	void DrawLine(int, int, int, int);
	void FillRect(int, int, int, int);
	void Blit(int x, int y, char image[], int image_width,
		int image_height, int img_bytes_per_row);    
	void StretchBlit(Rect imageRect, Rect screenRect, char image[],
		int img_bytes_per_row);
	void CopyRect(Rect src, Rect dest, const Region &inCleanRegion,
		Region &outNotCopied);	

	void SetColor(char color);
	void DrawString(int x, int y, const char *string);

private:

	Region fClipRegion;
	Renderer *fRenderer;
	char fCurrentColor;
	int fXOrigin;
	int fYOrigin;
};

	
inline const Region& GraphicsContext::ClipRegion() 
{
	return fClipRegion;
}

inline void GraphicsContext::SetClipRegion(const Region &region)
{
	fClipRegion = region;
	fClipRegion.ConstrainTo(fRenderer->Bounds());
}

inline void GraphicsContext::SetRenderer(Renderer *renderer)
{
	fRenderer = renderer;
}

inline Renderer* GraphicsContext::GetRenderer()
{
	return fRenderer;
}

inline void GraphicsContext::SetOrigin(int x, int y)
{
	fXOrigin = x;
	fYOrigin = y;
}


#endif
