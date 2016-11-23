//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		global fps independent animations
//
// $NoKeywords: $anim
//===============================================================================//

#include "AnimationHandler.h"

#include "Engine.h"
#include "ConVar.h"

ConVar debug_anim("debug_anim", false);

AnimationHandler *anim = NULL;

AnimationHandler::AnimationHandler()
{
	anim = this;
}

AnimationHandler::~AnimationHandler()
{
	for (int i=0; i<m_vAnimations.size(); i++)
	{
		delete m_vAnimations[i]->m_fTarget;
		delete m_vAnimations[i];
	}
	anim = NULL;
}

void AnimationHandler::update()
{
	for (int i=0; i<m_vAnimations.size(); i++)
	{
		// start animation
		Animation *anim = m_vAnimations[i];
		if (engine->getTime() < anim->m_fStartTime)
			continue;
		else if (!anim->m_bStarted)
		{
			// after our delay, take the current value as startValue, then start animating to the target
			anim->m_fStartValue = anim->m_fBase[0];
			anim->m_bStarted = true;
		}

		// calculate percentage
		float percent = clamp<float>((engine->getTime() - anim->m_fStartTime) / (anim->m_fDuration), 0.0f, 1.0f);

		if (debug_anim.getBool())
			debugLog("animation #%i, percent = %f\n", i, percent);

		// check if finished
		if (percent >= 1.0f)
		{
			anim->m_fBase[0] = anim->m_fTarget[0];

			if (debug_anim.getBool())
				debugLog("removing animation #%i, dtime = %f\n", i, engine->getTime()-anim->m_fStartTime);

			delete anim->m_fTarget;
			delete anim;
			m_vAnimations.erase(m_vAnimations.begin()+i);
			i--;
			continue;
		}

		// modify percentage
		switch(anim->m_animType)
		{
		case ANIMATION_TYPE::MOVE_LINEAR:
			anim->m_fBase[0] = anim->m_fStartValue + percent*(anim->m_fTarget[0] - anim->m_fStartValue);
			break;
		case ANIMATION_TYPE::MOVE_SMOOTH_END:
			percent = clamp<float>(1-std::pow(1-percent,anim->m_fFactor),0.0f,1.0f);
			if ((int)(percent*(anim->m_fTarget[0] - anim->m_fStartValue) + anim->m_fStartValue) == (int)anim->m_fTarget[0])
				percent = 1.0f;
			break;
		case ANIMATION_TYPE::MOVE_QUAD_IN:
			percent = percent*percent;
			break;
		case ANIMATION_TYPE::MOVE_QUAD_OUT:
			percent = -percent*(percent-2);
			break;
		case ANIMATION_TYPE::MOVE_QUAD_INOUT:
			if ((percent *= 2) < 1)
				percent = 0.5f*percent*percent;
			else
			{
				percent -= 1;
				percent = -0.5f * ((percent)*(percent-2) - 1);
			}
			break;
		case ANIMATION_TYPE::MOVE_CUBIC_IN:
			percent = percent*percent*percent;
			break;
		case ANIMATION_TYPE::MOVE_CUBIC_OUT:
			percent = percent - 1;
			percent = percent*percent*percent + 1.0f;
			break;
		case ANIMATION_TYPE::MOVE_QUART_IN:
			percent = percent*percent*percent*percent;
			break;
		case ANIMATION_TYPE::MOVE_QUART_OUT:
			percent = percent -1;
			percent = 1.0f - percent*percent*percent*percent;
			break;
		}

		// set new value
		anim->m_fBase[0] = anim->m_fStartValue + percent*(anim->m_fTarget[0] - anim->m_fStartValue);
	}

	if (m_vAnimations.size() > 512)
		debugLog("WARNING: AnimationHandler has %i animations!\n", m_vAnimations.size());

	//printf("AnimStackSize = %i\n", m_vAnimations.size());
}

void AnimationHandler::moveLinear(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_LINEAR);
}

void AnimationHandler::moveQuadIn(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_QUAD_IN);
}

void AnimationHandler::moveQuadOut(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_QUAD_OUT);
}

void AnimationHandler::moveQuadInOut(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_QUAD_INOUT);
}

void AnimationHandler::moveCubicIn(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_CUBIC_IN);
}

void AnimationHandler::moveCubicOut(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_CUBIC_OUT);
}

void AnimationHandler::moveQuartIn(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_QUART_IN);
}

void AnimationHandler::moveQuartOut(float *base, float target, float duration, float delay, bool overrideExisting)
{
	addAnimation(base, target, duration, delay, overrideExisting, ANIMATION_TYPE::MOVE_QUART_OUT);
}

void AnimationHandler::moveSmoothEnd(float *base, float target, float duration, int smoothFactor, float delay)
{
	addAnimation(base, target, duration, delay, true, ANIMATION_TYPE::MOVE_SMOOTH_END, smoothFactor);
}

void AnimationHandler::addAnimation(float *base, float target, float duration, float delay, bool overrideExisting, AnimationHandler::ANIMATION_TYPE type, float smoothFactor)
{
	if (overrideExisting)
		overrideExistingAnimation(base);

	float *_target = new float;
	_target[0] = target;
	Animation *anim = new Animation();

	anim->m_fBase = base;
	anim->m_fTarget = _target;
	anim->m_fDuration = duration;
	anim->m_fStartValue = base[0];
	anim->m_fStartTime = engine->getTime() + delay;
	anim->m_animType = type;
	anim->m_fFactor = smoothFactor;
	anim->m_bStarted = delay == 0.0f;

	m_vAnimations.push_back(anim);
}

void AnimationHandler::overrideExistingAnimation(float *base)
{
	for (int i=0; i<m_vAnimations.size(); i++)
	{
		if (m_vAnimations[i]->m_fBase == base)
		{
			delete m_vAnimations[i]->m_fTarget;
			delete m_vAnimations[i];
			m_vAnimations.erase(m_vAnimations.begin()+i);
			i--;
		}
	}
}

void AnimationHandler::deleteExistingAnimation(float *base)
{
	for (int i=0; i<m_vAnimations.size(); i++)
	{
		if (m_vAnimations[i]->m_fBase == base)
		{
			delete m_vAnimations[i]->m_fTarget;
			delete m_vAnimations[i];
			m_vAnimations.erase(m_vAnimations.begin()+i);
			i--;
		}
	}
}

bool AnimationHandler::isAnimating(float *base)
{
	for (int i=0; i<m_vAnimations.size(); i++)
	{
		if (m_vAnimations[i]->m_fBase == base)
			return true;
	}
	return false;
}

