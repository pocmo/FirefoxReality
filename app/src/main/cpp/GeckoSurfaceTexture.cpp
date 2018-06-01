#include <vrb/include/vrb/ConcreteClass.h>
#include "GeckoSurfaceTexture.h"

#include "vrb/ClassLoaderAndroid.h"
#include "vrb/Logger.h"

namespace {

static vrb::ClassLoaderAndroidPtr sClassLoader;
static JNIEnv* sEnv;
static jobject sActivity;
static jclass sGeckoSurfaceTextureClass;
static jmethodID sLookup;
static jmethodID sAttachToGLContext;
static jmethodID sUpdateTexImage;
static jmethodID sReleaseTexImage;

static const char* kClassName = "org/mozilla/gecko/gfx/GeckoSurfaceTexture";
static const char* kLookupName = "lookup";
static const char* kLookupSignature = "(I)Lorg/mozilla/gecko/gfx/GeckoSurfaceTexture;";
static const char* kAttachToGLContextName = "attachToGLContext";
static const char* kAttachToGLContextSignature = "(JI)V";
static const char* kUpdateTexImageName = "updateTexImage";
static const char* kUpdateTexImageSignature= "()V";
static const char* kReleaseTexImageName = "releaseTexImage";
static const char* kReleaseTexImageSignature = "()V";

static jmethodID
FindMethod(jclass aClass, const char* aName, const char* aSignature, const bool aIsStatic = false) {
  if (!sEnv) {
    VRB_LOG("Null JNIEnv, unable to find GeckoSurfaceTexture method: %s", aName);
    return nullptr;
  }
  if (!aClass) {
    VRB_LOG("Null java class. Unable to find GeckoSurfaceTexture method: %s", aName);
    return nullptr;
  }
  jmethodID result = nullptr;
  if (aIsStatic) {
    result = sEnv->GetStaticMethodID(aClass, aName, aSignature);
  } else {
    result = sEnv->GetMethodID(aClass, aName, aSignature);
  }
  if (sEnv->ExceptionCheck() == JNI_TRUE) {
    VRB_LOG("Failed to find GeckoSurfaceTexture method: %s %s", aName, aSignature);
    sEnv->ExceptionClear();
    return nullptr;
  }
  return result;

}

}

namespace crow {

struct GeckoSurfaceTexture::State {
  jobject surface;
  State()
      : surface(nullptr)
  {}
  ~State() {}
  void Shutdown() {
    if (surface && sEnv) {
      sEnv->DeleteGlobalRef(surface);
      surface = nullptr;
    }
  }
};

void
GeckoSurfaceTexture::InitializeJava(JNIEnv* aEnv, jobject aActivity) {
  if (aEnv == sEnv) {
    return;
  }
  sEnv = aEnv;
  if (!sEnv) {
    return;
  }
  sClassLoader = vrb::ClassLoaderAndroid::Create();
  sClassLoader->Init(aEnv, aActivity);
  sActivity = sEnv->NewGlobalRef(aActivity);
  jclass foundClass = sClassLoader->FindClass(kClassName);
  if (!foundClass) {
    return;
  }
  sGeckoSurfaceTextureClass = (jclass)sEnv->NewGlobalRef(foundClass);
  sEnv->DeleteLocalRef(foundClass);
  sLookup = FindMethod(sGeckoSurfaceTextureClass, kLookupName, kLookupSignature, /*aIsStatic*/ true);
  sAttachToGLContext = FindMethod(sGeckoSurfaceTextureClass, kAttachToGLContextName, kAttachToGLContextSignature);
  sUpdateTexImage = FindMethod(sGeckoSurfaceTextureClass, kUpdateTexImageName, kUpdateTexImageSignature);
  sReleaseTexImage = FindMethod(sGeckoSurfaceTextureClass, kReleaseTexImageName, kReleaseTexImageSignature);
}

void
GeckoSurfaceTexture::ShutdownJava() {
  if (sEnv) {
    if (sClassLoader) {
      sClassLoader->Shutdown();
      sClassLoader = nullptr;
    }
    if (sActivity) {
      sEnv->DeleteGlobalRef(sActivity);
      sActivity = nullptr;
    }
    if (sGeckoSurfaceTextureClass) {
      sEnv->DeleteGlobalRef(sGeckoSurfaceTextureClass);
      sGeckoSurfaceTextureClass = nullptr;
    }
    sLookup = nullptr;
    sAttachToGLContext = nullptr;
    sReleaseTexImage = nullptr;
    sUpdateTexImage = nullptr;
    sEnv = nullptr;
  }
}

GeckoSurfaceTexturePtr
GeckoSurfaceTexture::Create(const int32_t aHandle) {
  GeckoSurfaceTexturePtr result;
  if (!sEnv) {
    VRB_LOG("Unable to create GeckoSurfaceTexture. Java not initialized?");
    return result;
  }
  if (!sLookup) {
    VRB_LOG("GeckoSurfaceTexture.lookup method missing");
    return result;
  }
  jobject surface = sEnv->CallStaticObjectMethod(sGeckoSurfaceTextureClass, sLookup, aHandle);
  if (!surface) {
    VRB_LOG("Unable to find GeckoSurfaceTexture with handle: %d", aHandle);
    return result;
  }
  result = std::make_shared<vrb::ConcreteClass<GeckoSurfaceTexture, GeckoSurfaceTexture::State> >();
  result->m.surface = sEnv->NewGlobalRef(surface);
  return result;
}

void
GeckoSurfaceTexture::AttachToGLContext(EGLContext aContext, GLuint aTexture) {
  if (!Validate(sAttachToGLContext, __FUNCTION__)) {
    return;
  }
  sEnv->CallVoidMethod(m.surface, sAttachToGLContext, (jlong)aContext, (jint)aTexture);
  CheckException(__FUNCTION__);
}

void
GeckoSurfaceTexture::UpdateTexImage() {
  if (!Validate(sUpdateTexImage, __FUNCTION__)) {
    return;
  }
  sEnv->CallVoidMethod(m.surface, sUpdateTexImage);
  CheckException(__FUNCTION__);
}

void
GeckoSurfaceTexture::ReleaseTexImage() {
  if (!Validate(sReleaseTexImage, __FUNCTION__)) {
    return;
  }
  sEnv->CallVoidMethod(m.surface, sReleaseTexImage);
  CheckException(__FUNCTION__);
  m.Shutdown();
  CheckException("GeckoSurfaceTexture::State::Shutdown");

}


GeckoSurfaceTexture::GeckoSurfaceTexture(State& aState) : m(aState) {}
GeckoSurfaceTexture::~GeckoSurfaceTexture() {
  if (m.surface) {
    ReleaseTexImage();
  }
}

bool
GeckoSurfaceTexture::Validate(jmethodID aMethod, const char* aName) {
  if (!sEnv) {
    VRB_LOG("GeckoSurfaceTexture::%s failed. Java not initialized.", aName);
    return false;
  }
  if (!aMethod) {
    VRB_LOG("GeckoSurfaceTexture::%s failed. Java method is null", aName);
    return false;
  }
  if (!m.surface) {
    VRB_LOG("GeckoSurfaceTexture::%s failed. Java object is null", aName);
    return false;
  }
  return true;
}

void
GeckoSurfaceTexture::CheckException(const char* aName) {
  if (!sEnv) {
    return;
  }
  if (sEnv->ExceptionCheck() == JNI_TRUE) {
    sEnv->ExceptionClear();
    VRB_LOG("Java exception encountered when calling %s", aName);
  }
}


} // namespace crow