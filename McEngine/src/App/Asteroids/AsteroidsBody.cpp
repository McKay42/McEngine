//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		collidable "Body", more or less the base for all interactable objects
//
// $NoKeywords: $
//====================================================================================//

#include "AsteroidsBody.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "ConVar.h"

#include "AsteroidsUtil.h"

ConVar asteroids_big_asteroid_speed("asteroids_big_asteroid_speed", 40.0f, FCVAR_NONE);
ConVar asteroids_medium_asteroid_speed("asteroids_medium_asteroid_speed", 100.0f, FCVAR_NONE);
ConVar asteroids_small_asteroid_speed("asteroids_small_asteroid_speed", 150.0f, FCVAR_NONE);
ConVar asteroids_asteroid_rotation_speed("asteroids_asteroid_rotation_speed", 50.0f, FCVAR_NONE);
ConVar asteroids_bullet_duration("asteroids_bullet_duration", 3.0f, FCVAR_NONE);

AsteroidsBody::AsteroidsBody(TYPE type, float positionX, float positionY, float velocityX, float velocityY)
{
	m_type = type;
	m_vPosition = Vector2(positionX, positionY);
	m_vVelocity = Vector2(velocityX, velocityY);

	m_fRotationVelocity = 0.0f;
	m_fRotation = 0.0f;
	m_fLastDeathTime = 0.0f;

	m_bShouldDraw = true;

	const int screenWidth = engine->getScreenWidth();
	const int screenHeight = engine->getScreenHeight();

	switch (m_type)
	{
	case TYPE::BIG_ASTEROID:
		{
			m_image = engine->getResourceManager()->getImage(UString::format("IMAGE_ASTEROID_BIG_%d", AsteroidsUtil::randi(1, 3)));
			m_fHitboxRadius = std::max(m_image->getWidth(), m_image->getHeight()) / 2;

			// spread randomly around the center of the screen, while not getting too close to it
			do
			{
				m_vPosition.x = (float)AsteroidsUtil::randi(0, screenWidth);
				m_vPosition.y = (float)AsteroidsUtil::randi(0, screenHeight);
			}
			while (m_vPosition.distance(Vector2(screenWidth / 2.0f, screenHeight / 2.0f)) < 2*m_fHitboxRadius);

			m_vVelocity = AsteroidsUtil::rotate(Vector2(asteroids_big_asteroid_speed.getFloat(), 0.0f), AsteroidsUtil::randf(0.0f, 6.28f));
			m_fRotationVelocity = AsteroidsUtil::randf(-asteroids_asteroid_rotation_speed.getFloat(), asteroids_asteroid_rotation_speed.getFloat());
			m_fRotation = AsteroidsUtil::randf(0.0f, 6.28f);
		}
		break;

	case TYPE::MEDIUM_ASTEROID:
		{
			m_image = engine->getResourceManager()->getImage(UString::format("IMAGE_ASTEROID_MEDIUM_%d", AsteroidsUtil::randi(1, 3)).toUtf8());
			m_fHitboxRadius = std::max(m_image->getWidth(), m_image->getHeight()) / 2;

			m_vVelocity = AsteroidsUtil::rotate(Vector2(asteroids_medium_asteroid_speed.getFloat(), 0.0f), AsteroidsUtil::randf(0.0f, 6.28f));
			m_fRotationVelocity = AsteroidsUtil::randf(-asteroids_asteroid_rotation_speed.getFloat(), asteroids_asteroid_rotation_speed.getFloat());
			m_fRotation = AsteroidsUtil::randf(0.0f, 6.28f);
		}
		break;

	case TYPE::SMALL_ASTEROID:
		{
			m_image = engine->getResourceManager()->getImage(UString::format("IMAGE_ASTEROID_SMALL_%d", AsteroidsUtil::randi(1, 3)));
			m_fHitboxRadius = std::max(m_image->getWidth(), m_image->getHeight()) / 2;

			m_vVelocity = AsteroidsUtil::rotate(Vector2(asteroids_small_asteroid_speed.getFloat(), 0.0f), AsteroidsUtil::randf(0.0f, 6.28f));
			m_fRotationVelocity = AsteroidsUtil::randf(-asteroids_asteroid_rotation_speed.getFloat(), asteroids_asteroid_rotation_speed.getFloat());
			m_fRotation = AsteroidsUtil::randf(0.0f, 6.28f);
		}
		break;

	case TYPE::SHIP:
		{
			m_image = engine->getResourceManager()->getImage("IMAGE_SHIP");
			m_fHitboxRadius = std::max(1, (std::max(m_image->getWidth(), m_image->getHeight()) / 2) - 5); // slightly reduce actual size to match visuals and not feel unfair

			m_vPosition.x = screenWidth / 2.0f;
			m_vPosition.y = screenHeight / 2.0f;
		}
		break;

	case TYPE::BULLET:
		{
			m_image = engine->getResourceManager()->getImage("IMAGE_BULLET");
			m_fHitboxRadius = 1;

			m_fLastDeathTime = engine->getTime() + asteroids_bullet_duration.getFloat();
		}
		break;
	}
}

