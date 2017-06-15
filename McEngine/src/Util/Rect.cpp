//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		rect
//
// $NoKeywords: $rect
//===============================================================================//

#include "Rect.h"

McRect::McRect(float x, float y, float width, float height, bool isCentered)
{
	set(x, y, width, height, isCentered);
}

void McRect::set(float x, float y, float width, float height, bool isCentered)
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

McRect &McRect::operator = (const McRect &rect)
{
	mMinX = rect.mMinX;
	mMaxX = rect.mMaxX;
	mMinY = rect.mMinY;
	mMaxY = rect.mMaxY;

	return *this;
}

bool McRect::contains(const Vector2 &point)
{
	return point.x >= mMinX && point.x <= mMaxX && point.y >= mMinY && point.y <= mMaxY;
}

McRect McRect::intersect(const McRect &rect)
{
	McRect intersection;

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

McRect McRect::Union(const McRect &rect)
{
	McRect Union;

	Union.mMinX = std::min(mMinX, rect.mMinX);
	Union.mMinY = std::min(mMinY, rect.mMinY);
	Union.mMaxX = std::max(mMaxX, rect.mMaxX);
	Union.mMaxY = std::max(mMaxY, rect.mMaxY);

	return Union;
}

bool McRect::intersects(const McRect &rect)
{
	const float minx = std::max(mMinX, rect.mMinX);
	const float miny = std::max(mMinY, rect.mMinY);
	const float maxx = std::min(mMaxX, rect.mMaxX);
	const float maxy = std::min(mMaxY, rect.mMaxY);

	return minx < maxx && (miny < maxy);
}

