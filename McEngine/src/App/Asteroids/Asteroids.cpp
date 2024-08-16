//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		port of 2DBoy's "Asteroids" example from the WoG prototyping framework
//
// $NoKeywords: $
//====================================================================================//

// NOTE: the original source code has been heavily cleaned up and refactored and fixed, you can find a copy of the original here: https://github.com/MlgmXyysd/2D-BOY-s-Boy-Framework
// NOTE: this app including all of its resources (Asteroids code, sounds, images) are GPLv3 licensed because of their origin

#include "Asteroids.h"

#include "Engine.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "SoundEngine.h"
#include "Keyboard.h"
#include "ConVar.h"

#include "AsteroidsUtil.h"

ConVar asteroids_ship_rotation_speed("asteroids_ship_rotation_speed", 200.0f, FCVAR_NONE);
ConVar asteroids_ship_acceleration("asteroids_ship_acceleration", 30.0f, FCVAR_NONE);
ConVar asteroids_ship_max_speed("asteroids_ship_max_speed", 250.0f, FCVAR_NONE);
ConVar asteroids_ship_dampening_factor("asteroids_ship_dampening_factor", 1.0f, FCVAR_NONE);

ConVar asteroids_bullet_speed("asteroids_bullet_speed", 400.0f, FCVAR_NONE);
ConVar asteroids_min_num_asteroids("asteroids_min_num_asteroids", 4, FCVAR_NONE);
ConVar asteroids_invulnerability_period("asteroids_invulnerability_period", 2.5f, FCVAR_NONE);
ConVar asteroids_respawn_delay("asteroids_respawn_delay", 1.0f, FCVAR_NONE);
ConVar asteroids_bomb_delay("asteroids_bomb_delay", 5.0f, FCVAR_NONE);

Asteroids::Asteroids()
{
	// seed rng
	srand(time(NULL));

	// vars
	m_ship = NULL;
	m_bThrust = false;
	m_bLeft = false;
	m_bRight = false;
	m_bGunArmed = true;
	m_bBombArmed = true;
	m_iLevel = 0;
	m_iNumLives = 3;
	m_iScore = 0;
	m_bGameOver = false;
	m_fLastRespawnTime = 0.0f;
	m_fLastDeathTime = 0.0f;
	m_fLastBombTime = 0.0f;

	// load resources
	m_shipImage = engine->getResourceManager()->loadImage("ship.png", "IMAGE_SHIP");
	m_thrustImage = engine->getResourceManager()->loadImage("thruster.png", "IMAGE_THRUST");
	engine->getResourceManager()->loadImage("bullet.png", "IMAGE_BULLET");
	engine->getResourceManager()->loadImage("asteroidb1.png", "IMAGE_ASTEROID_BIG_1");
	engine->getResourceManager()->loadImage("asteroidb2.png", "IMAGE_ASTEROID_BIG_2");
	engine->getResourceManager()->loadImage("asteroidb3.png", "IMAGE_ASTEROID_BIG_3");
	engine->getResourceManager()->loadImage("asteroidm1.png", "IMAGE_ASTEROID_MEDIUM_1");
	engine->getResourceManager()->loadImage("asteroidm2.png", "IMAGE_ASTEROID_MEDIUM_2");
	engine->getResourceManager()->loadImage("asteroidm3.png", "IMAGE_ASTEROID_MEDIUM_3");
	engine->getResourceManager()->loadImage("asteroids1.png", "IMAGE_ASTEROID_SMALL_1");
	engine->getResourceManager()->loadImage("asteroids2.png", "IMAGE_ASTEROID_SMALL_2");
	engine->getResourceManager()->loadImage("asteroids3.png", "IMAGE_ASTEROID_SMALL_3");
	m_fireSound = engine->getResourceManager()->loadSound("fire.ogg", "SOUND_FIRE");
	m_fireSound->setOverlayable(true);
	m_boomSound = engine->getResourceManager()->loadSound("boom.ogg", "SOUND_BOOM");
	m_boomSound->setOverlayable(true);
	m_thrustSound = engine->getResourceManager()->loadSound("thrust.ogg", "SOUND_THRUST", false, false, true); // looped
	m_font = engine->getResourceManager()->loadFont("ubuntu.ttf", "FONT_ASTEROIDS", 32.0f, true, 96 * env->getDPIScale());
}

