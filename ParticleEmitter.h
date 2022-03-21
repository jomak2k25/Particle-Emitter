#pragma once
#include <vector>
#include <algorithm>
#include <ctime>
#include <random>

#include <d3d12.h>
#include <DirectXMath.h>
#include "FrameResource.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

struct Particle
{
	RenderItem				render_item;
	DirectX::XMFLOAT3		position;
	DirectX::XMFLOAT3		direction;
	float					age;
	bool					alive;
	Particle()
		:render_item(), direction(0.0f, 0.0f, 0.0f), age(0.0f), alive(false)
	{}
	void Reset() { direction = DirectX::XMFLOAT3(); age = 0.0f; alive = false; position = DirectX::XMFLOAT3(); }
};

namespace Emission_policies
{
	constexpr float g_defaultEmitInterval = 0.1f;
	class EmissionBase									//Base for emission policy classes
	{
	public:
		void SetSpawnPos(DirectX::XMFLOAT3 position) { m_spawnPos = position; }
	protected:
		virtual void Emit(float deltaTime, std::vector<Particle>& particles) = 0;
		DirectX::XMFLOAT3 m_spawnPos;					//Position for spawning particles
		float			m_spawnTime;					//An accumalative float which totals delta time and is decreased by spawning particles
		float			m_emitInterval;					//Frequency of particle emission
		EmissionBase():m_spawnPos(0.0f,0.0f,0.0f), m_spawnTime(0.0f), m_emitInterval(g_defaultEmitInterval)
		{
			srand(static_cast<unsigned>(time(nullptr)));
		}
	};

	class ConeEmission: public EmissionBase				//Emits particles in cone shape 
	{
		DirectX::XMFLOAT3		m_dir;		//Direction of the cone
		float			m_maxAngle;	//Maximum angle of emission around the direction
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles)override{}
	public:
		ConeEmission():EmissionBase(){}
	};

	class SphereEmission : public EmissionBase			//Simply emits randomly in all directions from a point
	{
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles) override;
		SphereEmission():EmissionBase()
		{}
	};

	class CircleEmission : public EmissionBase			//Emits particles in random directions on a plan defined by a normal vector
	{
		DirectX::XMFLOAT3 m_normal;		//The normal of the circle can be used to 
	protected:
		void Emit(float deltaTime, std::vector<Particle>& particles)override{};
	};
}

namespace Update_policies			//These are used to define how the particles will move after emission
{
	constexpr float g_defualtSpeed = 2.0f;
	class Accelerating
	{
		float m_initSpeed;			//The initial velocity of the particles when emitted
		float m_acceleration;		//The rate of acceleration for particles
	protected:
		void UpdatePositions(float deltaTime, std::vector<Particle>& particles) {}
	};
	class Constant
	{
		float m_speed;				//The velocity of the particles when emitted
	protected:
		void UpdatePositions(float deltaTime, std::vector<Particle>& particles);
		Constant() :m_speed(g_defualtSpeed) {};
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
	class DeletionBase
	{
	public:
		virtual void SetSpawnPos(DirectX::XMFLOAT3 pos) { m_spawnPos = pos; };
	protected:
		virtual void DeleteParticles(float deltaTime, std::vector<Particle>& particles) = 0;
		DirectX::XMFLOAT3 m_spawnPos;
	};
	constexpr float g_defaultMaxLifeTime = 2.0f;
	class LifeSpan : public DeletionBase
	{
		float m_maxLifeTime;		//This is used to define how long, in seconds, a particle has before being culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles) override;
		LifeSpan() :m_maxLifeTime(g_defaultMaxLifeTime)
		{}
	};
	class CubeBoundaries : public DeletionBase
	{
		struct Bounds
		{
			float xMin, xMax;
			float yMin, yMax;
			float zMin, zMax;
			Bounds(DirectX::XMFLOAT3 init) :xMin(-init.x), xMax(init.x), yMin(-init.y), yMax(init.y), zMin(-init.z), zMax(init.z){};
			Bounds() { memset(this, 0.0f, sizeof(Bounds)); }
		}m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles) override;
		void SetSpawnPos(DirectX::XMFLOAT3 pos) override;
		CubeBoundaries() :m_bounds(DirectX::XMFLOAT3{3.0f,3.0f,3.0f}){}
	};
	class SphereBoundaries : public DeletionBase
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		void DeleteParticles(float deltaTime, std::vector<Particle>& particles) override {}
	};
}


template<class Emission, class Update, class Deletion>
class ParticleEmitter : public Emission, public Update, public Deletion
{
	std::vector<Particle>	m_vParticles;		//Stores the particle objects

	using Emission::Emit;
	using Update::UpdatePositions;
	using Deletion::DeleteParticles;
public:
	ParticleEmitter()
		:Emission(), m_vParticles(50)  //MOVE POLICY VALUES TO PUBLIC SETTERS
	{}

	void Init(Particle initParticle, DirectX::XMFLOAT3 position)
	{
		int cbOffset = initParticle.render_item.ObjCBIndex;
		std::generate(m_vParticles.begin(), m_vParticles.end(),[&]() 
			{Particle p = initParticle;
			p.render_item.ObjCBIndex = cbOffset++;
			return p;});
		Emission::EmissionBase::SetSpawnPos(position);
		Deletion::SetSpawnPos(position);
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

	void StartEmission(){}	//TODO
	void StopEmission(){}  //TODO

	void SetEmissionRate(){}  //TODO
	void SetMaxParticles(){}  //TODO

	void SetPosition(DirectX::XMFLOAT3 newPos)
	{
		Emission::EmissionBase::SetSpawnPos(newPos);
		Deletion::DeletionBase::SetSpawnPos(newPos);
	}

	std::vector<Particle>& GetParticles() { return m_vParticles; }
};
