#define LOG_TAG "GlTest"
#include <ccstone/gl/RenderEngine.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <ccstone/gl/Texture.h>
#include <ccstone/gl/Rect.h>
#include <ccstone/gl/vec2.h>
#include <ccstone/gl/Mesh.h>
#include <ccstone/gl/Transform.h>
#include <ccstone/gl/util.h>
#include "videobuffer.h"
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <sys/system_properties.h>
using namespace CCStone;
/*
IOMX_HAL_PIXEL_FORMAT_RGBA_8888          = 1,
IOMX_HAL_PIXEL_FORMAT_RGBX_8888          = 2,
IOMX_HAL_PIXEL_FORMAT_RGB_888            = 3,
IOMX_HAL_PIXEL_FORMAT_RGB_565            = 4,
IOMX_HAL_PIXEL_FORMAT_BGRA_8888          = 5,
*/

static bool FindGraphicsAPI(OmxGraphics_t &api)
{
#define GB_SYMBOL "IAndroid_CreateOmxGraphics"
    typedef int (*GInterface)(OmxGraphics_t *);
    int ret = false;

    char buffer[1024] = {0};
    __system_property_get("ro.build.version.sdk",buffer);
    int32_t sdk = atoi(buffer) + 1;

    ALOGD("Find sdk version: %d\n", sdk);
    const char *namestr = "libstone_omx_%d.so";

    GInterface gapi = NULL;
    void *libhand = NULL;

    while (--sdk > 10) {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, namestr, sdk);
        ALOGD("Try dlopen '%s'", buffer);
        void *hwd = dlopen(buffer, RTLD_NOW);
        if (hwd == NULL) {
            continue;
        }else {
            ALOGD("Try find symbol '%s' in library %s", GB_SYMBOL, buffer); 
            void * method = dlsym(hwd, GB_SYMBOL);
            if (method) {
                gapi = (GInterface) method;
                libhand = hwd;
                break;
            }
            ALOGD("%s", dlerror());
            dlclose(hwd);
        }
    }

    if (gapi) {
        gapi(&api);
        if (!(api.alloc && api.destory)) {
            dlclose(libhand);
            ALOGE("Error OmxGraphics method.");
            memset(&api, 0, sizeof(api));
        }else {
         ret = true;
        }
    }
    return ret;
}

static void checkEglError(const char* op, EGLBoolean returnVal = EGL_TRUE) {
    if (returnVal != EGL_TRUE) {
        fprintf(stderr, "%s() returned %d\n", op, returnVal);
    }   

    for (EGLint error = eglGetError(); error != EGL_SUCCESS; error
            = eglGetError()) {
        fprintf(stderr, "after %s() (0x%x)\n", op, 
                error);
    }   
}

struct Geometry {
    uint32_t w;
    uint32_t h;
    Rect crop;
    inline bool operator ==(const Geometry& rhs) const {
        return (w == rhs.w && h == rhs.h && crop == rhs.crop);
    }
    inline bool operator !=(const Geometry& rhs) const {
        return !operator ==(rhs);
    }
};


struct State {
    Geometry active;
    Geometry requested;
    uint32_t z;
    uint32_t layerStack;
    uint8_t alpha;
    uint8_t flags;
    uint8_t reserved[2];
    int32_t sequence; // changes when visible regions can change
    Transform transform;
    Region activeTransparentRegion;
    Region requestedTransparentRegion;
};

class Layer {
    private:
        RenderEngine *mRenderEngine;
        uint32_t mTextureName;
        Texture mTexture;
        Texture::Target mTextureTarget;
        State mCurrentState;
        State mDrawingState;
        int32_t mWidth;
        int32_t mHeight;
        graphics_handle *mNativeBuffer;
        OmxGraphics_t mApi;
    private:
        EGLImageKHR createImage();
        bool bindTextureImage();
    public:
        Layer(RenderEngine *engine, int w, int h);
        void createBuffer();
        bool draw();
};

Layer::Layer(RenderEngine *engine, int w, int h):
    mRenderEngine(engine),
    mTextureName(0),
    mTextureTarget(Texture::TEXTURE_EXTERNAL),
    mWidth(0),
    mHeight(0)
{
    mCurrentState.active.w = w;
    mCurrentState.active.h = h;
    mCurrentState.active.crop.makeInvalid();
    mCurrentState.z = 0;
    mCurrentState.alpha = 0xFF;
    mCurrentState.layerStack = 0;
    mCurrentState.flags = 0;
    mCurrentState.sequence = 0;
    mCurrentState.transform.set(0, 0);
    mCurrentState.requested = mCurrentState.active;

    mDrawingState = mCurrentState;

    engine->genTextures(1, &mTextureName);
    engine->checkErrors();
    mTexture.init(mTextureTarget, mTextureName);
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);
    checkEglError("eglGetCurrentDisplay");
    eglQuerySurface(eglGetCurrentDisplay(), surface, EGL_WIDTH, &mWidth);
    eglQuerySurface(eglGetCurrentDisplay(), surface, EGL_HEIGHT, &mHeight);
    checkEglError("eglGetCurrentDisplay");
    ALOGD("TextureName %u w %d h %d", mTextureName, mWidth, mHeight);

    // Init graphic buffer
   LOG_ALWAYS_FATAL_IF(!FindGraphicsAPI(mApi), "Not found avalid graphics interface.");

   mNativeBuffer = mApi.alloc(w, h, IOMX_HAL_PIXEL_FORMAT_YV12, IOMX_GRALLOC_USAGE_HW_TEXTURE);

   LOG_ALWAYS_FATAL_IF(!mNativeBuffer, "Alloc GraphicBuffer failed.");

   void *addr = NULL;

   mApi.lock(mNativeBuffer,IOMX_GRALLOC_USAGE_SW_WRITE_NEVER, &addr);
   LOG_ALWAYS_FATAL_IF(!addr, "Lock NativeBuffer failed.");

   char yuvname[1024] = {0};

   sprintf(yuvname,"%dx%d@yv12.yuv", w, h);
   ALOGD("Read yuv texture data from '%s'", yuvname);
   FILE *yuv = fopen(yuvname, "r");

   int size = 0;
   if (yuv) {
     size = w * h * 3 / 2;
     fread(addr, size, 1, yuv);
     fclose(yuv);
   }

   ALOGD("Read yuv data %d bytes", size);

   mApi.unlock(mNativeBuffer);

   ALOGD("GraphicBuffer w %d h %d stride %d pixfmt %d",(int) mApi.width(mNativeBuffer),
           (int)mApi.height(mNativeBuffer),(int) mApi.stride(mNativeBuffer),(int) mApi.pixelFormat(mNativeBuffer));
}

