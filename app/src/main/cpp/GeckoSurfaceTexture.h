#ifndef VRBROWSER_GECKOSURFACETEXTURE_H
#define VRBROWSER_GECKOSURFACETEXTURE_H

#include "vrb/gl.h"
#include "vrb/MacroUtils.h"

#include <EGL/egl.h>
#include <jni.h>
#include <memory>

namespace crow {

class GeckoSurfaceTexture;
typedef std::shared_ptr<GeckoSurfaceTexture> GeckoSurfaceTexturePtr;

class GeckoSurfaceTexture {
public:
  static void InitializeJava(JNIEnv* aEnv, jobject aActivity);
  static void ShutdownJava();
  static GeckoSurfaceTexturePtr Create(const int32_t aHandle);
  void AttachToGLContext(EGLContext aContext, GLuint aTexture);
  void UpdateTexImage();
  void ReleaseTexImage();

protected:
  struct State;
  GeckoSurfaceTexture(State& aState);
  ~GeckoSurfaceTexture();

private:
  State& m;
  GeckoSurfaceTexture() = delete;
  VRB_NO_DEFAULTS(GeckoSurfaceTexture)
  bool Validate(jmethodID aMethod, const char* aName);
  void CheckException(const char* aName);
};

}

#endif //VRBROWSER_GECKOSURFACETEXTURE_H