void Asteroids::draw(Graphics *g)
{
	g->setBlendMode(Graphics::BLEND_MODE::BLEND_MODE_ADDITIVE);
	{
		// draw the ship
		if (m_ship != NULL)
		{
			const bool invulnerable = (engine->getTime() - m_fLastRespawnTime < asteroids_invulnerability_period.getFloat());
			const bool blinking = ((int)(engine->getTime()*1000)) % 100 < 50;

			if (!invulnerable || blinking)
			{
				m_ship->draw(g);

				// if the thrusters are on
				if (m_bThrust)
				{
					// draw the thruster image
					g->setColor(0xffffffff);
					g->pushTransform();
					{
						g->rotate(m_ship->m_fRotation);
						g->translate(m_ship->m_vPosition);
						g->drawImage(m_thrustImage);
					}
					g->popTransform();
				}
			}
		}

		// draw all asteroids, bullets and explosions, in that order
		{
			for (size_t i=0; i<m_asteroids.size(); i++)
			{
				m_asteroids[i].draw(g);
			}

			// bullets
			{
				// NOTE: this is how you would draw every bullet one by one, which is very slow
				/*
				for (size_t i=0; i<m_bullets.size(); i++)
				{
					m_bullets[i].draw(g);
				}
				*/

				// NOTE: so instead, we batch all bullets into one single draw call
				if (m_bullets.size() > 0 && m_bullets[0].m_image != NULL)
				{
					Image *bulletImage = m_bullets[0].m_image;

					static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
					vao.empty();
					g->setColor(0xffffffff);
					bulletImage->bind();
					{
						for (size_t i=0; i<m_bullets.size(); i++)
						{
							if (!m_bullets[i].shouldDraw()) continue;

							const Vector2 topLeft = Vector2(m_bullets[i].m_vPosition.x - bulletImage->getWidth()/2, m_bullets[i].m_vPosition.y - bulletImage->getHeight()/2);
							const Vector2 topRight = topLeft + Vector2(bulletImage->getWidth(), 0);
							const Vector2 bottomLeft = topLeft + Vector2(0, bulletImage->getHeight());
							const Vector2 bottomRight = topRight + Vector2(0, bulletImage->getHeight());

							vao.addVertex(topLeft);
							vao.addTexcoord(0, 0);

							vao.addVertex(topRight);
							vao.addTexcoord(1, 0);

							vao.addVertex(bottomRight);
							vao.addTexcoord(1, 1);

							vao.addVertex(bottomLeft);
							vao.addTexcoord(0, 1);
						}
						g->drawVAO(&vao);
					}
					bulletImage->unbind();
				}
			}

			for (size_t i=0; i<m_explosions.size(); i++)
			{
				m_explosions[i].draw(g);
			}
		}
	}
	g->setBlendMode(Graphics::BLEND_MODE::BLEND_MODE_ALPHA);

	// draw HUD
	{
		const float dpiScale = env->getDPIScale();

		// lives
		g->setColor(0xffffffff);
		g->pushTransform();
		{
			g->translate(50 * dpiScale, 100 * dpiScale);
			for (int i=0; i<m_iNumLives; i++)
			{
				g->drawImage(m_shipImage);
				g->translate(50 * dpiScale, 0);
			}
		}
		g->popTransform();

		// score
		{
			g->pushTransform();
			{
				g->translate(50 * dpiScale, 50 * dpiScale);
				g->setColor(0xffffffff);
				g->drawString(m_font, UString::format("%04d", m_iScore));
			}
			g->popTransform();
		}

		// gameover text
		if (m_bGameOver)
		{
			UString string1("GAME OVER");
			UString string2("press ENTER to restart");

			const float string2scale = 0.5f;
			const float x1 = (engine->getScreenWidth() - m_font->getStringWidth(string1)) / 2.0f;
			const float x2 = (engine->getScreenWidth() - m_font->getStringWidth(string2)*string2scale) / 2.0f;

			g->setColor(0xffffffff);
			g->pushTransform();
			{
				g->translate(x1, engine->getScreenHeight()/2.0f - (50.0f * dpiScale));
				g->drawString(m_font, string1);
			}
			g->popTransform();
			g->pushTransform();
			{
				g->scale(string2scale, string2scale);
				g->translate(x2, engine->getScreenHeight()/2.0f + (50.0f * dpiScale));
				g->drawString(m_font, string2);
			}
			g->popTransform();
		}
	}
}

