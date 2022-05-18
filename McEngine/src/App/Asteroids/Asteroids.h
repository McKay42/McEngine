//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		port of 2DBoy's "Asteroids" example from the WoG prototyping framework
//
// $NoKeywords: $
//====================================================================================//

// NOTE: the original source code has been heavily cleaned up and refactored and fixed, you can find a copy of the original here: https://github.com/MlgmXyysd/2D-BOY-s-Boy-Framework
// NOTE: this app including all of its resources (Asteroids code, sounds, images) are GPLv3 licensed because of their origin

#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "App.h"

#include "AsteroidsBody.h"
#include "AsteroidsExplosion.h"

class Image;
class Sound;
class McFont;

class Asteroids : public App
{
public:
	Asteroids();
	virtual ~Asteroids() {;}

	virtual void update();
	virtual void draw(Graphics *g);

	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onKeyDown(KeyboardEvent &e);

private:
	void death();
	void gameOver();
	void newGame();
	void nextLevel();

	// resources
	Image *m_shipImage;
	Image *m_thrustImage;
	Sound *m_boomSound;
	Sound *m_fireSound;
	Sound *m_thrustSound;
	McFont *m_font;

	// entities
	AsteroidsBody *m_ship;
	std::vector<AsteroidsBody> m_asteroids;
	std::vector<AsteroidsBody> m_bullets;
	std::vector<AsteroidsExplosion> m_explosions;

	// input
	bool m_bThrust;
	bool m_bLeft;
	bool m_bRight;
	bool m_bGunArmed;
	bool m_bBombArmed;

	// game state
	int m_iLevel;
	int m_iNumLives;
	int m_iScore;
	bool m_bGameOver;
	float m_fLastRespawnTime;
	float m_fLastDeathTime;
	float m_fLastBombTime;
};

#endif
