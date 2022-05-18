//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		collidable "Body", more or less the base for all interactable objects
//
// $NoKeywords: $
//====================================================================================//

#ifndef ASTEROIDSBODY_H
#define ASTEROIDSBODY_H

#include "cbase.h"

class AsteroidsBody
{
public:
	enum class TYPE
	{
		SHIP,
		BULLET,
		SMALL_ASTEROID,
		MEDIUM_ASTEROID,
		BIG_ASTEROID
	};

public:
	AsteroidsBody(TYPE type, float positionX = 0.0f, float positionY = 0.0f, float velocityX = 0.0f, float velocityY = 0.0f);

	void draw(Graphics *g);
	void update(float dt);

	void reset();

	// very simple/fast circle-circle intersection test
	inline bool collidesWith(const AsteroidsBody &body) const {return ((m_vPosition.x - body.m_vPosition.x)*(m_vPosition.x - body.m_vPosition.x) + (m_vPosition.y - body.m_vPosition.y)*(m_vPosition.y - body.m_vPosition.y) < (m_fHitboxRadius + body.m_fHitboxRadius)*(m_fHitboxRadius + body.m_fHitboxRadius));}

	inline TYPE getType() const {return m_type;}
	inline bool shouldDraw() const {return m_bShouldDraw;}

	Image *m_image;

	Vector2 m_vPosition;
	Vector2 m_vVelocity;

	float m_fRotation;
	float m_fRotationVelocity;
	float m_fHitboxRadius;
	float m_fLastDeathTime;

private:
	void drawInt(Graphics *g, Vector2 &pos);

	TYPE m_type;

	bool m_bShouldDraw;
};

#endif
