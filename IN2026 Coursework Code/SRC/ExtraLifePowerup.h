#ifndef __EXTRALIFEPOWERUP_H__
#define __EXTRALIFEPOWERUP_H__

#include "GameObject.h"

class ExtraLifePowerup : public GameObject
{
public:
	ExtraLifePowerup();
	virtual ~ExtraLifePowerup();

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
};

#endif