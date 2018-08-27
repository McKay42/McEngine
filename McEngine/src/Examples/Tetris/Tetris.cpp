//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		buggy shitty tetris test
//
// $NoKeywords: $tetris
//===============================================================================//

#include "Tetris.h"

#include "Engine.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "AnimationHandler.h"

#include <time.h>

#define BOARD_SIZE_MULTIPLIER 1

#define HIDDEN_TOP 4 // tetrimino spawn area
#define KEY_REPEAT_SPEED 0.07f // how fast left/right/down movements are

Tetris::Tetris()
{
	srand(time(NULL));

	m_tetris = new TetrisEngine(10*BOARD_SIZE_MULTIPLIER,20*BOARD_SIZE_MULTIPLIER+HIDDEN_TOP);
	m_vPos = Vector2(50,0);
	m_vSize = Vector2(300,700);

	m_bLeft = m_bRight = m_bDown = false;
	m_fLeftDelay = m_fRightDelay = m_fDownDelay = false;

	m_fZoom = 0.0f;
}

Tetris::~Tetris()
{
	SAFE_DELETE(m_tetris);
}

Color Tetris::getTetriminoColor(TETRIMINO t)
{
	if (t == I)
		return 0xff00ffff;
	else if (t == J)
		return 0xff0000ff;
	else if (t == L)
		return COLOR(255, 255, 165, 0);
	else if (t == O)
		return 0xffffff00;
	else if (t == S)
		return COLOR(255, 128, 255, 0);
	else if (t == T)
		return COLOR(255, 128, 0, 128);
	else if (t == Z)
		return COLOR(255, 255, 0, 0);

	return 0x44000000;
}

