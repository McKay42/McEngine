//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		rect
//
// $NoKeywords: $rect
//===============================================================================//

#include "Rect.h"

Rect::Rect(float x, float y, float width, float height, bool isCentered)
{
	set(x, y, width, height, isCentered);
}

void Rect::set(float x, float y, float width, float height, bool isCentered)
{
	if (isCentered)
	{
		mMinX = x - width/2;
		mMaxX = x + width/2;
		mMinY = y - height/2;
		mMaxY = y + height/2;
	}
	else
	{
		mMinX = x;
		mMaxX = x + width;
		mMinY = y;
		mMaxY = y + height;
	}
}

Rect &Rect::operator = (const Rect &rect)
{
	mMinX = rect.mMinX;
	mMaxX = rect.mMaxX;
	mMinY = rect.mMinY;
	mMaxY = rect.mMaxY;

	return *this;
}

bool Rect::contains(const Vector2 &point)
{
	return point.x >= mMinX && point.x <= mMaxX && point.y >= mMinY && point.y <= mMaxY;
}

Rect Rect::intersect(const Rect &rect)
{
	Rect intersection;

	intersection.mMinX = std::max(mMinX, rect.mMinX);
	intersection.mMinY = std::max(mMinY, rect.mMinY);
	intersection.mMaxX = std::min(mMaxX, rect.mMaxX);
	intersection.mMaxY = std::min(mMaxY, rect.mMaxY);

	// if the rects don't intersect:
	if (intersection.mMinX > intersection.mMaxX || intersection.mMinY > intersection.mMaxY)
	{
		// reset the rect to a null rect
		intersection.mMinX = 0.0f;
		intersection.mMaxX = 0.0f;
		intersection.mMinY = 0.0f;
		intersection.mMaxY = 0.0f;
	}

	return intersection;
}

Rect Rect::Union(const Rect &rect)
{
	Rect Union;

	Union.mMinX = std::min(mMinX, rect.mMinX);
	Union.mMinY = std::min(mMinY, rect.mMinY);
	Union.mMaxX = std::max(mMaxX, rect.mMaxX);
	Union.mMaxY = std::max(mMaxY, rect.mMaxY);

	return Union;
}

bool Rect::intersects(const Rect &rect)
{
	const float minx = std::max(mMinX, rect.mMinX);
	const float miny = std::max(mMinY, rect.mMinY);
	const float maxx = std::min(mMaxX, rect.mMaxX);
	const float maxy = std::min(mMaxY, rect.mMaxY);

	return minx < maxx && (miny < maxy);
}

