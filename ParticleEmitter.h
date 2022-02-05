#pragma once
#include <vector>
#include <algorithm>
#include <d3d12.h>
#include <DirectXMath.h>
#include "FrameResource.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
//using namespace DirectX;

struct Vector3Float
{
	Vector3Float(float xInit, float yInit, float zInit)
		:x(xInit), y(yInit), z(zInit)
	{}
	float x, y, z;
};

struct Particle
{
	RenderItem		m_renderItem;
	DirectX::XMFLOAT3		m_vel;
	float			m_age;
	bool			m_alive;
	Particle()
		:m_renderItem(), m_vel(0.0f, 0.0f, 0.0f), m_age(0.0f), m_alive(false)
	{}
};

namespace Emission_policies
{
	constexpr float g_default_emit_interval = 0.2f;
	class ConeEmission				//Emits particles in cone shape 
	{
		DirectX::XMFLOAT3		m_spawnPos;	//The positon for spawning objects
		float			m_emitInterval;//Frequency of particle emission
		float			m_spawnTime;//An accumalative float which totals delta time and is decreased by spawning particles
		DirectX::XMFLOAT3		m_dir;		//Direction of the cone
		float			m_maxAngle;	//Maximum angle of emission around the direction
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles){}
	public:
		ConeEmission(){}
	};
	class SphereEmission			//Simply emits randomly in all directions from a point
	{
		DirectX::XMFLOAT3		m_spawnPos;	//The positon for spawning objects
		float			m_emitInterval;//Frequency of particle emission
		float			m_spawnTime;//An accumalative float which totals delta time and is decreased by spawning particles
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles);
		SphereEmission(DirectX::XMFLOAT3 pos):m_spawnPos(pos), m_emitInterval(g_default_emit_interval), m_spawnTime(0.0f)
		{}
	};
	class CircleEmission			//Emits particles in random directions on a plan defined by a normal vector
	{
		DirectX::XMFLOAT3		m_spawnPos;	//The positon for spawning objects
		float			m_emitInterval;//Frequency of particle emission
		float			m_spawnTime;//An accumalative float which totals delta time and is decreased by spawning particles
		Vector3Float m_normal;		//The normal of the circle can be used to 
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles){}
	};
}

namespace Update_policies			//These are used to define how the particles will move after emission
{
	class Accelerating
	{
		float m_initVel;			//The initial velocity of the particles when emitted
		float m_acceleration;		//The rate of acceleration for particles
	protected:
		void UpdatePositions(float deltaTime, std::vector<Particle>& particles){}
	};
	class Constant
	{
		float m_speed;				//The velocity of the particles when emitted
	protected:
		void UpdatePositions(float deltaTime, std::vector<Particle>& particles);
	};
	class WithGravity
	{
		float m_speed;				//The velocity of emitted particles
		float m_gravity;			//The strength of gravity for the particles
	protected:
		void UpdatePositions(float deltaTime, std::vector<Particle>& particles){}
	};
}

namespace Deletion_policies			//These are used to define how when particles are culled
{
	class LifeSpan
	{
		float m_maxLifeTime;		//This is used to define how long, in seconds, a particle has before being culled
	protected:
		void DeleteParticles(std::vector<Particle>& particles);
	};
	class CubeBoundaries
	{
		DirectX::XMFLOAT3 m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		void DeleteParticles(std::vector<Particle>& particles){}
	};
	class SphereBoundaries
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		void DeleteParticles(std::vector<Particle>& particles){}
	};
}


template<class Emission, class Update, class Deletion>
class ParticleEmitter : public Emission, public Update, public Deletion
{
	DirectX::XMFLOAT3				m_pos;				//Position of the emitter
	std::vector<Particle>	m_vParticles;		//Stores the particle objects

	using Emission::Emit;
	using Update::UpdatePositions;
	using Deletion::DeleteParticles;
public:
	ParticleEmitter()
		:m_pos(0.0f,0.0f,0.0f), m_vParticles(50), Emission(m_pos)  //MOVE POLICY VALUES TO PUBLIC SETTERS
	{}

	void Init(Particle initParticle)
	{
		int cbOffset = initParticle.m_renderItem.ObjCBIndex;
		std::generate(m_vParticles.begin(), m_vParticles.end(),[&]() 
			{Particle p = initParticle;
			p.m_renderItem.ObjCBIndex = cbOffset++;
			return p;});
		DirectX::XMStoreFloat4x4(&m_vParticles[0].m_renderItem.World, DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z));
		m_vParticles[0].m_vel = DirectX::XMFLOAT3(0.2f, 0.2f, 0.0f);
		m_vParticles[0].m_alive = true;
	}
	void Update(float deltaTime)
	{
		Emit(deltaTime, m_vParticles);
		UpdatePositions(deltaTime, m_vParticles);
		DeleteParticles(m_vParticles);
	}

	void UpdateParticleCBs(UploadBuffer<ObjectConstants>* currObjectCB)
	{
		for (auto& p : m_vParticles)
		{
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (p.m_renderItem.NumFramesDirty > 0)
			{
				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&p.m_renderItem.World);
				DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&p.m_renderItem.TexTransform);

				ObjectConstants objConstants;
				DirectX::XMStoreFloat4x4(&objConstants.World, DirectX::XMMatrixTranspose(world));
				DirectX::XMStoreFloat4x4(&objConstants.TexTransform, DirectX::XMMatrixTranspose(texTransform));

				currObjectCB->CopyData(p.m_renderItem.ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				--p.m_renderItem.NumFramesDirty;
			}
		}
	}

	void DrawParticles(ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* objCBResource, ID3D12Resource* matCBResource)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ToonMaterialConstants));
		// For each render item...
		for (size_t i = 0; i < m_vParticles.size(); ++i)
		{
			Particle p = m_vParticles[i];
			if (p.m_alive)
			{

				cmdList->IASetVertexBuffers(0, 1, &p.m_renderItem.Geo->VertexBufferView());
				cmdList->IASetIndexBuffer(&p.m_renderItem.Geo->IndexBufferView());
				cmdList->IASetPrimitiveTopology(p.m_renderItem.PrimitiveType);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCBResource->GetGPUVirtualAddress() + p.m_renderItem.ObjCBIndex * objCBByteSize;
				D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCBResource->GetGPUVirtualAddress() + p.m_renderItem.Mat->MatCBIndex * matCBByteSize;

				cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

				cmdList->DrawIndexedInstanced(p.m_renderItem.IndexCount, 1, p.m_renderItem.StartIndexLocation, p.m_renderItem.BaseVertexLocation, 0);

			}
		}
	}

	void StartEmission(){}
	void StopEmission(){}

	void SetEmissionRate(){}
	void SetMaxParticles(){}

	void SetPosition(DirectX::XMFLOAT3 newPos) { m_pos = newPos; }

	std::vector<Particle>& GetParticles() { return m_vParticles; }
};
