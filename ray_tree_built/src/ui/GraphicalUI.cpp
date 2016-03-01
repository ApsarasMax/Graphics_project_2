//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"

#include "../scene/scene.h"//zyc

#define MAX_INTERVAL 500

#ifdef _WIN32
#define print sprintf_s
#else
#define print sprintf
#endif

bool GraphicalUI::stopTrace = false;
bool GraphicalUI::doneTrace = true;
GraphicalUI* GraphicalUI::pUI = NULL;
char* GraphicalUI::traceWindowLabel = "Raytraced Image";
bool TraceUI::m_debug = false;

int	GraphicalUI::m_nKdtreeMaxDepth = 5; //zyc
int	GraphicalUI::m_nKdtreeLeafSize = 10; //zyc
Fl_Slider*	GraphicalUI::m_kdtreeMaxDepthSlider = nullptr;
Fl_Slider*	GraphicalUI::m_kdtreeLeafSizeSlider = nullptr;

bool GraphicalUI::m_kdtreeInfo = true;
bool GraphicalUI::m_antiAliaseInfo = true;
int	GraphicalUI::m_nAntiAliasingDegree = 3; //zyc
Fl_Slider*	GraphicalUI::m_antiAliasingDegreeSlider = nullptr;

bool GraphicalUI::m_cubeMapInfo = false;

//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ((GraphicalUI*)(o->parent()->user_data()));
}

//--------------------------------- Callback Functions --------------------------------------------
void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	static char* lastFile = 0;
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			print(buf, "Ray <%s>", newfile);
			stopTracing();	// terminate the previous rendering
		} else print(buf, "Ray <Not Loaded>");

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
			pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}
}

void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	pUI = whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	pUI = (GraphicalUI *)(o->user_data());

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project for CS384g.");
}

void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());

	// terminate the rendering so we don't get crashes
	stopTracing();

	pUI->m_nSize=int(((Fl_Slider *)o)->value());
	int width = (int)(pUI->getSize());
	int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow(width, height);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_refreshSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->refreshInterval=clock_t(((Fl_Slider *)o)->value()) ;
}

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_displayDebuggingInfo)
	  {
	    pUI->m_debuggingWindow->show();
	    pUI->m_debug = true;
	  }
	else
	  {
	    pUI->m_debuggingWindow->hide();
	    pUI->m_debug = false;
	  }
}

//kdtree
void GraphicalUI::cb_kdtreeCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_kdtreeInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_kdtreeInfo){
		pUI->m_kdtreeMaxDepthSlider->activate();
		pUI->m_kdtreeLeafSizeSlider->activate();
	}else{
		pUI->m_kdtreeMaxDepthSlider->deactivate();
		pUI->m_kdtreeLeafSizeSlider->deactivate();
	}

}

void GraphicalUI::cb_kdtreeMaxSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nKdtreeMaxDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_kdtreeLeafSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nKdtreeLeafSize=int( ((Fl_Slider *)o)->value() ) ;
}

//anti-aliasing
void GraphicalUI::cb_antiAliaseCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_antiAliaseInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_antiAliaseInfo){
		pUI->m_antiAliasingDegreeSlider->activate();
	}else{
		pUI->m_antiAliasingDegreeSlider->deactivate();
	}
}

void GraphicalUI::cb_antiAliasingDegreeSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nAntiAliasingDegree=int( ((Fl_Slider *)o)->value() ) ;
}

//cube map
void GraphicalUI::cb_cubeMapCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_cubeMapInfo = (((Fl_Check_Button*)o)->value() == 1);
	// if (pUI->m_antiAliaseInfo){
	// 	pUI->m_antiAliasingDegreeSlider->activate();
	// }else{
	// 	pUI->m_antiAliasingDegreeSlider->deactivate();
	// }

}