void Tetris::draw(Graphics *g)
{
	g->setColor(0xff888888);
	g->fillRect(0,0,engine->getScreenWidth(),engine->getScreenHeight());

	// draw grid
	int blockWidth = m_vSize.x / m_tetris->getWidth();
	int blockHeight = m_vSize.y / m_tetris->getHeight();
	m_vPos = Vector2(engine->getScreenWidth()/2 - (m_tetris->getWidth()*blockWidth)/2, engine->getScreenHeight()/2 - ((m_tetris->getHeight()+HIDDEN_TOP)*blockHeight)/2); //HACKHACK:

	g->push3DScene(McRect(0,0,engine->getScreenWidth(),engine->getScreenHeight()));
	g->translate3DScene(0,0,m_fZoom);
	g->rotate3DScene(-m_vRotation.y, m_vRotation.x, 0);

	g->setColor(0xff000000);
	for (int y=HIDDEN_TOP; y<m_tetris->getHeight()+1; y++)
	{
		g->drawLine(m_vPos.x, m_vPos.y + y*blockHeight, m_vPos.x + m_tetris->getWidth()*blockWidth, m_vPos.y + y*blockHeight);
	}
	for (int x=0; x<m_tetris->getWidth()+1; x++)
	{
		g->drawLine(m_vPos.x + x*blockWidth, m_vPos.y + HIDDEN_TOP*blockHeight, m_vPos.x + x*blockWidth, m_vPos.y + m_tetris->getHeight()*blockHeight );
	}

	// draw tetriminos in the most inefficient way possible
	for (int x=0; x<m_tetris->getWidth(); x++)
	{
		for (int y=4; y<m_tetris->getHeight(); y++)
		{
			TETRIMINO t = m_tetris->getTetrimino(x,y);

			if (t == EMPTY)
				continue;

			Color c = getTetriminoColor(t);
			g->setColor(c);

			int insetX = blockWidth/4;
			int insetY = blockHeight/4;

			Color bottom = COLOR(COLOR_GET_Ai(c), clamp<int>(COLOR_GET_Ri(c)-100, 30, 255), clamp<int>(COLOR_GET_Gi(c)-100, 30, 255), clamp<int>(COLOR_GET_Bi(c)-100, 30, 255));

			// left trapez
			Color leftBottom = COLOR(COLOR_GET_Ai(c), clamp<int>(COLOR_GET_Ri(c)+45, 0, 255), clamp<int>(COLOR_GET_Gi(c)+45, 0, 255), clamp<int>(COLOR_GET_Bi(c)+45, 0, 255));
			Color leftTop = COLOR(COLOR_GET_Ai(c), clamp<int>(COLOR_GET_Ri(c)+140, 0, 255), clamp<int>(COLOR_GET_Gi(c)+140, 0, 255), clamp<int>(COLOR_GET_Bi(c)+140, 0, 255));
			g->drawQuad(Vector2(m_vPos.x + x*blockWidth, m_vPos.y + y*blockHeight),
						Vector2(m_vPos.x + x*blockWidth + insetX, m_vPos.y + y*blockHeight + insetY),
						Vector2(m_vPos.x + x*blockWidth + insetX, m_vPos.y + y*blockHeight + blockHeight - insetY),
						Vector2(m_vPos.x + x*blockWidth, m_vPos.y + y*blockHeight + blockHeight),
						leftTop, leftTop, leftBottom, leftBottom);

			// top trapez
			Color topLeft = COLOR(COLOR_GET_Ai(c), clamp<int>(COLOR_GET_Ri(c)+170, 0, 255), clamp<int>(COLOR_GET_Gi(c)+170, 0, 255), clamp<int>(COLOR_GET_Bi(c)+170, 0, 255));
			g->drawQuad(Vector2(m_vPos.x + x*blockWidth, m_vPos.y + y*blockHeight),
						Vector2(m_vPos.x + x*blockWidth + insetX, m_vPos.y + y*blockHeight + insetY),
						Vector2(m_vPos.x + x*blockWidth + blockWidth - insetX, m_vPos.y + y*blockHeight + insetY),
						Vector2(m_vPos.x + x*blockWidth + blockWidth, m_vPos.y + y*blockHeight),
						topLeft, topLeft, leftTop, leftTop);

			// right trapez
			Color rightBottom = COLOR(COLOR_GET_Ai(c), clamp<int>(COLOR_GET_Ri(c)-50, 0, 255), clamp<int>(COLOR_GET_Gi(c)-50, 0, 255), clamp<int>(COLOR_GET_Bi(c)-50, 0, 255));
			g->drawQuad(Vector2(m_vPos.x + x*blockWidth + blockWidth, m_vPos.y + y*blockHeight),
						Vector2(m_vPos.x + x*blockWidth + blockWidth - insetX, m_vPos.y + y*blockHeight + insetY),
						Vector2(m_vPos.x + x*blockWidth + blockWidth - insetX, m_vPos.y + y*blockHeight + blockHeight - insetY),
						Vector2(m_vPos.x + x*blockWidth + blockWidth, m_vPos.y + y*blockHeight + blockHeight),
						bottom, bottom, rightBottom, rightBottom);

			// bottom trapez
			g->drawQuad(Vector2(m_vPos.x + x*blockWidth + blockWidth, m_vPos.y + y*blockHeight + blockHeight),
						Vector2(m_vPos.x + x*blockWidth + blockWidth - insetX, m_vPos.y + y*blockHeight + blockHeight - insetY),
						Vector2(m_vPos.x + x*blockWidth + insetX, m_vPos.y + y*blockHeight + blockHeight - insetY),
						Vector2(m_vPos.x + x*blockWidth, m_vPos.y + y*blockHeight + blockHeight),
						bottom, bottom, bottom, bottom);

			g->setColor(c);
			g->fillRect(m_vPos.x + x*blockWidth + insetX, m_vPos.y + y*blockHeight + insetY, blockWidth - insetX*2, blockHeight - insetY*2);
		}
	}

	g->pop3DScene();
}

