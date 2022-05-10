#include <boost/log/trivial.hpp>

#include "pi4renderer.hpp"

//
// *****************************************************************************************************************************************************
// *****************************************************************************************************************************************************
//


/*
 * Based on https://github.com/matusnovak/rpi-opengl-without-x
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <boost/log/trivial.hpp>


static const EGLint configAttribs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE };

static const EGLint contextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE };


// Get the EGL error back as a string. Useful for debugging.
static const char* eglGetErrorStr()
{
    switch (eglGetError())
    {
    case EGL_SUCCESS:
        return "The last function succeeded without error.";
    case EGL_NOT_INITIALIZED:
        return "EGL is not initialized, or could not be initialized, for the "
            "specified EGL display connection.";
    case EGL_BAD_ACCESS:
        return "EGL cannot access a requested resource (for example a context "
            "is bound in another thread).";
    case EGL_BAD_ALLOC:
        return "EGL failed to allocate resources for the requested operation.";
    case EGL_BAD_ATTRIBUTE:
        return "An unrecognized attribute or attribute value was passed in the "
            "attribute list.";
    case EGL_BAD_CONTEXT:
        return "An EGLContext argument does not name a valid EGL rendering "
            "context.";
    case EGL_BAD_CONFIG:
        return "An EGLConfig argument does not name a valid EGL frame buffer "
            "configuration.";
    case EGL_BAD_CURRENT_SURFACE:
        return "The current surface of the calling thread is a window, pixel "
            "buffer or pixmap that is no longer valid.";
    case EGL_BAD_DISPLAY:
        return "An EGLDisplay argument does not name a valid EGL display "
            "connection.";
    case EGL_BAD_SURFACE:
        return "An EGLSurface argument does not name a valid surface (window, "
            "pixel buffer or pixmap) configured for GL rendering.";
    case EGL_BAD_MATCH:
        return "Arguments are inconsistent (for example, a valid context "
            "requires buffers not supplied by a valid surface).";
    case EGL_BAD_PARAMETER:
        return "One or more argument values are invalid.";
    case EGL_BAD_NATIVE_PIXMAP:
        return "A NativePixmapType argument does not refer to a valid native "
            "pixmap.";
    case EGL_BAD_NATIVE_WINDOW:
        return "A NativeWindowType argument does not refer to a valid native "
            "window.";
    case EGL_CONTEXT_LOST:
        return "A power management event has occurred. The application must "
            "destroy all contexts and reinitialise OpenGL ES state and "
            "objects to continue rendering.";
    default:
        break;
    }
    return "Unknown error!";
}


bool Pi4Renderer::getDisplay(EGLDisplay* display)
{
    gbmDevice = gbm_create_device(drmHelper->getDRMDeviceFd());
    gbmSurface = gbm_surface_create(gbmDevice, drmHelper->getScreenSize().width, drmHelper->getScreenSize().height, GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    *display = eglGetDisplay(gbmDevice);
    return true;
}

static int matchConfigToVisual(EGLDisplay display, EGLint visualId, EGLConfig* configs, int count)
{
    EGLint id;
    for (int i = 0; i < count; ++i)
    {
        if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &id))
            continue;
        if (id == visualId)
            return i;
    }
    return -1;
}


void Pi4Renderer::gbmSwapBuffers(EGLDisplay* display, EGLSurface* surface)
{
    eglSwapBuffers(*display, *surface);

    struct gbm_bo* bo = gbm_surface_lock_front_buffer(gbmSurface);
    uint32_t handle = gbm_bo_get_handle(bo).u32;
    uint32_t pitch = gbm_bo_get_stride(bo);
    uint32_t fb;
    fb = drmHelper->addAndActivateFramebuffer(pitch, handle);

    if (previousBo)
    {
        drmHelper->removeFramebuffer(previousFb);
        gbm_surface_release_buffer(gbmSurface, previousBo);
    }
    previousBo = bo;
    previousFb = fb;
}

void Pi4Renderer::gbmSwapBuffers() {
    gbmSwapBuffers(&display, &surface);
}

void Pi4Renderer::gbmClean()
{
    // set the previous crtc
    drmHelper->setPreviousCrtc();

    if (previousBo)
    {
        drmHelper->removeFramebuffer(previousFb);
        gbm_surface_release_buffer(gbmSurface, previousBo);
    }

    gbm_surface_destroy(gbmSurface);
    gbm_device_destroy(gbmDevice);
}

bool Pi4Renderer::connectToDisplay(int displayNumber) {

    drmHelper = DRMHelper::getDRMForDisplay(displayNumber);

    if (getDisplay(&display)) {
        BOOST_LOG_TRIVIAL(info) << "[pi4renderer] got display connector via DRM device " << drmHelper->getDRMDeviceFilename();
        return true;
    } else 
    {
        BOOST_LOG_TRIVIAL(info) << "[pi4renderer] unable to get EGL display on DRM device " << drmHelper->getDRMDeviceFilename();
    }

    return false;
}

bool Pi4Renderer::startOpenGL(int width, int height)
{
    // We will use the screen resolution as the desired width and height for the viewport.
    int desiredWidth = width;
    if (desiredWidth == 0) {
        desiredWidth = drmHelper->getScreenSize().width;
    }

    int desiredHeight = height;
    if (desiredHeight == 0) {
        desiredHeight = drmHelper->getScreenSize().height;
    }

    // Other variables we will need further down the code.
    int major, minor;

    if (eglInitialize(display, &major, &minor) == EGL_FALSE)
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] failed to get EGL version! Error: " << eglGetErrorStr();
        eglTerminate(display);
        gbmClean();
        return false;
    }

    // Make sure that we can use OpenGL in this EGL app.
    eglBindAPI(EGL_OPENGL_API);

    BOOST_LOG_TRIVIAL(info) << "[pi4renderer] initialized EGL version: " << major << "." << minor;

    EGLint count;
    EGLint numConfigs;
    eglGetConfigs(display, NULL, 0, &count);
    EGLConfig* configs = (EGLConfig*)malloc(count * sizeof(configs));

    if (!eglChooseConfig(display, configAttribs, configs, count, &numConfigs))
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] failed to get EGL configs! Error: " << eglGetErrorStr();
        eglTerminate(display);
        gbmClean();
        return false;
    }

    // I am not exactly sure why the EGL config must match the GBM format.
    // But it works!
    int configIndex = matchConfigToVisual(display, GBM_FORMAT_XRGB8888, configs, numConfigs);
    if (configIndex < 0)
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] failed to find matching EGL config! Error: " << eglGetErrorStr();
        eglTerminate(display);
        gbm_surface_destroy(gbmSurface);
        gbm_device_destroy(gbmDevice);
        return false;
    }

    context =
        eglCreateContext(display, configs[configIndex], EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] failed to create EGL context! Error: " << eglGetErrorStr();
        eglTerminate(display);
        gbmClean();
        return false;
    }

    surface =
        eglCreateWindowSurface(display, configs[configIndex], gbmSurface, NULL);
    if (surface == EGL_NO_SURFACE)
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] failed to create EGL surface! Error: " << eglGetErrorStr();
        eglDestroyContext(display, context);
        eglTerminate(display);
        gbmClean();
        return false;
    }

    free(configs);
    eglMakeCurrent(display, surface, surface, context);

    // Set GL Viewport size, always needed!
    glViewport(0, 0, desiredWidth, desiredHeight);

    // Get GL Viewport size and test if it is correct.
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // viewport[2] and viewport[3] are viewport width and height respectively
    BOOST_LOG_TRIVIAL(debug) << "[pi4renderer] GL Viewport size: " << viewport[2] << "x" << viewport[3];

    if (viewport[2] != desiredWidth || viewport[3] != desiredHeight)
    {
        BOOST_LOG_TRIVIAL(error) << "[pi4renderer] error: glViewport returned incorrect values, something is wrong!";
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        gbmClean();
        return false;
    }

    return true;
}


void Pi4Renderer::stop_fullscreen_ogl() {

    // Cleanup
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    gbmClean();

    // DRM will be cleared by the DRM destructor
}

//
// *****************************************************************************************************************************************************
// *****************************************************************************************************************************************************
//

Pi4Renderer::Pi4Renderer() {
	shader_prefix = "pi4";
}

bool Pi4Renderer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer)
{
    // Only add Pi4 specific settings
    displayNumber = pt_renderer.get("display_number", 0);

    return OpenGLRenderer::configureFromPtree(pt_general, pt_renderer);
}

void Pi4Renderer::swapBuffers() {
	gbmSwapBuffers();
}

bool Pi4Renderer::initializeDisplay()
{
    
    if (!connectToDisplay(displayNumber)) {
        return false;
    }

	return startOpenGL(width, height);
}
