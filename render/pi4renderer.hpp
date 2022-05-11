#pragma once

#include <memory>

#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "openglrenderer.hpp"

#include "../drm/drmhelper.hpp"

class Pi4Renderer : public OpenGLRenderer
{
public:
	Pi4Renderer();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) override;

private:
	virtual bool initializeDisplay() override;
	virtual void swapBuffers() override;

	shared_ptr<DRMHelper> drmHelper;
	int displayNumber;

	// Low level graphics stuff
	struct gbm_device* gbmDevice;
	struct gbm_surface* gbmSurface;

	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;

	bool getDisplay(EGLDisplay* display);
	struct gbm_bo* previousBo = NULL;
	uint32_t previousFb;
	void gbmSwapBuffers(EGLDisplay* display, EGLSurface* surface);
	void gbmSwapBuffers();
	void gbmClean();
	bool connectToDisplay(int displayNumber);
	bool startOpenGL(int width = 0, int height = 0);
	void stop_fullscreen_ogl();

};
