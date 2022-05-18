//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		asteroid and ship explosion particle effect container
//
// $NoKeywords: $
//====================================================================================//

#ifndef ASTEROIDSEXPLOSION_H
#define ASTEROIDSEXPLOSION_H

#include "cbase.h"

class Image;

class AsteroidsExplosion
{
public:
	AsteroidsExplosion(float x, float y, float radius);

	void draw(Graphics *g);
	void update(float dt);

	bool isOver() const;

private:
	struct PARTICLE
	{
		Vector2 position;
		Vector2 velocity;
	};

private:
	Image *m_image;

	float m_fStartTime;

	std::vector<PARTICLE> m_particles;
};

#endif
