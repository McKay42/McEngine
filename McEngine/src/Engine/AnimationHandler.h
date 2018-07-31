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

	// base
	void moveLinear(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuadIn(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuadOut(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuadInOut(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveCubicIn(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveCubicOut(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuartIn(float *base, float target, float duration, float delay, bool overrideExisting = false);
	void moveQuartOut(float *base, float target, float duration, float delay, bool overrideExisting = false);

	// simplified, without delay
	void moveLinear(float *base, float target, float duration, bool overrideExisting = false) {moveLinear(base, target, duration, 0.0f, overrideExisting);}
	void moveQuadIn(float *base, float target, float duration, bool overrideExisting = false) {moveQuadIn(base, target, duration, 0.0f, overrideExisting);}
	void moveQuadOut(float *base, float target, float duration, bool overrideExisting = false) {moveQuadOut(base, target, duration, 0.0f, overrideExisting);}
	void moveQuadInOut(float *base, float target, float duration, bool overrideExisting = false) {moveQuadInOut(base, target, duration, 0.0f, overrideExisting);}
	void moveCubicIn(float *base, float target, float duration, bool overrideExisting = false) {moveCubicIn(base, target, duration, 0.0f, overrideExisting);}
	void moveCubicOut(float *base, float target, float duration, bool overrideExisting = false) {moveCubicOut(base, target, duration, 0.0f, overrideExisting);}
	void moveQuartIn(float *base, float target, float duration, bool overrideExisting = false) {moveQuartIn(base, target, duration, 0.0f, overrideExisting);}
	void moveQuartOut(float *base, float target, float duration, bool overrideExisting = false) {moveQuartOut(base, target, duration, 0.0f, overrideExisting);}

	// DEPRECATED:
	void moveSmoothEnd(float *base, float target, float duration, int smoothFactor = 20, float delay = 0.0f);

	void deleteExistingAnimation(float *base);

	bool isAnimating(float *base);

private:
	enum class ANIMATION_TYPE
	{
		MOVE_LINEAR,
		MOVE_SMOOTH_END,
		MOVE_QUAD_INOUT,
		MOVE_QUAD_IN,
		MOVE_QUAD_OUT,
		MOVE_CUBIC_IN,
		MOVE_CUBIC_OUT,
		MOVE_QUART_IN,
		MOVE_QUART_OUT
	};

	struct Animation
	{
		float *m_fBase;
		float m_fTarget;
		float m_fDuration;

		float m_fStartValue;
		float m_fStartTime;
		ANIMATION_TYPE m_animType;

		float m_fFactor;
		bool m_bStarted;
	};

	void addAnimation(float *base, float target, float duration, float delay, bool overrideExisting, ANIMATION_TYPE type, float smoothFactor = 0.0f);
	void overrideExistingAnimation(float *base);

	std::vector<Animation> m_vAnimations;
};

extern AnimationHandler *anim;

#endif
