#pragma once
#include <vector>
#include <algorithm>
#include <d3d12.h>
#include <DirectXMath.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
using namespace DirectX;

struct Vector3Float
{
	Vector3Float(float xInit, float yInit, float zInit)
		:x(xInit), y(yInit), z(zInit)
	{}
	float x, y, z;
};

namespace Emission_policies
{
	class ConeEmission				//Emits particles in cone shape 
	{
		XMFLOAT3	m_dir;		//Direction of the cone
		float			m_maxAngle;	//Maximum angle of emission around the direction
	protected:
		void Emit(float deltaTime){};
	};
	class SphereEmission			//Simply emits randomly in all directions from a point
	{
	protected:
		void Emit(float deltaTime);
	};
	class CircleEmission			//Emits particles in random directions on a plan defined by a normal vector
	{
		Vector3Float m_normal;		//The normal of the circle can be used to 
	protected:
		void Emit(float deltaTime){};
	};
}

namespace Update_policies			//These are used to define how the particles will move after emission
{
	class Accelerating
	{
		float m_initVel;			//The initial velocity of the particles when emitted
		float m_acceleration;		//The rate of acceleration for particles
	protected:
		void UpdatePositions(float deltaTime){};
	};
	class Constant
	{
		float m_speed;				//The velocity of the particles when emitted
	protected:
		void UpdatePositions(float deltaTime);
	};
	class WithGravity
	{
		float m_speed;				//The velocity of emitted particles
		float m_gravity;			//The strength of gravity for the particles
	protected:
		void UpdatePositions(float deltaTime){};
	};
}

namespace Deletion_policies			//These are used to define how when particles are culled
{
	class LifeSpan
	{
		float m_maxLifeTime;		//This is used to define how long, in seconds, a particle has before being culled
	protected:
		void DeleteParticles();
	};
	class CubeBoundaries
	{
		XMFLOAT3 m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		void DeleteParticles(){};
	};
	class SphereBoundaries
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		void DeleteParticles(){};
	};
}


template<class Particle, class Emission, class Update, class Deletion>
class ParticleEmitter : public Emission, public Update, public Deletion
{
	XMFLOAT3				m_pos;				//Position of the emitter
	float					m_frequency;		//The amount of particles which can be emitted per second
	std::vector<Particle>	m_vParticles;		//Stores the particle objects

	using Emission::Emit;
	using Update::UpdatePositions;
	using Deletion::DeleteParticles;
public:
	ParticleEmitter()
		:m_pos(0.0f,0.0f,0.0f), m_frequency(10.f), m_vParticles(50)
	{}

	void Init(Particle initParticle)
	{
		std::fill(m_vParticles.begin(), m_vParticles.end(), initParticle);
		//Temporary
		m_vParticles[0].m_alive = true;
		XMStoreFloat4x4(&m_vParticles[0].m_renderItem.World, DirectX::XMMatrixTranslation(-3.0f, 1.5f, -10.0f + 5.0f));
	}
	void Update(float deltaTime)
	{
		Emit();
		UpdatePositions(deltaTime);
		DeleteParticles();
	};


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
	};

	void StartEmission(){};
	void StopEmission(){};

	void SetEmissionRate(){};
	void SetMaxParticles(){};

	void SetPosition(XMFLOAT3 newPos) { m_pos = newPos; };
};
