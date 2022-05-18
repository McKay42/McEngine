//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		helper functions
//
// $NoKeywords: $
//====================================================================================//

#ifndef ASTEROIDSUTIL_H
#define ASTEROIDSUTIL_H

#include "cbase.h"

class AsteroidsUtil
{
public:
	// this function rotates a point p1 around the point p0.
	// based on http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BASICMAT/node4.html
	static Vector2 rotate(const Vector2 &p1, const Vector2 &p0, float angle)
	{
		const Vector2 d = p1 - p0;

		return p0 + Vector2(d.x * std::cos(angle) - d.y * std::sin(angle),
							d.x * std::sin(angle) + d.y * std::cos(angle));
	}

	// this function rotates a point p around the point origin.
	// based on http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BASICMAT/node4.html
	static Vector2 rotate(const Vector2 &p, float angle)
	{
		return Vector2(p.x * std::cos(angle) - p.y * std::sin(angle),
					   p.x * std::sin(angle) + p.y * std::cos(angle));
	}

	static float randf(float min, float max)
	{
		return min + (((float)rand() / RAND_MAX) * (max - min));
	}

	static int randi(int min, int max)
	{
		return min + (rand() % (max - min + 1));
	}
};

#endif