void AsteroidsBody::draw(Graphics *g)
{
	if (!m_bShouldDraw) return;

	const int screenWidth = engine->getScreenWidth();
	const int screenHeight = engine->getScreenHeight();

	Vector2 position = m_vPosition;

	g->setColor(0xffffffff);
	drawInt(g, position); // center

	// screen wrapping effect
	if (m_type != TYPE::BULLET) // bullets don't wrap around
	{
		position.x += screenWidth;
		drawInt(g, position); // right
		position.y += screenHeight;
		drawInt(g, position); // bottom right
		position.x -= screenWidth;
		drawInt(g, position); // bottom
		position.x -= screenWidth;
		drawInt(g, position); // bottom left
		position.y -= screenHeight;
		drawInt(g, position); // left
		position.y -= screenHeight;
		drawInt(g, position); // top left
		position.x += screenWidth;
		drawInt(g, position); // top
		position.x += screenWidth;
		drawInt(g, position); // top right
	}
}

void AsteroidsBody::drawInt(Graphics *g, Vector2 &pos)
{
	if (m_image == NULL) return; // sanity

	// offscreen culling
	{
		const float minX = pos.x + m_image->getWidth();
		const float minY = pos.y + m_image->getHeight();
		const float maxX = pos.x - m_image->getWidth();
		const float maxY = pos.y - m_image->getHeight();

		if (minX < 0 || maxX > engine->getScreenWidth() || minY < 0 || maxY > engine->getScreenHeight())
			return;
	}

	// draw
	g->pushTransform();
	{
		g->rotate(m_fRotation);
		g->translate(pos.x, pos.y);
		g->drawImage(m_image);
	}
	g->popTransform();
}

void AsteroidsBody::update(float dt)
{
	m_fRotation += m_fRotationVelocity * dt;
	m_vPosition += m_vVelocity * dt;

	// make sure we stay "inbounds" (i.e. within one of the four parallel universes touching the screen borders, and not any further out ones)
	const int screenWidth = engine->getScreenWidth();
	const int screenHeight = engine->getScreenHeight();
	bool killBullet = false;
	{
		if (m_vPosition.x > screenWidth)
		{
			killBullet = true;
			m_vPosition.x -= screenWidth;
		}
		else if (m_vPosition.x < 0)
		{
			killBullet = true;
			m_vPosition.x += screenWidth;
		}

		if (m_vPosition.y > screenHeight)
		{
			killBullet = true;
			m_vPosition.y -= screenHeight;
		}
		else if (m_vPosition.y < 0)
		{
			killBullet = true;
			m_vPosition.y += screenHeight;
		}
	}

	if (m_type == TYPE::BULLET && killBullet)
	{
		m_fLastDeathTime = engine->getTime() + 0.2f; // let it live a little bit longer, just in case it hits an asteroid halfway outside the screen
		m_bShouldDraw = false; // but don't draw it anymore
	}
}
