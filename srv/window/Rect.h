#ifndef RECT_H
#define RECT_H

#include <stdio.h>
#include "util.h"

class Rect { 
public:
	inline Rect();
	inline Rect(int left, int top, int right, int bottom);
	inline void SetTo(int l, int t, int r, int b);
	inline Rect& InsetBy(int, int);
	inline Rect& OffsetBy(int, int);
	inline Rect& OffsetTo(int, int);
	inline bool Contains(int, int) const;	
 	inline bool Contains(const Rect&) const;
	inline bool Intersects(const Rect&) const;
	inline Rect& Intersect(const Rect&);	
	inline bool Valid() const;
	inline int Width() const;
	inline int Height() const;
	inline void Dump() const;

	int left;
	int top;
	int right;
	int bottom;
};

inline Rect::Rect()
	:	left(0),
		top(0),
		right(0),
		bottom(0)
{
}

inline Rect::Rect(int l, int t, int r, int b)
	:	left(l),
		top(t),
		right(r),
		bottom(b)
{
}

inline void Rect::SetTo(int l, int t, int r, int b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}



inline Rect& Rect::InsetBy(int h, int v)
{
	left += h;
	right -= h;
	top += v;
	bottom -= v;
	return *this;
}

inline Rect& Rect::OffsetBy(int h, int v)
{
	left += h;
	right += h;
	top += v;
	bottom += v;	
	return *this;
}


inline Rect& Rect::OffsetTo(int h, int v)
{
	right += (h - left);
	bottom += (v - top);
	left = h;
	top = v;
	return *this;
}

inline bool Rect::Intersects(const Rect &rect) const
{
	return max(left, rect.left) <= min(right, rect.right)
		&& max(top, rect.top) <= min(bottom, rect.bottom);
}

inline bool Rect::Valid() const
{
	return right >= left && bottom >= top;
}

inline void Rect::Dump() const
{
	printf("Rect (%d, %d, %d, %d)\n", left, top, right, bottom);
}


inline int Rect::Width() const
{
	return right - left;
}

inline int Rect::Height() const
{
	return bottom - top;
}

inline bool Rect::Contains(int x, int y) const
{
	return (x >= left && x <= right && y >= top && y <= bottom);       
}

inline bool Rect::Contains(const Rect &rect) const
{
	return rect.left >= left && rect.right <= right 
		&& rect.top >= top && rect.bottom <= bottom;
}

inline Rect& Rect::Intersect(const Rect &rect) 
{
	left = max(left, rect.left);
	right = min(right, rect.right);
	top = max(top, rect.top);
	bottom = min(bottom, rect.bottom);
	return *this;
}

#endif
