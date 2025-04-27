#ifndef __SPIRALBULLET_H__
#define __SPIRALBULLET_H__

#include "Bullet.h"

class SpiralBullet : public Bullet
{
public:
    SpiralBullet(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r, int ttl);
    virtual void Update(int t) override;

private:
    float mSpiralAngle;
};

#endif
