#include "SpiralBullet.h"
#include <cmath>

SpiralBullet::SpiralBullet(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r, int ttl)
    : Bullet(p, v, a, h, r, ttl), mSpiralAngle(0.0f)
{
}

void SpiralBullet::Update(int t)
{
    mSpiralAngle += 0.2f; //spiral frequency

    //perpendicular offset to simulate a spiral
    float spiralAmplitude = 30.0f; //wider spiral
    GLVector3f perpendicular(-mVelocity.y, mVelocity.x, 0);
    perpendicular.normalize();

    //oscillation
    GLVector3f spiralOffset = perpendicular * spiralAmplitude * sin(mSpiralAngle);

    //update position with spiral offset
    mPosition += spiralOffset * (t / 1000.0f);

    Bullet::Update(t);
}

