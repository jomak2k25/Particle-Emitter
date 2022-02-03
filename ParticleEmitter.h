#pragma once
#include <vector>

struct Vector3Float
{
	float x, y, z;
};

namespace Emission_policies
{
	class ConeEmission				//Emits particles in cone shape 
	{
		Vector3Float	m_dir;		//Direction of the cone
		float			m_maxAngle;	//Maximum angle of emission around the direction
	protected:
		void Emit();
	};
	class SphereEmission			//Simply emits randomly in all directions from a point
	{
	protected:
		void Emit();
	};
	class CircleEmission			//Emits particles in random directions on a plan defined by a normal vector
	{
		Vector3Float m_normal;		//The normal of the circle can be used to 
	protected:
		void Emit();
	};
}

namespace Motion_policies			//These are used to define how the particles will move after emission
{
	class Accelerating
	{
		float m_initVel;			//The initial velocity of the particles when emitted
		float m_acceleration;		//The rate of acceleration for particles
	protected:
		void UpdatePositions();
	};
	class Constant
	{
		float m_vel;				//The velocity of the particles when emitted
	protected:
		void UpdatePositions();
	};
	class WithGravity
	{
		float m_vel;				//The velocity of emitted particles
		float m_gravity;			//The strength of gravity for the particles
	protected:
		void UpdatePositions();
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
		Vector3Float m_bounds;		//Defines how far in each direction a particle can travel before being culled
	protected:
		void DeleteParticles();
	};
	class SphereBoundaries
	{
		float m_maxDistance;		//Defines how far a particle can travel from the emitted befor it is culled
	protected:
		void DeleteParticles();
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
