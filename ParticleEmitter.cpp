#include "ParticleEmitter.h"

void Emission_policies::SphereEmission::Emit(float deltaTime, std::vector<Particle>& particles)
{
	m_spawnTime += deltaTime;
	float tempSpawnTime(m_spawnTime);
	int spawnCount(0);
	while(tempSpawnTime > 0)
	{
		tempSpawnTime = -m_emitInterval;
		if(tempSpawnTime >= 0)
		{
			++spawnCount;
			m_spawnTime = tempSpawnTime;
		}
	}
	for(Particle& p : particles)
	{
		if(!p.alive)
		{
			//Resetting the particle and moving it back to the position of the particle emitter
			p.Reset();
			p.alive = true;
			DirectX::XMStoreFloat4x4(&p.render_item.World, DirectX::XMMatrixTranslation(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z));

			//Give the particle its direction



			if(--spawnCount == 0)
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
					DirectX::XMMatrixTranslation(p.direction.x * deltaTime, p.direction.y * deltaTime, p.direction.z * deltaTime)));
			p.render_item.NumFramesDirty = g_numFrameResources;
		}
	}
}

void Deletion_policies::LifeSpan::DeleteParticles(float deltaTime, std::vector<Particle>& particles)
{
	
}
