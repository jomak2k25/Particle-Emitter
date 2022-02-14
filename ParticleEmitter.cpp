#include "ParticleEmitter.h"
#include "random"

inline void NormalizeFloat3(DirectX::XMFLOAT3& vec)
{
	const float LENGTH = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
	vec = DirectX::XMFLOAT3(vec.x / LENGTH, vec.y / LENGTH, vec.z / LENGTH);
}

void Emission_policies::SphereEmission::Emit(float deltaTime, std::vector<Particle>& particles)
{
	m_spawnTime += deltaTime;
	int spawnCount(0);
	while(m_spawnTime > m_emitInterval)
	{
		++spawnCount;
		m_spawnTime -= m_emitInterval;
	}
	if(spawnCount == 0)
	{
		return;
	}
	for(Particle& p : particles)
	{
		if(!p.alive)
		{
			//Resetting the particle and moving it back to the position of the particle emitter
			p.alive = true;
			DirectX::XMStoreFloat4x4(&p.render_item.World, DirectX::XMMatrixTranslation(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z));

			//Give the particle its direction
			p.direction.x = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) - 0.5f);
			p.direction.y = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) - 0.5f);
			p.direction.z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) - 0.5f);

			NormalizeFloat3(p.direction);
			if (--spawnCount <= 0)
			{
				break;
			}
		}
	}
}

void Update_policies::Constant::UpdatePositions(float deltaTime, std::vector<Particle>& particles)
{
	for(Particle& p : particles)
	{
		if(p.alive)
		{
			DirectX::XMStoreFloat4x4(&p.render_item.World,
				DirectX::XMMatrixMultiply(XMLoadFloat4x4(&p.render_item.World), 
					DirectX::XMMatrixTranslation(p.direction.x * deltaTime * m_speed, p.direction.y * deltaTime * m_speed, p.direction.z * deltaTime * m_speed)));
			p.render_item.NumFramesDirty = g_numFrameResources;
		}
	}
}

void Deletion_policies::LifeSpan::DeleteParticles(float deltaTime, std::vector<Particle>& particles)
{
	for(Particle& p : particles)
	{
		if(p.alive)
		{
			p.age += deltaTime;
			if(p.age > m_maxLifeTime)
			{
				p.Reset();
			}
		}
	}
}

void Deletion_policies::CubeBoundaries::DeleteParticles(float deltaTime, std::vector<Particle>& particles)
{
	for(Particle& p : particles)
	{
		if(p.alive)
		{
			p.age += deltaTime;
			if(true) //Do this
			{
				p.Reset();
			}
		}
	}
}

