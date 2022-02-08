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
	RenderItem		render_item;
	DirectX::XMFLOAT3		direction;
	float			age;
	bool			alive;
	Particle()
		:render_item(), direction(0.0f, 0.0f, 0.0f), age(0.0f), alive(false)
	{}
	void Reset() { direction = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f); age = 0.0f; alive = false; }
};

namespace Emission_policies
{
	constexpr float g_defaultEmitInterval = 0.2f;
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
		SphereEmission(DirectX::XMFLOAT3 pos):m_spawnPos(pos), m_emitInterval(g_defaultEmitInterval), m_spawnTime(0.0f)
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
		float m_initSpeed;			//The initial velocity of the particles when emitted
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
	constexpr float g_defaultMaxLifeTime = 2.0f;
	class LifeSpan
	{
		float m_maxLifeTime;		//This is used to define how long, in seconds, a particle has before being culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles);
		LifeSpan() :m_maxLifeTime(g_defaultMaxLifeTime)
		{}
	};
	class CubeBoundaries
	{
		DirectX::XMFLOAT3 m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles){}
	};
	class SphereBoundaries
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles){}
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
		int cbOffset = initParticle.render_item.ObjCBIndex;
		std::generate(m_vParticles.begin(), m_vParticles.end(),[&]() 
			{Particle p = initParticle;
			p.render_item.ObjCBIndex = cbOffset++;
			return p;});
		DirectX::XMStoreFloat4x4(&m_vParticles[0].render_item.World, DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z));
		m_vParticles[0].direction = DirectX::XMFLOAT3(0.2f, 0.2f, 0.0f);
		m_vParticles[0].alive = true;
	}
	void Update(float deltaTime)
	{
		Emit(deltaTime, m_vParticles);
		UpdatePositions(deltaTime, m_vParticles);
		DeleteParticles(deltaTime, m_vParticles);
	}

	void UpdateParticleCBs(UploadBuffer<ObjectConstants>* currObjectCB)
	{
		for (auto& p : m_vParticles)
		{
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (p.render_item.NumFramesDirty > 0)
			{
				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&p.render_item.World);
				DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&p.render_item.TexTransform);

				ObjectConstants objConstants;
				DirectX::XMStoreFloat4x4(&objConstants.World, DirectX::XMMatrixTranspose(world));
				DirectX::XMStoreFloat4x4(&objConstants.TexTransform, DirectX::XMMatrixTranspose(texTransform));

				currObjectCB->CopyData(p.render_item.ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				--p.render_item.NumFramesDirty;
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
			if (p.alive)
			{

				cmdList->IASetVertexBuffers(0, 1, &p.render_item.Geo->VertexBufferView());
				cmdList->IASetIndexBuffer(&p.render_item.Geo->IndexBufferView());
				cmdList->IASetPrimitiveTopology(p.render_item.PrimitiveType);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCBResource->GetGPUVirtualAddress() + p.render_item.ObjCBIndex * objCBByteSize;
				D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCBResource->GetGPUVirtualAddress() + p.render_item.Mat->MatCBIndex * matCBByteSize;

				cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

				cmdList->DrawIndexedInstanced(p.render_item.IndexCount, 1, p.render_item.StartIndexLocation, p.render_item.BaseVertexLocation, 0);

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
