#include <cmath>

#include "light.h"

using namespace std;

double DirectionalLight::distanceAttenuation(const Vec3d& P) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  Vec3d dirShadow = -orientation;

  ray shadow(p, dirShadow, ray :: SHADOW);
  isect i;

  if(scene->intersect(shadow, i)){
    return Vec3d(0, 0, 0);
  }

  return color;

}

Vec3d DirectionalLight::getColor() const
{
  return color;
}

Vec3d DirectionalLight::getDirection(const Vec3d& P) const
{
  // for directional light, direction doesn't depend on P
  return -orientation;
}

double PointLight::distanceAttenuation(const Vec3d& P) const
{

  // YOUR CODE HERE

  double d = (position - P).length();

  // You'll need to modify this method to attenuate the intensity 
  // of the light based on the distance between the source and the 
  // point P.  For now, we assume no attenuation and just return 1.0

  return min( 1.0, 1/( constantTerm + linearTerm * d + quadraticTerm * d * d ) );
}

Vec3d PointLight::getColor() const
{
  return color;
}

Vec3d PointLight::getDirection(const Vec3d& P) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  Vec3d dirShadow = position - p;
  dirShadow.normalize();

  ray shadow(p, dirShadow, ray :: SHADOW);
  isect i;

  if(scene->intersect(shadow, i)){
    double distLight = (position - p).length();
    double distIscet = (shadow.at(i.t) - p).length();
    if(distLight<distIscet){
      return color;
    }
    return Vec3d(0, 0, 0);
  }

  return color;
}