void Layer::createBuffer()
{
}

bool Layer::draw()
{
   float red = 125;
   float green = 0;
   float blue = 0;
   float alpha = 0;
   Transform tr;
   Rect win(mWidth, mHeight);
   tr.makeBounds(mWidth, mHeight);
   Mesh mesh(Mesh::TRIANGLE_FAN, 4, 2, 0);
   
   Mesh::VertexArray<vec2> position(mesh.getPositionArray<vec2>());
   position[0] = tr.transform(win.left,  win.top);
   position[1] = tr.transform(win.left,  win.bottom);
   position[2] = tr.transform(win.right, win.bottom);
   position[3] = tr.transform(win.right, win.top);

   ALOGD("VertexArray: 4");

   for (int i = 0; i < 4; i ++) {
    ALOGD("Pos[%d][%f,%f]",i, position[i][0], position[i][1]);
   }

   mRenderEngine->setupFillWithColor(red, green, blue, alpha);
   mRenderEngine->checkErrors();
   mRenderEngine->drawMesh(mesh);
   mRenderEngine->checkErrors();

   bindTextureImage();

   int buffer_w = mCurrentState.active.w;
   int buffer_h = mCurrentState.active.h;
   mTexture.setDimensions(buffer_w, buffer_h);
   mTexture.setFiltering(false);
   // setup transform matrix
   // mTexture.setMatrix()
   mRenderEngine->setupLayerTexturing(mTexture);

   {
       // Draw with OpenGL
       mRenderEngine->setupLayerBlending(false, false, 0);
       mRenderEngine->checkErrors();
       // Compute geometry to mesh 
       // .....
       mRenderEngine->drawMesh(mesh);
       mRenderEngine->checkErrors();
       mRenderEngine->disableTexturing();
       mRenderEngine->checkErrors();
   }

   ALOGD("Drawing done.");
   return true;
}

bool Layer::bindTextureImage()
{
   // Bind texture
   //glBindTexture(mTextureTarget, mTextureName);
   //mRenderEngine->checkErrors();

   // create EGLImageKHR from GraphicBuffer
   EGLImageKHR image = createImage();
   glEGLImageTargetTexture2DOES(mTextureTarget, (GLeglImageOES)image);
   mRenderEngine->checkErrors();
   return true;
}

EGLImageKHR Layer::createImage()
{
    void *winbuffer = mApi.winbuffer(mNativeBuffer);

    ALOGD("EGLClientBuffer %p", winbuffer);
    EGLClientBuffer clientBuffer = (EGLClientBuffer)winbuffer; 
    EGLImageKHR img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            clientBuffer, 0);
    checkEglError("eglCreateImageKHR");
    LOG_ALWAYS_FATAL_IF(img == EGL_NO_IMAGE_KHR, "Create EGLImageKHR failed.");
    return img;
}

void createEglSurface(RenderEngine *engine,EGLDisplay dsp ,int w, int h)
{
    EGLint attribs[] = { EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE, EGL_NONE };
    EGLSurface surface = eglCreatePbufferSurface(dsp, engine->getEGLConfig(), attribs);
    checkEglError("eglCreatePbufferSurface");
    if (surface == EGL_NO_SURFACE) {
     LOG_ALWAYS_FATAL_IF(surface == EGL_NO_SURFACE,"create EGLSurface failed.");
    }
    EGLBoolean success = eglMakeCurrent(dsp, surface, surface, engine->getEGLContext());
    checkEglError("eglMakeCurrent");
    LOG_ALWAYS_FATAL_IF(!success, "can't make current surface failed.");
}

int main(int argc, char **args)
{
    EGLDisplay mEGLDisplay;
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(mEGLDisplay, NULL, NULL);
    RenderEngine* engine = RenderEngine::create(mEGLDisplay, 1);
    ALOGI("RenderEngine %p config %u context %u\n", engine,(uint32_t) engine->getEGLConfig(),(uint32_t) engine->getEGLContext());
    
    createEglSurface(engine, mEGLDisplay, 1920, 1080);

    Layer *layer = new Layer(engine, 1920, 1080);
    layer->createBuffer();
    layer->draw();

    char *buffer = new char[1920 * 1080 * 4];
    memset(buffer, 0, 1920*1080*4);
    engine->readPixels(0,0,1920,1080, (uint32_t *)buffer);
    FILE *save = fopen("gl.rgba8888", "w");
    fwrite(buffer, 1920*1080*4, 1, save);
    fclose(save);
    delete layer;
   return 0;
}
