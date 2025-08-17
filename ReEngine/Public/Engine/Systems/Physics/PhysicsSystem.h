#pragma once


#include "System/System.h"




REFSYSTEM()
class PhysicsSystem : public System
{
public:
	REFFUNCTION()
	void Init();

	REFFUNCTION()
	void Update(float dt);
};
