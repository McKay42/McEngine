//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		asteroid and ship explosion particle effect container
//
// $NoKeywords: $
//====================================================================================//

#include "AsteroidsExplosion.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "ConVar.h"

#include "AsteroidsUtil.h"

ConVar asteroids_explosion_num_particles("asteroids_explosion_num_particles", 20, FCVAR_NONE);
ConVar asteroids_explosion_particle_min_speed("asteroids_explosion_particle_min_speed", 100.0f, FCVAR_NONE);
ConVar asteroids_explosion_particle_max_speed("asteroids_particle_max_speed", 200.0f, FCVAR_NONE);
ConVar asteroids_explosion_particle_duration("asteroids_explosion_particle_duration", 1.0f, FCVAR_NONE);

AsteroidsExplosion::AsteroidsExplosion(float x, float y, float radius)
{
	m_image = engine->getResourceManager()->getImage("IMAGE_BULLET");
	m_fStartTime = engine->getTime();

	// spawn particles
	{
		const int numParticlesToSpawn = std::max(0, asteroids_explosion_num_particles.getInt());

		m_particles.reserve(numParticlesToSpawn);
		for (int i=0; i<numParticlesToSpawn; i++)
		{
			PARTICLE particle;
			{
				particle.position = AsteroidsUtil::rotate(Vector2(0.0f, radius), AsteroidsUtil::randf(0.0f, 6.28f));
				particle.position.normalize();
				{
					particle.velocity = particle.position * AsteroidsUtil::randf(asteroids_explosion_particle_min_speed.getFloat(), asteroids_explosion_particle_max_speed.getFloat());
				}
				particle.position += Vector2(x, y);
			}
			m_particles.push_back(particle);
		}
	}
}

void AsteroidsExplosion::draw(Graphics *g)
{
	g->setColor(0xffffffff);
	g->setAlpha(clamp<float>(1.0f - (engine->getTime() - m_fStartTime) / asteroids_explosion_particle_duration.getFloat(), 0.0f, 1.0f));

	// NOTE: this is how you would draw every particle one by one, which is very slow
	/*
	for (size_t i=0; i<m_particles.size(); i++)
	{
		g->pushTransform();
		{
			g->translate(m_particles[i].position);
			g->drawImage(m_image);
		}
		g->popTransform();
	}
	*/

	// NOTE: so instead, we batch all particles into one single draw call
	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.empty();
	m_image->bind();
	{
		for (size_t i=0; i<m_particles.size(); i++)
		{
			const Vector2 topLeft = Vector2(m_particles[i].position.x - m_image->getWidth()/2, m_particles[i].position.y - m_image->getHeight()/2);
			const Vector2 topRight = topLeft + Vector2(m_image->getWidth(), 0);
			const Vector2 bottomLeft = topLeft + Vector2(0, m_image->getHeight());
			const Vector2 bottomRight = topRight + Vector2(0, m_image->getHeight());

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
	m_image->unbind();
}

void AsteroidsExplosion::update(float dt)
{
	// move particles
	for (size_t i=0; i<m_particles.size(); i++)
	{
		m_particles[i].position += m_particles[i].velocity * dt;
		m_particles[i].velocity *= 1.0f - (2.7f * dt);
	}
}

bool AsteroidsExplosion::isOver() const
{
	return (engine->getTime() - m_fStartTime > asteroids_explosion_particle_duration.getFloat());
}