void Asteroids::update()
{
	const float currentTime = engine->getTime();
	const float dt = engine->getFrameTime();

	// handle ship movement and respawns
	if (m_ship != NULL)
	{
		// tick the ship
		m_ship->update(dt);

		// turn the ship
		if (m_bLeft && !m_bRight)
			m_ship->m_fRotationVelocity = -asteroids_ship_rotation_speed.getFloat();
		else if (m_bRight && !m_bLeft)
			m_ship->m_fRotationVelocity = asteroids_ship_rotation_speed.getFloat();
		else
			m_ship->m_fRotationVelocity = 0;

		// accelerate
		if (m_bThrust)
			m_ship->m_vVelocity += AsteroidsUtil::rotate(Vector2(0, -asteroids_ship_acceleration.getFloat()), deg2rad(m_ship->m_fRotation));

		// cap velocity
		if (m_ship->m_vVelocity.length() > asteroids_ship_max_speed.getFloat())
			m_ship->m_vVelocity = m_ship->m_vVelocity.normalize() * asteroids_ship_max_speed.getFloat();

		// dampen ship velocity
		m_ship->m_vVelocity *= (1.0f - asteroids_ship_dampening_factor.getFloat()*dt);
	}
	else if (!m_bGameOver && currentTime > m_fLastDeathTime + asteroids_respawn_delay.getFloat())
	{
		// respawn
		m_ship = new AsteroidsBody(AsteroidsBody::TYPE::SHIP);
		m_fLastRespawnTime = currentTime;

		// if the thruster is already on, play the sound if necessary
		if (m_bThrust && !m_thrustSound->isPlaying())
			engine->getSound()->play(m_thrustSound);
	}

	// miscellaneous game logic
	if (currentTime > m_fLastBombTime + asteroids_bomb_delay.getFloat())
		m_bBombArmed = true;

	// tick the asteroids
	for (size_t i=0; i<m_asteroids.size(); i++)
	{
		m_asteroids[i].update(dt);

		// see if this asteroid collides with any bullets
		for (size_t j=0; j<m_bullets.size(); j++)
		{
			if (m_bullets[j].m_fLastDeathTime == -1.0f) continue;

			if (m_asteroids[i].m_fLastDeathTime != -1.0f && m_asteroids[i].collidesWith(m_bullets[j]))
			{
				// if this is a medium or large asteroid, add two new asteroids
				switch (m_asteroids[i].getType())
				{
				case AsteroidsBody::TYPE::BIG_ASTEROID:
					m_asteroids.push_back(AsteroidsBody(AsteroidsBody::TYPE::MEDIUM_ASTEROID, m_asteroids[i].m_vPosition.x, m_asteroids[i].m_vPosition.y));
					m_asteroids.push_back(AsteroidsBody(AsteroidsBody::TYPE::MEDIUM_ASTEROID, m_asteroids[i].m_vPosition.x, m_asteroids[i].m_vPosition.y));
					break;
				case AsteroidsBody::TYPE::MEDIUM_ASTEROID:
					m_asteroids.push_back(AsteroidsBody(AsteroidsBody::TYPE::SMALL_ASTEROID, m_asteroids[i].m_vPosition.x, m_asteroids[i].m_vPosition.y));
					m_asteroids.push_back(AsteroidsBody(AsteroidsBody::TYPE::SMALL_ASTEROID, m_asteroids[i].m_vPosition.x, m_asteroids[i].m_vPosition.y));
					break;
				case AsteroidsBody::TYPE::SMALL_ASTEROID:
					m_explosions.push_back(AsteroidsExplosion(m_asteroids[i].m_vPosition.x, m_asteroids[i].m_vPosition.y, m_asteroids[i].m_fHitboxRadius));
					break;
				}

				// mark the hit asteroid for death
				m_asteroids[i].m_fLastDeathTime = -1.0f;

				// mark the bullet for death
				m_bullets[j].m_fLastDeathTime = -1.0f;

				// play a sound
				engine->getSound()->play(m_boomSound);

				// increment score
				m_iScore++;
			}
		}

		// if the asteroid collides with the ship
		if (m_ship != NULL && m_asteroids[i].m_fLastDeathTime != -1.0f && m_asteroids[i].collidesWith(*m_ship))
		{
			// if we are not in the invulnerability period
			if (currentTime - m_fLastRespawnTime > asteroids_invulnerability_period.getFloat())
				death();
		}
	}

	// tick the explosions, remove finished explosions
	for (size_t i=0; i<m_explosions.size(); i++)
	{
		m_explosions[i].update(dt);

		if (m_explosions[i].isOver())
		{
			m_explosions.erase(m_explosions.begin() + i);
			i--;
		}
	}

	// remove dead asteroids
	for (size_t i=0; i<m_asteroids.size(); i++)
	{
		if (m_asteroids[i].m_fLastDeathTime < 0.0f)
		{
			m_asteroids.erase(m_asteroids.begin() + i);
			i--;
		}
	}

	// tick the bullets, remove dead bullets
	for (size_t i=0; i<m_bullets.size(); i++)
	{
		m_bullets[i].update(dt);

		if (currentTime > m_bullets[i].m_fLastDeathTime)
		{
			m_bullets.erase(m_bullets.begin() + i);
			i--;
		}
	}

	// if there are no more asteroids then go to the next level
	if (m_asteroids.size() == 0)
		nextLevel();
}

