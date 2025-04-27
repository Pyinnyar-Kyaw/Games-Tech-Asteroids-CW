#include <stdlib.h>
#include "GameUtil.h"
#include "ExtraLifePowerup.h"
#include "BoundingShape.h"
#include "Spaceship.h"

ExtraLifePowerup::ExtraLifePowerup(void) : GameObject("ExtraLifePowerup")
{
	// Random angle for the powerup
	mAngle = rand() % 360;
	mRotation = 0;
	// Random position within the game area
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	// Slow moving powerup
	mVelocity.x = 5.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 5.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}

ExtraLifePowerup::~ExtraLifePowerup()
{
}

bool ExtraLifePowerup::CollisionTest(shared_ptr<GameObject> o)
{
	// Only collide with spaceships, ignore all other objects
	if (o->GetType() != GameObjectType("Spaceship")) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void ExtraLifePowerup::OnCollision(const GameObjectList& objects)
{
	for (auto& obj : objects)
	{
		if (obj->GetType() == GameObjectType("Spaceship"))
		{
			auto spaceship = dynamic_pointer_cast<Spaceship>(obj);
			if (spaceship)
			{
				mWorld->FlagForRemoval(GetThisPtr());

			}
		}
	}
}