void Tetris::update()
{
	// handle movement keys
	if (m_bLeft && engine->getTime() > m_fLeftDelay)
	{
		m_tetris->moveLeft();
		m_fLeftDelay = engine->getTime()+KEY_REPEAT_SPEED;
	}

	if (m_bRight && engine->getTime() > m_fRightDelay)
	{
		m_tetris->moveRight();
		m_fRightDelay = engine->getTime()+KEY_REPEAT_SPEED;
	}

	if (m_bDown && engine->getTime() > m_fDownDelay)
	{
		m_tetris->moveDown();
		m_fDownDelay = engine->getTime()+KEY_REPEAT_SPEED;
	}

	// update tetris engine
	m_tetris->update();

	// handle mouse 3d rotation
	Vector2 delta = engine->getMouse()->getPos() - m_vMouseBackup;
	m_vMouseBackup = engine->getMouse()->getPos();

	if (!anim->isAnimating(&m_vRotation.x) && !anim->isAnimating(&m_vRotation.y) && !anim->isAnimating(&m_fZoom))
	{
		if (engine->getMouse()->isLeftDown())
			m_vRotation += delta*0.5f;
		if (engine->getMouse()->getWheelDeltaVertical() != 0)
			m_fZoom += engine->getMouse()->getWheelDeltaVertical()*0.1f;
	}

	if (engine->getMouse()->isRightDown() && !anim->isAnimating(&m_vRotation.x) && !anim->isAnimating(&m_vRotation.y) && !anim->isAnimating(&m_fZoom) && (m_vRotation.x != 0.0f || m_vRotation.y != 0.0f || m_fZoom != 0.0f))
	{
		anim->moveQuadInOut(&m_vRotation.x, 0.0f, 1.0f, 0.0f, true);
		anim->moveQuadInOut(&m_vRotation.y, 0.0f, 1.0f, 0.0f, true);
		anim->moveQuadInOut(&m_fZoom, 1.0f, 1.0f, 0.0f, true);
	}
}

void Tetris::onKeyDown(KeyboardEvent &key)
{
	if (key == KEY_LEFT)
		m_bLeft = true;
	if (key == KEY_RIGHT)
		m_bRight = true;
	if (key == KEY_DOWN)
		m_bDown = true;

	if (key == KEY_UP)
		m_tetris->rotateClockwise();
	if (key == KEY_SPACE || key == KEY_ENTER)
		m_tetris->slamDown();
}

void Tetris::onKeyUp(KeyboardEvent &key)
{
	if (key == KEY_LEFT)
		m_bLeft = false;
	if (key == KEY_RIGHT)
		m_bRight = false;
	if (key == KEY_DOWN)
		m_bDown = false;
}



TetrisEngine::TetrisEngine(unsigned int gridWidth, unsigned int gridHeight)
{
	m_iWidth = gridWidth;
	m_iHeight = gridHeight;

	m_playfield = new TETRIMINO*[m_iWidth];
	for (int i=0; i<m_iWidth; i++)
	{
		m_playfield[i] = new TETRIMINO[m_iHeight];
	}

	clear();

	spawnTetrimino();

	m_fLastUpdate = engine->getTime()+1.0f;
}

TetrisEngine::~TetrisEngine()
{
	for (int i=0; i<m_iWidth; i++)
	{
		delete m_playfield[i];
	}
	delete m_playfield;
}

void TetrisEngine::update()
{
	if (engine->getTime() < m_fLastUpdate)
		return;

	// spawn tetrimino if necessary
	if (m_activeTetrimino.size() < 1)
		spawnTetrimino();

	// tick tetrimino
	moveDown();

	m_fLastUpdate = engine->getTime()+0.3f;
}

bool TetrisEngine::isSelf(std::vector<TVEC> & tetrimino, int x, int y)
{
	for (int i=0; i<tetrimino.size(); i++)
	{
		if (tetrimino[i].x == x && tetrimino[i].y == y)
			return true;
	}
	return false;
}

bool TetrisEngine::isValidActiveTetrimino()
{
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		TVEC v = m_activeTetrimino[i];
		if (v.x < 0 || v.x > m_iWidth-1 || v.y < 0 || v.y > m_iHeight-1)
			return false;
		else if (m_playfield[v.x][v.y] != EMPTY)
			return false;
	}
	return true;
}

bool TetrisEngine::collisionDetection(std::vector<TVEC> &tetrimino, int x, int y, bool placeOnCollision)
{
	// this is very buggy and unfinished

	// tick tetrimino
	if (tetrimino.size() > 0)
	{
		// collision detection
		for (int i=0; i<tetrimino.size(); i++)
		{
			TVEC v = tetrimino[i];

			// if we hit something at the bottom or hit the end of the playfield, stop

			// down check
			if (x == 0 && !isSelf(tetrimino,v.x,v.y+y) && ((m_playfield[v.x][v.y+y] != EMPTY && !isGhost(m_playfield[v.x][v.y+y])) || v.y+y > m_iHeight-1))
			{
				if (placeOnCollision)
				{
					tetrimino.clear(); // tetrimino has been placed
					while(fullLineCheck()){}; // check for full lines
					gameOverCheck(); // check for gameover
					m_fLastUpdate = 0; // force immediate spawn
				}
				return true;
			}

			// left/right check
			if (x != 0 && !isSelf(tetrimino,v.x+x,v.y+y) && (v.x+x < 0 || v.x+x > m_iWidth-1 || m_playfield[v.x+x][v.y+y] != EMPTY))
				return true;
		}
	}
	return false;
}

