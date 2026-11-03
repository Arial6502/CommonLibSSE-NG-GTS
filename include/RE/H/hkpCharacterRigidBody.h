#pragma once

#include "RE/H/hkRefPtr.h"
#include "RE/H/hkpContactPointEvent.h"
#include "RE/H/hkContactPoint.h"
#include "RE/H/hkVector4.h"
#include "RE/H/hkpCharacterControl.h"
#include "RE/H/hkpWorldPostSimulationListener.h"

namespace RE
{

	class hkpCharacterRigidBodyListener;

	class hkpCharacterRigidBody : 
		public hkReferencedObject,            //00
		public hkpEntityListener,             //10
		public hkpWorldPostSimulationListener //18
	{
		public:
		inline static constexpr auto RTTI = RTTI_hkpCharacterRigidBody;

		~hkpCharacterRigidBody() override; //00

		struct SupportInfo
		{
			hkContactPoint m_point;
			hkpRigidBody*  m_rigidBody;
			hkVector4      m_surfaceVelocity;
		};

		struct VertPointInfo
		{
			hkContactPoint                 m_vertPoint;
			hkpSimpleConstraintContactMgr* m_mgr;
		};

		// override (hkpEntityListener)
		void EntityAddedCallback(hkpEntity* a_entity) override;    // 01
		void EntityRemovedCallback(hkpEntity* a_entity) override;  // 02

		// override (hkpWorldPostSimulationListener)
		void PostSimulationCallback(hkpWorld* a_world) override;  // 01

		// add
		virtual void                           CheckSupport(const hkStepInfo& a_stepInfo, hkpSurfaceInfo& a_ground);                                                   // 03
		virtual hkpSurfaceInfo::SupportedState GetSupportInfo(const hkStepInfo& a_stepInfo, hkArray<SupportInfo>& a_supportInfo) const;                                // 04
		virtual void                           GetGround(const hkArray<SupportInfo>& a_supportInfo, bool a_useDynamicBodyVelocities, hkpSurfaceInfo& a_ground) const;  // 05

		hkpRigidBody*                  m_character;
		hkpCharacterRigidBodyListener* m_listener;
		hkVector4                      m_up;
		float                          m_unweldingHeightOffsetFactor;
		float                          m_maxSlopeCosine;
		float                          m_maxSpeedForSimplexSolver;
		float                          m_supportDistance;
		float                          m_hardSupportDistance;
		hkVector4                      m_acceleration;
		float                          m_maxForce;
		hkArray<VertPointInfo>         m_verticalContactPoints;
	};

	static_assert(sizeof(hkpCharacterRigidBody) == 0x90);
}