void Asteroids::onKeyUp(KeyboardEvent &e)
{
	if (e == KEY_LEFT || e == KEY_A)
		m_bLeft = false;
	else if (e == KEY_RIGHT || e == KEY_D)
		m_bRight = false;
	else if (e == KEY_UP || e == KEY_W)
	{
		m_bThrust = false;
		engine->getSound()->stop(m_thrustSound);
	}
	else if (e == KEY_SPACE)
		m_bGunArmed = true;
}

void Asteroids::onKeyDown(KeyboardEvent &e)
{
	// DEBUG:
	/*
	{
		if (e == KEY_F)
			gameOver();

		if (e == KEY_S)
		{
			if (!m_bGameOver && m_ship != NULL)
			{
				const int numBullets = 360;
				const float rot = 360.0f / numBullets;
				for (int i=0; i<numBullets; i++)
				{
					Vector2 vel = AsteroidsUtil::rotate(AsteroidsUtil::rotate(Vector2(0.0f,-asteroids_bullet_speed.getFloat()),deg2rad(abs(i*rot))), -deg2rad(m_ship->m_fRotation));
					m_bullets.push_back(AsteroidsBody(AsteroidsBody::TYPE::BULLET, (engine->getScreenWidth()/2), (engine->getScreenHeight()/2), vel.x, vel.y));
				}
			}
		}
	}
	*/

	if (e == KEY_ESCAPE)
		engine->shutdown();
	else if (e == KEY_LEFT || e == KEY_A)
		m_bLeft = true;
	else if (e == KEY_RIGHT || e == KEY_D)
		m_bRight = true;
	else if (e == KEY_UP || e == KEY_W)
	{
		m_bThrust = true;
		if (!m_bGameOver && m_ship != NULL && !m_thrustSound->isPlaying())
			engine->getSound()->play(m_thrustSound);
	}
	else if (e == KEY_RETURN)
	{
		if (m_bGameOver)
			newGame();
	}
	else if (e == KEY_SPACE)
	{
		if (m_bGunArmed && !m_bGameOver && m_ship != NULL)
		{
			// add a bullet
			const Vector2 velocity = AsteroidsUtil::rotate(Vector2(0.0f, -asteroids_bullet_speed.getFloat()), deg2rad(m_ship->m_fRotation)) + m_ship->m_vVelocity;
			m_bullets.push_back(AsteroidsBody(AsteroidsBody::TYPE::BULLET, m_ship->m_vPosition.x, m_ship->m_vPosition.y, velocity.x, velocity.y));

			// make a sound
			engine->getSound()->play(m_fireSound);

			// gun is no longer armed
			m_bGunArmed = false;
		}
	}
	else if (e == KEY_SHIFT || e == KEY_CONTROL)
	{
		if (m_bBombArmed && !m_bGameOver && m_ship != NULL)
		{
			m_bBombArmed = false;
			m_fLastBombTime = engine->getTime();

			const int numBullets = 60;
			const float rot = 360.0f / numBullets;
			for (int i=0; i<numBullets; i++)
			{
				const Vector2 velocity = AsteroidsUtil::rotate(AsteroidsUtil::rotate(Vector2(0.0f, -asteroids_bullet_speed.getFloat()), deg2rad(abs(i*rot))), -deg2rad(m_ship->m_fRotation)) + m_ship->m_vVelocity;
				m_bullets.push_back(AsteroidsBody(AsteroidsBody::TYPE::BULLET, m_ship->m_vPosition.x, m_ship->m_vPosition.y, velocity.x, velocity.y));
			}

			engine->getSound()->play(m_fireSound);
		}
	}
}