void TetrisEngine::moveActiveTetrimino(int x, int y)
{
	// remove tetrimino from playfield
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		TVEC v = m_activeTetrimino[i];
		m_playfield[v.x][v.y] = EMPTY;
	}

	// and put it back again, with the updated position
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		((TVEC*)(&m_activeTetrimino[i]))->x += x;
		((TVEC*)(&m_activeTetrimino[i]))->y += y;

		TVEC v = m_activeTetrimino[i];
		m_playfield[v.x][v.y] = v.t;
	}

	updateGhost();
}

void TetrisEngine::updateGhost()
{
	// calculate ghost

	// remove last ghost from playfield
	for (int i=0; i<m_ghost.size(); i++)
	{
		TVEC v = m_ghost[i];
		if (!isSelf(m_activeTetrimino, v.x, v.y))
			m_playfield[v.x][v.y] = EMPTY;
	}

	// copy current tetrimino to ghost
	m_ghost = std::vector<TVEC>(m_activeTetrimino);

	// calculate new ghost position
	while(!collisionDetection(m_ghost, 0, 1))
	{
		for (int i=0; i<m_ghost.size(); i++)
		{
			((TVEC*)(&m_ghost[i]))->y += 1;
		}
	}

	// remove all invalid blocks which are already occupied by the active tetrimino
	for (int i=0; i<m_ghost.size(); i++)
	{
		TVEC v = m_ghost[i];
		if (isSelf(m_activeTetrimino, v.x, v.y))
		{
			m_ghost.erase(m_ghost.begin()+i);
			i--;
		}
	}

	// and put the remaining ghost blocks back again
	for (int i=0; i<m_ghost.size(); i++)
	{
		TVEC v = m_ghost[i];
		m_playfield[v.x][v.y] = I_GHOST; // HACKHACK: any change to the TETRIMINO enum will fuck this up
	}
}

void TetrisEngine::rotateClockwise()
{
	// this is also very buggy and unfinished

	if (m_activeTetrimino.size() < 1)
		return;
	if (m_activeTetrimino[0].t == O) // O blocks don't get rotation
		return;

	// remove tetrimino from playfield
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		TVEC v = m_activeTetrimino[i];
		m_playfield[v.x][v.y] = EMPTY;
	}

	// center of all tetriminos is always m_activeTetrimino[1], the second block.
	// rotate around center block
	std::vector<TVEC> prediction;
	TVEC center = m_activeTetrimino[1];
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		if (i != 1) // if not the center block
		{
			TVEC t = m_activeTetrimino[i];
			if ((t.x > center.x && t.y <= center.y) || (t.y >= center.y && t.x < center.x)) // right/left quadrant
				prediction.push_back({t.t, center.x-(t.y-center.y), center.y+(t.x-center.x)});
			else if ((t.x >= center.x && t.y > center.y) || (t.x <= center.x && t.y < center.y)) // bottom/top quadrant
				prediction.push_back({t.t, center.x+(center.y-t.y), center.y-(center.x-t.x)});
		}
	}
	prediction.insert(prediction.begin()+1, center);

	// check for possible collisions
	bool rotationPossible = true;
	for (int i=0; i<prediction.size(); i++)
	{
		TVEC t = prediction[i];
		if (t.x > m_iWidth-1 || t.y > m_iHeight-1 || t.x < 0 || t.y < 0 || m_playfield[t.x][t.y] != EMPTY) // if we hit a wall because of our rotation
		{
			// try going left by 1 block
			if (t.x > m_iWidth-1)
			{
				std::vector<TVEC> backup = std::vector<TVEC>(m_activeTetrimino);
				m_activeTetrimino = prediction;

				// move left
				for (int i=0; i<m_activeTetrimino.size(); i++)
				{
					((TVEC*)(&m_activeTetrimino[i]))->x += -1;
					((TVEC*)(&m_activeTetrimino[i]))->y += 0;
				}

				if (isValidActiveTetrimino()) // if there is no more collision, we are happy
				{
					rotationPossible = true;
					prediction = m_activeTetrimino;
					break;
				}
				else
				{
					rotationPossible = false;
					m_activeTetrimino = backup; // if there is a collision, restore our backup
					break;
				}
			}

			// try going right by 1 block
			if (t.x < 0)
			{
				std::vector<TVEC> backup = std::vector<TVEC>(m_activeTetrimino);
				m_activeTetrimino = prediction;

				// move right
				for (int i=0; i<m_activeTetrimino.size(); i++)
				{
					((TVEC*)(&m_activeTetrimino[i]))->x += 1;
					((TVEC*)(&m_activeTetrimino[i]))->y += 0;
				}

				if (isValidActiveTetrimino()) // if there is no more collision, we are happy
				{
					rotationPossible = true;
					prediction = m_activeTetrimino;
					break;
				}
				else
				{
					rotationPossible = false;
					m_activeTetrimino = backup; // if there is a collision, restore our backup
					break;
				}
			}

			rotationPossible = false;
			break;
		}
	}
	if (rotationPossible) // swap to rotated tetrimino
		m_activeTetrimino = prediction;

	// and put it back again
	for (int i=0; i<m_activeTetrimino.size(); i++)
	{
		TVEC v = m_activeTetrimino[i];
		m_playfield[v.x][v.y] = v.t;
	}

	if (rotationPossible)
		updateGhost();
}

