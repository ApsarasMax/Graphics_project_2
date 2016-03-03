#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/ray.h"
#include <time.h>
#include <queue>
#include "scene/cubeMap.h"

class Scene;

class RayTracer
{
public:
	RayTracer();
        ~RayTracer();

	Vec3d tracePixel(int i, int j);
	Vec3d trace(double x, double y);
	Vec3d traceRay(ray& r, int depth);

	void getBuffer(unsigned char *&buf, int &w, int &h);
	double aspectRatio();

	void traceSetup( int w, int h );

	bool loadScene(char* fn);
	bool sceneLoaded() { return scene != 0; }

	void setReady(bool ready) { m_bBufferReady = ready; }
	bool isReady() const { return m_bBufferReady; }

	const Scene& getScene() { return *scene; }

	void setCubeMap(CubeMap* m) {
            if (cubeMap) delete cubeMap;
            cubeMap = m;
        }
    CubeMap *getCubeMap() {return cubeMap;}
    
    bool haveCubeMap() { return cubeMap != 0; }

public:
        unsigned char *buffer;
        int buffer_width, buffer_height;
        int bufferSize;
        Scene* scene;
        CubeMap* cubeMap = 0;

        bool m_bBufferReady;
};

#endif // __RAYTRACER_H__
