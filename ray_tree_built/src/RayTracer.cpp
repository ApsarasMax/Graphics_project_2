// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

//#include "ui/TraceUI.h"
#include "ui/GraphicalUI.h"//zyc
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;
extern GraphicalUI* graphicalUI;//zyc

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

Vec3d RayTracer::trace(double x, double y)
{
  // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) scene->intersectCache.clear();
  ray r(Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY);
  scene->getCamera().rayThrough(x,y,r);
  Vec3d ret = traceRay(r, traceUI->getDepth());
  ret.clamp();
  return ret;
}

Vec3d RayTracer::tracePixel(int i, int j)
{
	Vec3d col(0,0,0);

	if( ! sceneLoaded() ) return col;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	col = trace(x, y);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}


// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay(ray& r, int depth)
{
	isect i;
	Vec3d colorC;

	if(scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.
		const Material& m = i.getMaterial();
	  	colorC = m.shade(scene, r, i);

	  	if(depth <= 0){
	  		return colorC;
	  	}

	  	//reflection
	  	Vec3d N = i.N;
	    Vec3d L = -r.d; //opposite of ray direction
	    L.normalize();

	    //if(!(m.kr(i)[0]==0 && m.kr(i)[1]==0 && m.kr(i)[2]==0)){
	    Vec3d dirReflect = 2*max((L*N), 0.0)*N-L;
	    dirReflect.normalize();

	    ray reflect(r.at(i.t), dirReflect, ray :: REFLECTION);

	    colorC += m.kr(i) % traceRay(reflect, depth-1);
		//}

		//refraction
		double yita_i=1.0;// = m.index(i); //object = i
	    double yita_t=1.0;// = 1.0; //air =t
	    double yita=1.0;// = yita_i/yita_t;
		double cosine_i = N * L;
		double flag = 1;
	    if(cosine_i>0){
	    	yita_i = 1.0; //air = i
		    yita_t = m.index(i); //object =t
		    yita = yita_i/yita_t;
		    
	    }else{
	    	yita_i = m.index(i); //object = i
		    yita_t = 1.0; //air =t
		    yita = yita_i/yita_t;
		    colorC -= m.shade(scene, r, i);
		    
		    flag = -1;

		    double sine_i = sqrt(1 - cosine_i*cosine_i);
		    double sine_t = yita * sine_i;
		    if(sine_t>1){
		    	return colorC;
		    }
	    }

	    double cosine_t = sqrt(1-yita*yita*(1-cosine_i*cosine_i));

	    Vec3d dirRefract = (yita*cosine_i - cosine_t * flag) * N  - yita * L;
	    dirRefract.normalize();
	    ray refract(r.at(i.t), dirRefract, ray :: REFRACTION);
	    colorC += m.kt(i) % traceRay(refract, depth-1);

	  
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		colorC = Vec3d(0.0, 0.0, 0.0);
	}
	return colorC;
}

RayTracer::RayTracer()
	: scene(0), buffer(0), buffer_width(256), buffer_height(256), m_bBufferReady(false)
{}

RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn ) {
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos ) path = ".";
	else path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if( !sceneLoaded() ) return false;

	if(graphicalUI->m_kdtreeInfo){
		scene->buildKdTree(graphicalUI->m_nKdtreeMaxDepth, graphicalUI->m_nKdtreeLeafSize);
	}

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w*h*3);
	m_bBufferReady = true;
}

