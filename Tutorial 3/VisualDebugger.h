#pragma once

#include "MyPhysicsEngine.h"

namespace VisualDebugger
{
	using namespace physx;

	///Init visualisation
	void Init(const char *window_name, int width=1280, int height=720);

	///Start visualisation
	void Start();
}

