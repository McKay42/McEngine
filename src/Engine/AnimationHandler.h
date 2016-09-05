//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		global fps independent animations
//
// $NoKeywords: $anim
//===============================================================================//

#ifndef ANIMATIONHANDLER_H
#define ANIMATIONHANDLER_H

#include "cbase.h"

class AnimationHandler
{
public:
	AnimationHandler();
	~AnimationHandler();

	void update();

	void moveLinear(float *base, float target, float duration, float delay, bool overrideExisting = false);

	void moveLinear(float *base, float target, float duration, bool overrideExisting = false) {moveLinear(base, target, duration, 0.0f, overrideExisting);}

	void moveQuadIn(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuadOut(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuadInOut(float *base, float target, float duration, float delay, bool overrideExisting = false);

	void moveQuadIn(float *base, float target, float duration, bool overrideExisting = false) {moveQuadIn(base, target, duration, 0.0f, overrideExisting);}
	void moveQuadOut(float *base, float target, float duration, bool overrideExisting = false) {moveQuadOut(base, target, duration, 0.0f, overrideExisting);}
	void moveQuadInOut(float *base, float target, float duration, bool overrideExisting = false) {moveQuadInOut(base, target, duration, 0.0f, overrideExisting);}

	// DEPRECATED:
	void moveSmoothEnd(float *base, float target, float duration, int smoothFactor = 20, float delay = 0.0f);

	void deleteExistingAnimation(float *base);

	bool isAnimating(float *base);

private:
	void overrideExistingAnimation(float *base);

	enum ANIMATION_TYPE
	{
		MOVE_LINEAR,
		MOVE_SMOOTH_END,
		MOVE_QUAD_INOUT,
		MOVE_QUAD_IN,
		MOVE_QUAD_OUT
	};

	struct Animation
	{
		float *m_fBase;
		float *m_fTarget;
		float m_fDuration;

		float m_fStartValue;
		float m_fStartTime;
		ANIMATION_TYPE m_animType;

		float m_fFactor;
		bool m_bStarted;
	};

	std::vector<Animation*> m_vAnimations;
};

extern AnimationHandler *anim;

#endif
