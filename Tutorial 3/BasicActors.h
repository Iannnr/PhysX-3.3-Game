#pragma once

#include "PhysicsEngine.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	///Plane class
	class Plane : public StaticActor
	{
	public:
		//A plane with default paramters: XZ plane centred at (0,0,0)
		Plane(PxVec3 normal = PxVec3(0.f, 1.f, 0.f), PxReal distance = .01f)
			: StaticActor(PxTransformFromPlaneEquation(PxPlane(normal, distance)))
		{
			CreateShape(PxPlaneGeometry());
		}
	};

	///Sphere class
	class Sphere : public DynamicActor
	{
	public:
		//a sphere with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m
		// - denisty: 1kg/m^3
		Sphere(const PxTransform& pose = PxTransform(PxIdentity), PxReal radius = 1.f, PxReal density = 1.5f)
			: DynamicActor(pose)
		{
			CreateShape(PxSphereGeometry(radius), density);
		}
	};

	///Box class
	class Box : public StaticActor
	{
	public:
		//a Box with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m x 1m x 1m
		// - denisty: 1kg/m^3
		Box(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};


	class Boxes : public DynamicActor
	{
	public:
		//a Box with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m x 1m x 1m
		// - denisty: 1kg/m^3
		Boxes(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(2.f, 2.f, 2.f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
			GetShape()->getActor()->setGlobalPose(PxTransform(PxVec3(-60.f, 0.5f, -20.f)));
			//GetShape()->setLocalPose(PxTransform(PxVec3(-0.f, -0.f, 0.0f), PxQuat((-PxPi / 2), PxVec3(0.f, 0.f, 1.f))));
		}
	};

	class Obstacle : public DynamicActor
	{
	public:
		//a Box with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m x 1m x 1m
		// - denisty: 1kg/m^3
		Obstacle(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};

	class Capsule : public DynamicActor
	{
	public:
		Capsule(const PxTransform& pose = PxTransform(PxIdentity), PxVec2 dimensions = PxVec2(1.f, 1.f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			CreateShape(PxCapsuleGeometry(dimensions.x, dimensions.y), density);
		}
	};


	//Distance joint with the springs switched on
	class DistanceJoint : public Joint
	{
	public:
		DistanceJoint(Actor* actor0, const PxTransform& localFrame0, Actor* actor1, const PxTransform& localFrame1)
		{
			PxRigidActor* px_actor0 = 0;
			if (actor0)
				px_actor0 = (PxRigidActor*)actor0->Get();

			joint = (PxJoint*)PxDistanceJointCreate(*GetPhysics(), px_actor0, localFrame0, (PxRigidActor*)actor1->Get(), localFrame1);
			joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
			((PxDistanceJoint*)joint)->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, true);
			Damping(1.f);
			Stiffness(1.f);
		}

		void Stiffness(PxReal value)
		{
			((PxDistanceJoint*)joint)->setStiffness(value);
		}

		PxReal Stiffness()
		{
			return ((PxDistanceJoint*)joint)->getStiffness();
		}

		void Damping(PxReal value)
		{
			((PxDistanceJoint*)joint)->setDamping(value);
		}

		PxReal Damping()
		{
			return ((PxDistanceJoint*)joint)->getDamping();
		}
	};

	class D6Joint : public Joint
	{
	public:
		D6Joint(Actor* actor0, const PxTransform& localFrame0, Actor* actor1, const PxTransform& localFrame1, bool x, bool y, bool z)
		{
			PxRigidActor* px_actor0 = 0;
			if (actor0)
				px_actor0 = (PxRigidActor*)actor0->Get();

			PxD6Joint* d6joint = PxD6JointCreate(*GetPhysics(), px_actor0, localFrame0, (PxRigidActor*)actor1->Get(), localFrame1);

			if (x)
			{
				d6joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
			}
			if (y)
			{
				d6joint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
			}
			if (z)
			{
				d6joint->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);
			}

			d6joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
			//void setLimit
		}
	};

	class PrismaticJoint : public Joint
	{
	public:
		PrismaticJoint(Actor* actor0, const PxTransform& localFrame0, Actor* actor1, const PxTransform& localFrame1)
		{
			PxRigidActor* px_actor0 = 0;
			if (actor0)
				px_actor0 = (PxRigidActor*)actor0->Get();

			PxPrismaticJoint* joint = PxPrismaticJointCreate(*GetPhysics(), px_actor0, localFrame0, (PxRigidActor*)actor1->Get(), localFrame1);
			
			//joint->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, true);
			joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
		}

		void setLimit(PxReal lower, PxReal upper, PxSpring spring)
		//void SetLimits(PxReal lower, PxReal upper, PxSpring spring)
		{
			//((PxPrismaticJoint*)joint)->setLimit(PxJointLinearLimitPair(;
			((PrismaticJoint*)joint)->setLimit(lower, upper, spring);
			((PxPrismaticJoint*)joint)->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, true);
		}
	};

	///Revolute Joint
	class RevoluteJoint : public Joint
	{
	public:
		RevoluteJoint(Actor* actor0, const PxTransform& localFrame0, Actor* actor1, const PxTransform& localFrame1)
		{
			PxRigidActor* px_actor0 = 0;
			if (actor0)
				px_actor0 = (PxRigidActor*)actor0->Get();

			joint = PxRevoluteJointCreate(*GetPhysics(), px_actor0, localFrame0, (PxRigidActor*)actor1->Get(), localFrame1);
			joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
		}

		void DriveVelocity(PxReal value)
		{
			//wake up the attached actors
			PxRigidDynamic *actor_0, *actor_1;
			((PxRevoluteJoint*)joint)->getActors((PxRigidActor*&)actor_0, (PxRigidActor*&)actor_1);
			if (actor_0)
			{
				if (actor_0->isSleeping())
					actor_0->wakeUp();
			}
			if (actor_1)
			{
				if (actor_1->isSleeping())
					actor_1->wakeUp();
			}
			((PxRevoluteJoint*)joint)->setDriveVelocity(value);
			((PxRevoluteJoint*)joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		}

		PxReal DriveVelocity()
		{
			return ((PxRevoluteJoint*)joint)->getDriveVelocity();
		}

		void SetLimits(PxReal lower, PxReal upper)
		{
			((PxRevoluteJoint*)joint)->setLimit(PxJointAngularLimitPair(lower, upper));
			((PxRevoluteJoint*)joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		}
	};

	class OutSkirtsSides : public StaticActor
	{
	public:
		OutSkirtsSides(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(50.0f, 8.5f, 1.5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(25.f, 8.5f, 1.5f), density);
			CreateShape(PxBoxGeometry(25.f, 8.5f, 1.5f), density);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(-15.0f, 8.5f, -25.0f)));
			GetShape(1)->setLocalPose(PxTransform(PxVec3(-15.0f, 8.5f, 25.0f)));
			GetShape(2)->setLocalPose(PxTransform(PxVec3(-65.5f, 8.5f, .0f), PxQuat(1.5708f, PxVec3(0.f, 1.f, 0.f))));
			GetShape(3)->setLocalPose(PxTransform(PxVec3(35.5f, 8.5f, .0f), PxQuat(1.5708f, PxVec3(0.f, 1.f, 0.f))));
		}
	};

	class Player : public DynamicActor
	{
	public:
		Player(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(0.15f, 2.5f, 4.5f), PxReal density = 2.5f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};

	class Flipper : public DynamicActor
	{
	public:
		Flipper(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(3.f, .5f, 7.f), PxReal density = .25f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};

	class Goals : public StaticActor
	{
	public:
		Goals(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(11.5f, 5.5f, .75f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(-64.6f, 5.5f, .0f), PxQuat(1.5708f, PxVec3(0.f, 1.f, 0.f))));
		}
	};

	class MotorArms : public DynamicActor
	{
	public:
		MotorArms(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(0.5f, .5f, 6.5f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi, PxVec3(0.f, 1.f, 0.f))));
			GetShape(1)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))));
			GetShape(2)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi / 4, PxVec3(0.f, -1.f, 0.f))));
			GetShape(3)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi / 4, PxVec3(0.f, 1.f, 0.f))));
		}
	};
}
