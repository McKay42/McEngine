//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		rect
//
// $NoKeywords: $rect
//===============================================================================//

#ifndef RECT_H
#define RECT_H

#include "Vectors.h"

class McRect
{
public:
	McRect(float x = 0, float y = 0, float width = 0, float height = 0, bool isCentered = false);
	McRect(Vector2 pos, Vector2 size, bool isCentered = false);

	void set(float x, float y, float width, float height, bool isCentered = false);

	inline bool contains(const Vector2 &point) const {return (point.x >= m_fMinX && point.x <= m_fMaxX && point.y >= m_fMinY && point.y <= m_fMaxY);}
	McRect intersect(const McRect &rect) const;
	bool intersects(const McRect &rect) const;
	McRect Union(const McRect &rect) const;

	inline Vector2 getPos() const {return Vector2(m_fMinX, m_fMinY);}
	inline Vector2 getSize() const {return Vector2(m_fMaxX - m_fMinX, m_fMaxY - m_fMinY);}

	inline float getX() const {return m_fMinX;}
	inline float getY() const {return m_fMinY;}
	inline float getWidth() const {return (m_fMaxX - m_fMinX);}
	inline float getHeight() const {return (m_fMaxY - m_fMinY);}

	inline float getMinX() const {return m_fMinX;}
	inline float getMinY() const {return m_fMinY;}
	inline float getMaxX() const {return m_fMaxX;}
	inline float getMaxY() const {return m_fMaxY;}

	inline void setMaxX(float maxx) {m_fMaxX = maxx;}
	inline void setMaxY(float maxy) {m_fMaxY = maxy;}
	inline void setMinX(float minx) {m_fMinX = minx;}
	inline void setMinY(float miny) {m_fMinY = miny;}

	// operators
	McRect &operator = (const McRect &rect);

private:
	float m_fMinX;
	float m_fMinY;
	float m_fMaxX;
	float m_fMaxY;
};

#endif