void GraphicalUI::cb_render(Fl_Widget* o, void* v) {

	char buffer[256];

	pUI = (GraphicalUI*)(o->user_data());
	doneTrace = stopTrace = false;

	
	if (pUI->raytracer->sceneLoaded())
	  {
		int width = pUI->getSize();
		int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		int origPixels = width * height;
		pUI->m_traceGlWindow->resizeWindow(width, height);
		pUI->m_traceGlWindow->show();
		pUI->raytracer->traceSetup(width, height);

		// Save the window label
                const char *old_label = pUI->m_traceGlWindow->label();

		clock_t now, prev;
		now = prev = clock();
		clock_t intervalMS = pUI->refreshInterval * 100;
		for (int y = 0; y < height; y++)
		  {
		    for (int x = 0; x < width; x++)
		      {
			if (stopTrace) break;
			// check for input and refresh view every so often while tracing
			now = clock();
			if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
			  {
			    prev = now;
			    sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
			    pUI->m_traceGlWindow->label(buffer);
			    pUI->m_traceGlWindow->refresh();
			    Fl::check();
			    if (Fl::damage()) { Fl::flush(); }
			  }
			// look for input and refresh window
			pUI->raytracer->tracePixel(x, y);
			pUI->m_debuggingWindow->m_debuggingView->setDirty();
		      }
		    if (stopTrace) break;
		  }
		doneTrace = true;
		stopTrace = false;
		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);
		pUI->m_traceGlWindow->refresh();

		if(m_antiAliaseInfo){
			unsigned char *buf;
			int width, height;
			pUI->getRayTracer()->getBuffer(buf, width, height);
			
			double degree = pUI->m_nAntiAliasingDegree;


			for( int j = 0; j < height; ++j ){
				for( int i = 0; i < width; ++i ){
					Vec3d col = Vec3d(0, 0, 0);
					if (stopTrace) break;
					double x = double(i)/double(width);
					double y = double(j)/double(height);
					double deltaW = 1.0 / (double) width / degree;
			 		double deltaH = 1.0 / (double) height / degree; 
			 		// double deltaW = 1.0 / (double)degree;
			 		// double deltaH = 1.0 / (double)degree; 
					for (int i1 = 0; i1 < degree; i1 ++){
						for (int ji = 0; ji < degree; ji ++){
							col += pUI->getRayTracer()->trace(x + deltaW * i1, y + deltaH * ji);
						}
					}

					col /= (degree*degree);
					unsigned char *pixel = buf + ( i + j * width ) * 3;
					pixel[0] = (int)( 255.0 * min(col[0], 1.0));
					pixel[1] = (int)( 255.0 * min(col[1], 1.0));
					pixel[2] = (int)( 255.0 * min(col[2], 1.0));
				}
			}

		}

	}
}

void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	stopTracing();
}