bool TetrisEngine::moveLeft()
{
	if (m_activeTetrimino.size() < 1)
		return false;

	if (collisionDetection(m_activeTetrimino, -1, 0))
		return false;

	moveActiveTetrimino(-1,0);

	return true;
}

bool TetrisEngine::moveRight()
{
	if (m_activeTetrimino.size() < 1)
		return false;

	if (collisionDetection(m_activeTetrimino, 1, 0))
		return false;

	moveActiveTetrimino(1,0);

	return true;
}

void TetrisEngine::moveDown()
{
	if (m_activeTetrimino.size() < 1)
		return;

	if (collisionDetection(m_activeTetrimino, 0, 1, true))
		return;

	moveActiveTetrimino(0,1);

	// delay next update
	m_fLastUpdate = engine->getTime()+0.3f;
}

void TetrisEngine::slamDown()
{
	if (m_activeTetrimino.size() < 1)
		return;

	while(m_activeTetrimino.size() > 0)
	{
		moveDown();
	}

	// delay next update
	m_fLastUpdate = engine->getTime()+0.3f;
}

void TetrisEngine::spawnTetrimino()
{
	int r = rand() % 7;

	gameOverCheck();

	if (r == 0)
	{
		// #
		// #
		// #
		// #

		m_playfield[m_iWidth/2][0] = I;
		m_playfield[m_iWidth/2][1] = I;
		m_playfield[m_iWidth/2][2] = I;
		m_playfield[m_iWidth/2][3] = I;

		m_activeTetrimino.push_back({I,m_iWidth/2,0});
		m_activeTetrimino.push_back({I,m_iWidth/2,1});  // center of tetrimino
		m_activeTetrimino.push_back({I,m_iWidth/2,2});
		m_activeTetrimino.push_back({I,m_iWidth/2,3});
	}
	else if (r == 1)
	{
		//
		// # # #
		//     #
		//

		m_playfield[m_iWidth/2 - 1][2] = J;
		m_playfield[m_iWidth/2][2] = J;
		m_playfield[m_iWidth/2 + 1][2] = J;
		m_playfield[m_iWidth/2 + 1][3] = J;

		m_activeTetrimino.push_back({J,m_iWidth/2 - 1,2});
		m_activeTetrimino.push_back({J,m_iWidth/2,2});  // center of tetrimino
		m_activeTetrimino.push_back({J,m_iWidth/2 + 1,2});
		m_activeTetrimino.push_back({J,m_iWidth/2 + 1,3});
	}
	else if (r == 2)
	{
		//
		// # # #
		// #
		//

		m_playfield[m_iWidth/2 - 1][1+2] = L;
		m_playfield[m_iWidth/2 - 1][2] = L;
		m_playfield[m_iWidth/2][2] = L;
		m_playfield[m_iWidth/2 + 1][2] = L;

		m_activeTetrimino.push_back({L,m_iWidth/2 - 1,1+2});
		m_activeTetrimino.push_back({L,m_iWidth/2,2}); // center of tetrimino
		m_activeTetrimino.push_back({L,m_iWidth/2 - 1,2});
		m_activeTetrimino.push_back({L,m_iWidth/2 + 1,2});

	}
	else if (r == 3)
	{
		//
		// # #
		// # #
		//

		m_playfield[m_iWidth/2 - 1][2] = O;
		m_playfield[m_iWidth/2][2] = O;
		m_playfield[m_iWidth/2 - 1][3] = O;
		m_playfield[m_iWidth/2][3] = O;

		m_activeTetrimino.push_back({O,m_iWidth/2 - 1,2});
		m_activeTetrimino.push_back({O,m_iWidth/2,2});
		m_activeTetrimino.push_back({O,m_iWidth/2 - 1,3});
		m_activeTetrimino.push_back({O,m_iWidth/2,3});
	}
	else if (r == 4)
	{
		//
		//   # #
		// # #
		//

		m_playfield[m_iWidth/2 - 1][3] = S;
		m_playfield[m_iWidth/2][3] = S;
		m_playfield[m_iWidth/2][2] = S;
		m_playfield[m_iWidth/2 + 1][2] = S;

		m_activeTetrimino.push_back({S,m_iWidth/2 - 1,3});
		m_activeTetrimino.push_back({S,m_iWidth/2,3});  // center of tetrimino
		m_activeTetrimino.push_back({S,m_iWidth/2,2});
		m_activeTetrimino.push_back({S,m_iWidth/2 + 1,2});
	}
	else if (r == 5)
	{
		//
		//   #
		// # # #
		//

		m_playfield[m_iWidth/2 - 1][3] = T;
		m_playfield[m_iWidth/2][3] = T;
		m_playfield[m_iWidth/2][2] = T;
		m_playfield[m_iWidth/2 + 1][3] = T;

		m_activeTetrimino.push_back({T,m_iWidth/2 - 1,3});
		m_activeTetrimino.push_back({T,m_iWidth/2,3});  // center of tetrimino
		m_activeTetrimino.push_back({T,m_iWidth/2,2});
		m_activeTetrimino.push_back({T,m_iWidth/2 + 1,3});
	}
	else if (r == 6)
	{
		//
		// # #
		//   # #
		//

		m_playfield[m_iWidth/2 - 1][2] = Z;
		m_playfield[m_iWidth/2][3] = Z;
		m_playfield[m_iWidth/2][2] = Z;
		m_playfield[m_iWidth/2 + 1][3] = Z;

		m_activeTetrimino.push_back({Z,m_iWidth/2 - 1,2});
		m_activeTetrimino.push_back({Z,m_iWidth/2,2});  // center of tetrimino
		m_activeTetrimino.push_back({Z,m_iWidth/2,3});
		m_activeTetrimino.push_back({Z,m_iWidth/2 + 1,3});
	}
}

