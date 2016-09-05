//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		buggy shitty tetris test
//
// $NoKeywords: $tetris
//===============================================================================//

#ifndef TETRIS_H
#define TETRIS_H

#include "App.h"

class TetrisEngine;

// https://en.wikipedia.org/wiki/Tetris
enum TETRIMINO
{
	EMPTY, I, J, L, O, S, T, Z,
	I_GHOST, J_GHOST, L_GHOST, O_GHOST, S_GHOST, T_GHOST, Z_GHOST
};


class Tetris : public App
{
public:
	Tetris();
	virtual ~Tetris();

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onKeyDown(KeyboardEvent &key);
	virtual void onKeyUp(KeyboardEvent &key);

	static Color getTetriminoColor(TETRIMINO t);

private:
	TetrisEngine *m_tetris;
	Vector2 m_vPos;
	Vector2 m_vSize;

	bool m_bLeft,m_bRight,m_bDown;
	float m_fLeftDelay,m_fRightDelay,m_fDownDelay;

	Vector2 m_vRotation;
	Vector2 m_vMouseBackup;
	float m_fZoom;
};



struct TVEC
{
	TETRIMINO t;
	int x,y;
};

class TetrisEngine
{
public:
	TetrisEngine(unsigned int gridWidth, unsigned int gridHeight);
	~TetrisEngine();

	void update();

	void rotateClockwise();
	bool moveLeft();
	bool moveRight();
	void moveDown();
	void slamDown();

	inline int getWidth() const {return m_iWidth;}
	inline int getHeight() const {return m_iHeight;}

	inline TETRIMINO getTetrimino(int x, int y) {return m_playfield[x][y];}

	void spawnTetrimino();
	void clear();

private:
	bool collisionDetection(std::vector<TVEC> &tetrimino, int x, int y, bool placeOnCollision = false);
	bool isSelf(std::vector<TVEC> & tetrimino, int x, int y);
	void moveActiveTetrimino(int x, int y);
	void updateGhost();

	bool isGhost(TETRIMINO t);
	bool isValidActiveTetrimino();
	bool fullLineCheck();
	void gameOverCheck();

	int m_iWidth,m_iHeight;
	TETRIMINO **m_playfield;
	std::vector<TVEC> m_activeTetrimino;
	std::vector<TVEC> m_ghost;

	float m_fLastUpdate;
};

#endif
