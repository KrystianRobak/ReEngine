#include "../Public/Engine/Systems/Physics/PhysicsSystem.h"
#include <iostream>
#include "../Public/Engine/Core/WindowManager.h"
#include "../Public/Engine/Components/Gravity.h"
#include "../Public/Engine/Components/RigidBody.h"
#include "../Public/Engine/Components/Thrust.h"
#include "../Public/Engine/Components/Transform.h"
#include "../Public/Engine/Components/Collision.h"
#include "../Public/Engine/Core/Coordinator/Coordinator.h"
#include "../Public/Engine/Components/Renderable.h"
#include "../Public/Engine/Components/Collision/Octree.h"
#include <memory>

void PhysicsSystem::Init()
{
}

void PhysicsSystem::Update(float dt)
{
	std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();


	std::vector<AABB*> allBoundingBoxes;

	for (auto entity : mEntities)
	{
		auto& transform = coordinator->GetComponent<Transform>(entity);
		// Forces
		auto const& gravity = coordinator->GetComponent<Gravity>(entity);

		auto& rigidBody = coordinator->GetComponent<RigidBody>(entity);

		glm::vec3 speed = rigidBody.velocity * dt * 20.f;

		rigidBody.velocity += gravity.force * dt;

		transform.position += speed;

		auto signature = coordinator->GetEntitySignature(entity);
		if (signature.test(coordinator->GetComponentType<Collision>()))
		{
			auto& collider = coordinator->GetComponent<Collision>(entity);
			collider.boundingBox.SetIsColliding(false);
			allBoundingBoxes.push_back(&collider.boundingBox);
		}
	}
	// Step 2: Detect potential collisions
	std::vector<std::pair<AABB*, AABB*>> collisions = checkCollisions(allBoundingBoxes);

	// Step 3: Handle detected collisions
	for (const auto& [a, b] : collisions)
	{
		a->SetIsColliding(true);
		b->SetIsColliding(true);
		std::cout << "Collision detected between objects!\n";
	}
}