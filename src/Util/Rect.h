//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		rect
//
// $NoKeywords: $rect
//===============================================================================//

#ifndef RECT_H
#define RECT_H

#include "cbase.h"
#include "Vectors.h"

class Rect
{
public:
	Rect(float x = 0, float y = 0, float width = 0, float height = 0, bool isCentered = false);
	virtual ~Rect() {;}

	void set(float x, float y, float width, float height, bool isCentered = false);
	virtual Rect &operator = (const Rect &rect);

	virtual bool contains(const Vector2 &point);
	virtual Rect intersect(const Rect &rect);
	virtual bool intersects(const Rect &rect);
	virtual Rect Union(const Rect &rect);

	inline float getX() const {return mMinX;}
	inline float getY() const {return mMinY;}
	inline float getWidth() const {return mMaxX-mMinX;}
	inline float getHeight() const {return mMaxY-mMinY;}

	inline float getMinX() const {return mMinX;}
	inline float getMinY() const {return mMinY;}
	inline float getMaxX() const {return mMaxX;}
	inline float getMaxY() const {return mMaxY;}

	inline void setMaxX(float maxx) {mMaxX = maxx;}
	inline void setMaxY(float maxy) {mMaxY = maxy;}
	inline void setMinX(float minx) {mMinX = minx;}
	inline void setMinY(float miny) {mMinY = miny;}

private:
	float mMinX;
	float mMinY;
	float mMaxX;
	float mMaxY;
};

#endif

