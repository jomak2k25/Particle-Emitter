#pragma once
#include <vector>
#include "../../Common/MathHelper.h"

struct Vector3Float
{
	float x, y, z;
};

namespace Emission_Policies
{
	class ConeEmission				//Emits particles in cone shape 
	{
		Vector3Float	m_dir;		//Direction of the cone
		float			m_maxAngle;	//Maximum angle of emission around the direction
	protected:
		Emit();
	};
	class SphereEmission			//Simply emits randomly in all directions from a point
	{
	protected:
		Emit();
	};
	class CircleEmission			//Emits particles in random directions on a plan defined by a normal vector
	{
		Vector3Float m_normal;		//The normal of the circle can be used to 
	protected:
		Emit();
	};
}

namespace Motion_Policies			//These are used to define how the particles will move after emission
{
	class Accelerating
	{
		float m_initVel;			//The initial velocity of the particles when emitted
		float m_acceleration;		//The rate of acceleration for particles
	protected:
		UpdatePositions();
	};
	class Constant
	{
		float m_Vel;				//The velocity of the particles when emitted
	protected:
		UpdatePositions();
	};
	class WithGravity
	{
		float m_vel;				//The velocity of emitted particles
		float m_gravity;			//The strength of gravity for the particles
	protected:
		UpdatePositions();
	}
}

namespace Deletion_Policies			//These are used to define how when particles are culled
{
	class LifeSpan
	{
		float m_maxLifeTime;		//This is used to define how long, in seconds, a particle has before being culled
	protected:
		DeleteParticles();
	};
	class CubeBoundaries
	{
		Vector3Float m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		DeleteParticles();
	};
	class SphereBoundaries
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		DeleteParticles();
	};
}


template<class Particle, class Emission, class Motion, class Deletion>
class ParticleEmitter : public Emission, public Motion, public Deletion
{
	std::vector<Particle>	m_vParticles;
	Vector3Float			m_Pos;
public:

	void Update(float deltaTime);

	void StartEmission();
	void StopEmission();

	void SetEmissionRate();
	void SetMaxParticles();
};