bool TetrisEngine::fullLineCheck()
{
	for (int y=0; y<m_iHeight; y++)
	{
		bool full = true;
		for (int x=0; x<m_iWidth; x++)
		{
			if (m_playfield[x][y] == EMPTY)
			{
				full = false;
				break;
			}
		}

		// if full, empty the full line, then go back up and push all blocks down
		if (full)
		{
			for (int x=0; x<m_iWidth; x++)
			{
				m_playfield[x][y] = EMPTY;
			}

			for (y = y-1; y >= 0; y--)
			{
				for (int x=0; x<m_iWidth; x++)
				{
					m_playfield[x][y+1] = m_playfield[x][y];
				}
			}

			return true;
		}
	}
	return false;
}

void TetrisEngine::gameOverCheck()
{
	for (int i=0; i<m_iWidth; i++)
	{
		if (m_playfield[i][HIDDEN_TOP-1] != EMPTY)
		{
			///engine->showInfo("Tetris", "Game over man, game over!");
			clear();
		}
	}
}

bool TetrisEngine::isGhost(TETRIMINO t)
{
	if (t == I_GHOST || t == J_GHOST || t == L_GHOST || t == O_GHOST || t == S_GHOST || t == T_GHOST || t == Z_GHOST)
		return true;
	else
		return false;
}

void TetrisEngine::clear()
{
	for (int x=0; x<m_iWidth; x++)
	{
		for (int y=0; y<m_iHeight; y++)
		{
			m_playfield[x][y] = EMPTY;
		}
	}
}