int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer(tracer);
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File", 0, 0, 0, FL_SUBMENU },
	{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
	{ "&Save Image...", FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
	{ "&Exit", FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
	{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
	{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
	{ 0 },

	{ 0 }
};

void GraphicalUI::stopTracing()
{
	stopTrace = true;
}

GraphicalUI::GraphicalUI() : refreshInterval(10) {
	// init.
	m_mainWindow = new Fl_Window(100, 40, 450, 459, "Ray <Not Loaded>");
	m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
	// install menu bar
	m_menubar = new Fl_Menu_Bar(0, 0, 440, 25);
	m_menubar->menu(menuitems);

	// set up "render" button
	m_renderButton = new Fl_Button(360, 37, 70, 25, "&Render");
	m_renderButton->user_data((void*)(this));
	m_renderButton->callback(cb_render);

	// set up "stop" button
	m_stopButton = new Fl_Button(360, 65, 70, 25, "&Stop");
	m_stopButton->user_data((void*)(this));
	m_stopButton->callback(cb_stop);

	// install depth slider
	m_depthSlider = new Fl_Value_Slider(10, 40, 180, 20, "Recursion Depth");
	m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_depthSlider->type(FL_HOR_NICE_SLIDER);
	m_depthSlider->labelfont(FL_COURIER);
	m_depthSlider->labelsize(12);
	m_depthSlider->minimum(0);
	m_depthSlider->maximum(10);
	m_depthSlider->step(1);
	m_depthSlider->value(m_nDepth);
	m_depthSlider->align(FL_ALIGN_RIGHT);
	m_depthSlider->callback(cb_depthSlides);

	// install size slider
	m_sizeSlider = new Fl_Value_Slider(10, 65, 180, 20, "Screen Size");
	m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_sizeSlider->type(FL_HOR_NICE_SLIDER);
	m_sizeSlider->labelfont(FL_COURIER);
	m_sizeSlider->labelsize(12);
	m_sizeSlider->minimum(64);
	m_sizeSlider->maximum(1024);
	m_sizeSlider->step(2);
	m_sizeSlider->value(m_nSize);
	m_sizeSlider->align(FL_ALIGN_RIGHT);
	m_sizeSlider->callback(cb_sizeSlides);

	// install refresh interval slider
	m_refreshSlider = new Fl_Value_Slider(10, 90, 180, 20, "Screen Refresh Interval (0.1 sec)");
	m_refreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_refreshSlider->type(FL_HOR_NICE_SLIDER);
	m_refreshSlider->labelfont(FL_COURIER);
	m_refreshSlider->labelsize(12);
	m_refreshSlider->minimum(1);
	m_refreshSlider->maximum(300);
	m_refreshSlider->step(1);
	m_refreshSlider->value(refreshInterval);
	m_refreshSlider->align(FL_ALIGN_RIGHT);
	m_refreshSlider->callback(cb_refreshSlides);

	// set up debugging display checkbox
	m_debuggingDisplayCheckButton = new Fl_Check_Button(10, 429, 140, 20, "Debugging display");
	m_debuggingDisplayCheckButton->user_data((void*)(this));
	m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
	m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);

	// set up kdtree implementation checkbox
	m_kdtreeCheckButton = new Fl_Check_Button(10, 165, 80, 20, "Kd-Tree");
	m_kdtreeCheckButton->user_data((void*)(this));
	m_kdtreeCheckButton->callback(cb_kdtreeCheckButton);
	m_kdtreeCheckButton->value(m_kdtreeInfo);

	// install ketree max depth slider
	m_kdtreeMaxDepthSlider = new Fl_Value_Slider(100, 150, 180, 20, "Max depth");
	m_kdtreeMaxDepthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_kdtreeMaxDepthSlider->type(FL_HOR_NICE_SLIDER);
	m_kdtreeMaxDepthSlider->labelfont(FL_COURIER);
	m_kdtreeMaxDepthSlider->labelsize(12);
	m_kdtreeMaxDepthSlider->minimum(1);
	m_kdtreeMaxDepthSlider->maximum(30);
	m_kdtreeMaxDepthSlider->step(1);
	m_kdtreeMaxDepthSlider->value(m_nKdtreeMaxDepth);
	m_kdtreeMaxDepthSlider->align(FL_ALIGN_RIGHT);
	m_kdtreeMaxDepthSlider->callback(cb_kdtreeMaxSlides);

	// install kdtree leaf size slider
	m_kdtreeLeafSizeSlider = new Fl_Value_Slider(100, 180, 180, 20, "Target Leaf size");
	m_kdtreeLeafSizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_kdtreeLeafSizeSlider->type(FL_HOR_NICE_SLIDER);
	m_kdtreeLeafSizeSlider->labelfont(FL_COURIER);
	m_kdtreeLeafSizeSlider->labelsize(12);
	m_kdtreeLeafSizeSlider->minimum(1);
	m_kdtreeLeafSizeSlider->maximum(100);
	m_kdtreeLeafSizeSlider->step(1);
	m_kdtreeLeafSizeSlider->value(m_nKdtreeLeafSize);
	m_kdtreeLeafSizeSlider->align(FL_ALIGN_RIGHT);
	m_kdtreeLeafSizeSlider->callback(cb_kdtreeLeafSlides);


	// set up anti aliasing implementation checkbox
	m_antiAliaseCheckButton = new Fl_Check_Button(10, 210, 80, 20, "~Aliase");
	m_antiAliaseCheckButton->user_data((void*)(this));
	m_antiAliaseCheckButton->callback(cb_antiAliaseCheckButton);
	m_antiAliaseCheckButton->value(m_antiAliaseInfo);

	// install anti aliasing degree slider
	m_antiAliasingDegreeSlider = new Fl_Value_Slider(100, 210, 180, 20, "Anti-Aliasing Degree");
	m_antiAliasingDegreeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_antiAliasingDegreeSlider->type(FL_HOR_NICE_SLIDER);//m_kdtreeLeafSizeSlider
	m_antiAliasingDegreeSlider->labelfont(FL_COURIER);
	m_antiAliasingDegreeSlider->labelsize(12);
	m_antiAliasingDegreeSlider->minimum(1);
	m_antiAliasingDegreeSlider->maximum(4);
	m_antiAliasingDegreeSlider->step(1);
	m_antiAliasingDegreeSlider->value(m_nAntiAliasingDegree);//m_nKdtreeLeafSize
	m_antiAliasingDegreeSlider->align(FL_ALIGN_RIGHT);
	m_antiAliasingDegreeSlider->callback(cb_antiAliasingDegreeSlides);//cb_kdtreeLeafSlides

	// set up cube map implementation checkbox
	m_cubeMapCheckButton = new Fl_Check_Button(10, 250, 80, 20, "CubeMap");
	m_cubeMapCheckButton->user_data((void*)(this));//m_antiAliaseCheckButton
	m_cubeMapCheckButton->callback(cb_cubeMapCheckButton);//cb_antiAliaseCheckButton
	m_cubeMapCheckButton->value(m_cubeMapInfo);//m_antiAliaseInfo


	m_mainWindow->callback(cb_exit2);
	m_mainWindow->when(FL_HIDE);
	m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, traceWindowLabel);
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);

	// debugging view
	m_debuggingWindow = new DebuggingWindow();

	

}

#endif
