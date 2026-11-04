#pragma once

#include "RE/H/hkReferencedObject.h"

namespace RE
{
	class hkpConstraintData;
	class hkpConstraintOwner;
	struct hkpModifierConstraintAtom;

	class hkpConstraintInstance : public hkReferencedObject
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkpConstraintInstance;

		~hkpConstraintInstance() override;  // 00

		// add
		virtual void Unk_03(void);  // 03
		virtual void Unk_04(void);  // 04
		virtual void Unk_05(void);  // 05
		virtual void Unk_06(void);  // 06

		inline hkpRigidBody* GetRigidBodyA() const
		{
			return reinterpret_cast<hkpRigidBody*>(entities[0]);
		}

		inline hkpRigidBody* GetRigidBodyB() const
		{
			return reinterpret_cast<hkpRigidBody*>(entities[1]);
		}

		// members
		hkpConstraintOwner*        owner;                // 10
		hkpConstraintData*         data;                 // 18
		hkpModifierConstraintAtom* constraintModifiers;  // 20
		hkpEntity*                 entities[2];          // 28
	};
	static_assert(sizeof(hkpConstraintInstance) == 0x38);
}
