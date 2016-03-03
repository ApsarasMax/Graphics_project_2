#pragma once

#include "../scene/material.h"
#include <iostream>
#include "ray.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

class CubeMap {

	TextureMap* tMap[6];
	int* kernel;

public:
	CubeMap() : kernel(0) { 
		for (int i = 0; i < 6; i++) tMap[i] = 0;
	}

	void setXposMap(TextureMap* m) {

		if(tMap[0] && tMap[0] != m) delete(tMap[0]);
		if (tMap[0] != m) tMap[0] = m;
	}
	void setXnegMap(TextureMap* m) {
		if(tMap[1] && tMap[1] != m) delete(tMap[1]);
		if (tMap[1] != m) tMap[1] = m;
	}
	void setYposMap(TextureMap* m) {
		if(tMap[2] && tMap[2] != m) delete(tMap[2]);
		if (tMap[2] != m) tMap[2] = m;
	}
	void setYnegMap(TextureMap* m) {
		if(tMap[3] && tMap[3] != m) delete(tMap[3]);
		if (tMap[3] != m) tMap[3] = m;
	}
	void setZposMap(TextureMap* m) {
		if(tMap[4] && tMap[4] != m) delete(tMap[4]);
		if (tMap[4] != m) tMap[4] = m;
	}
	void setZnegMap(TextureMap* m) {
		if(tMap[5] && tMap[5] != m) delete(tMap[5]);
		if (tMap[5] != m) tMap[5] = m;
	}

	Vec3d getColor(ray r) const{

		int filterwidth = traceUI->getFilterWidth();
		Vec3d dir = r.getDirection();
		Vec2d coord;
		double u,v;
		int front = 0;

		if( (fabs(dir[0])>fabs(dir[1])) && (fabs(dir[0])>fabs(dir[2])) ){
			double tmp = 0.5 / dir[0];
			double delta_y = tmp * dir[1];
			double delta_z = tmp * dir[2];
			if(tmp >0){
				front = 0;
				u = 0.5 - delta_z;
				v = 0.5 + delta_y;
			}else{
				front = 1;
				u = 0.5 - delta_z;
				v = 0.5 - delta_y;
			}
		}else if((fabs(dir[1])>fabs(dir[0])) && (fabs(dir[1])>fabs(dir[2]))){
			double tmp = 0.5 / dir[1];
			double delta_x = tmp * dir[0];
			double delta_z = tmp * dir[2];
			if(tmp >0){
				front = 2;
				u = 0.5 + delta_x;
				v = 0.5 - delta_z;
			}else{
				front = 3;
				u = 0.5 - delta_x;
				v = 0.5 - delta_z;
			}
		}else{
			double tmp = 0.5 / dir[2];
			double delta_x = tmp * dir[0];
			double delta_y = tmp * dir[1];
			if(tmp >0){
				front = 4;
				u = 0.5 + delta_x;
				v = 0.5 + delta_y;
			}else{
				front = 5;
				u = 0.5 + delta_x;
				v = 0.5 - delta_y;
			}

		}

		if (r.type() != ray::VISIBILITY || filterwidth == 1){
			return tMap[front]->getMappedValue(Vec2d(u, v));
		}

		int width = tMap[front]->getWidth();
		int height = tMap[front]->getHeight();
		double step=1.0/(width>height ? double(height) : double(width) );
		Vec3d col(0, 0, 0);

		for(int i=0;i<filterwidth;i++){
			for(int j=0; j<filterwidth;j++){
				coord[0]=u+i*step;
				coord[1]=v+j*step;
	   			col+=tMap[front]->getMappedValue(coord);
			}
		}
		return col/double(filterwidth*filterwidth);

	}

	~CubeMap() {
		for (int i = 0; i < 6; i++) 
			if (tMap[i]) { delete tMap[i]; tMap[i] = 0; }
		if (kernel) delete[] kernel;
	}
};