void Asteroids::death()
{
	if (m_ship == NULL) return; // sanity

	// create an explosion
	m_explosions.push_back(AsteroidsExplosion(m_ship->m_vPosition.x, m_ship->m_vPosition.y, m_ship->m_fHitboxRadius));
	m_explosions.push_back(AsteroidsExplosion(m_ship->m_vPosition.x, m_ship->m_vPosition.y, m_ship->m_fHitboxRadius));

	// destroy ship
	SAFE_DELETE(m_ship);

	// make sound
	engine->getSound()->play(m_boomSound);

	// stop thruster sound if it's playing
	engine->getSound()->stop(m_thrustSound);

	// decrement lives and see if game is over
	m_iNumLives--;
	if (m_iNumLives == 0)
		gameOver();

	// mark time of death for respawn later
	m_fLastDeathTime = engine->getTime();
}

void Asteroids::gameOver()
{
	m_bGameOver = true;
}

void Asteroids::newGame()
{
	// reset
	m_iScore = 0;
	m_iNumLives = 3;
	m_iLevel = 0;
	m_bGameOver = false;

	// cleanup
	m_asteroids.clear();

	// and spawn
	nextLevel();
}

void Asteroids::nextLevel()
{
	// increment level
	m_iLevel++;
	debugLog("level = %i\n", m_iLevel);

	// destroy ship if necessary
	SAFE_DELETE(m_ship);

	// create new ship
	m_ship = new AsteroidsBody(AsteroidsBody::TYPE::SHIP);
	m_fLastRespawnTime = engine->getTime();

	// create some asteroids, more the higher the level
	for (int i=0; i<(asteroids_min_num_asteroids.getInt() + m_iLevel - 1); i++)
	{
		m_asteroids.push_back(AsteroidsBody(AsteroidsBody::TYPE::BIG_ASTEROID));
	}

	// cleanup
	m_bullets.clear();
	m_explosions.clear();
}
