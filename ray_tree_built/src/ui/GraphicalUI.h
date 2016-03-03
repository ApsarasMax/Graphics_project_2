//
// GraphicalUI.h
//
// The header file for the graphical UI
//

#ifndef __GraphicalUI_h__
#define __GraphicalUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>

#include "TraceUI.h"
#include "TraceGLWindow.h"
#include "debuggingWindow.h"
#include "CubeMapChooser.h"

class ModelerView;

class GraphicalUI : public TraceUI {
public:
	GraphicalUI();

	int run();

	void alert( const string& msg );

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;
	Fl_Slider*			m_thresholdSlider;
	Fl_Slider*			m_blockSlider;
	Fl_Slider*			m_aaSamplesSlider;
	Fl_Slider*			m_aaThreshSlider;
	Fl_Slider*			m_refreshSlider;
	Fl_Slider*			m_treeDepthSlider;
	Fl_Slider*			m_leafSizeSlider;
	Fl_Slider*			m_filterSlider;
	

	Fl_Check_Button*	m_debuggingDisplayCheckButton;
	Fl_Check_Button*	m_aaCheckButton;
	Fl_Check_Button*	m_kdCheckButton;
	Fl_Check_Button*	m_cubeMapCheckButton;
	Fl_Check_Button*	m_ssCheckButton;
	Fl_Check_Button*	m_shCheckButton;
	Fl_Check_Button*	m_bfCheckButton;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	CubeMapChooser*     m_cubeMapChooser;

	TraceGLWindow*		m_traceGlWindow;

	DebuggingWindow*	m_debuggingWindow;

	CubeMapChooser* pMap;

	//kdtree
	Fl_Check_Button*	m_kdtreeCheckButton;
	static int	m_nKdtreeMaxDepth; //zyc
	static int	m_nKdtreeLeafSize;
	static Fl_Slider*			m_kdtreeMaxDepthSlider;
	static Fl_Slider*			m_kdtreeLeafSizeSlider;
	static bool m_kdtreeInfo;

	//anti-aliasing
	Fl_Check_Button*	m_antiAliaseCheckButton;
	static int	m_nAntiAliasingDegree;
	static Fl_Slider*			m_antiAliasingDegreeSlider;
	static bool m_antiAliaseInfo;

	//cube map
	static bool m_cubeMapInfo;

	//multithread
	Fl_Check_Button*	m_multiThreadCheckButton;
	static bool m_multiThreadInfo;
	static Fl_Slider*			m_multiThreadSlider;
	static int	m_nMultiThread; //zyc
	

	// member functions
	void setRayTracer(RayTracer *tracer);
	RayTracer* getRayTracer() { return raytracer; }

	static void stopTracing();

	// static vars
	static char *traceWindowLabel;
	
private:

	clock_t refreshInterval;

	// static class members
	static Fl_Menu_Item menuitems[];

	static GraphicalUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_load_cubeMap(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);
	static void cb_exit3(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
	static void cb_refreshSlides(Fl_Widget* o, void* v);

	static void show_picture(const char *old_label, int width_start, int width, int height_start, int height);
	//static void *show_picture(void *threadarg);
	static void *PrintHello(void *threadarg);
	static void antiAliasing(int height_start, int height_end);


	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
	static void cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v);
	static void cb_ssCheckButton(Fl_Widget* o, void* v);
	static void cb_shCheckButton(Fl_Widget* o, void* v);
	static void cb_bfCheckButton(Fl_Widget* o, void* v);

	//kdtree
	static void cb_kdtreeCheckButton(Fl_Widget* o, void* v);
	static void cb_kdtreeMaxSlides(Fl_Widget* o, void* v);
	static void cb_kdtreeLeafSlides(Fl_Widget* o, void* v);

	//anti-aliasing
	static void cb_antiAliaseCheckButton(Fl_Widget* o, void* v);
	static void cb_antiAliasingDegreeSlides(Fl_Widget* o, void* v);

	//multithread
	static void cb_multiThreadCheckButton(Fl_Widget* o, void* v);
	
	//cubeMap
	static void cb_cubeMapCheckButton(Fl_Widget* o, void* v);
	static void cb_filterSlides(Fl_Widget* o, void* v);
	
	static void cb_multiThreadSlides(Fl_Widget* o, void* v);
			
	static bool stopTrace;
	static bool doneTrace;
	static GraphicalUI* pUI;

};

#endif
