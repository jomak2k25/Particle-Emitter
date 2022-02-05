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
		if(!p.m_alive)
		{
			//Spawn particle here
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
		if(p.m_alive)
		{
			DirectX::XMStoreFloat4x4(&p.m_renderItem.World,
				DirectX::XMMatrixMultiply(XMLoadFloat4x4(&p.m_renderItem.World), 
					DirectX::XMMatrixTranslation(p.m_vel.x * deltaTime, p.m_vel.y * deltaTime, p.m_vel.z * deltaTime)));
			p.m_renderItem.NumFramesDirty = g_numFrameResources;
		}
	}
}

void Deletion_policies::LifeSpan::DeleteParticles(float deltaTime, std::vector<Particle>& particles)
{
	
}
