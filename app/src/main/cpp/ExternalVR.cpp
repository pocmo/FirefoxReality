#include "ExternalVR.h"

#include "vrb/ConcreteClass.h"
#include "vrb/Matrix.h"
#include "vrb/Quaternion.h"
#include "vrb/Vector.h"
#include "moz_external_vr.h"
#include <pthread.h>

namespace {

class Lock {
  pthread_mutex_t& mMutex;
  bool mLocked;
public:
  Lock(pthread_mutex_t& aMutex) : mMutex(aMutex), mLocked(false) {
    if (pthread_mutex_lock(&mMutex) == 0) {
      mLocked = true;
    }
  }

  ~Lock() {
    if (mLocked) {
      pthread_mutex_unlock(&mMutex);
    }
  }

  bool IsLocked() {
    return mLocked;
  }

private:
  Lock() = delete;
  VRB_NO_DEFAULTS(Lock)
  VRB_NO_NEW_DELETE
};

}

namespace crow {

struct ExternalVR::State {
  mozilla::gfx::VRExternalShmem data;
  mozilla::gfx::VRSystemState system;
  mozilla::gfx::VRBrowserState browser;
  device::CapabilityFlags deviceCapabilities;

  State() : deviceCapabilities(0) {
    memset(&data, 0, sizeof(mozilla::gfx::VRExternalShmem));
    memset(&system, 0, sizeof(mozilla::gfx::VRSystemState));
    memset(&browser, 0, sizeof(mozilla::gfx::VRBrowserState));
    data.version = mozilla::gfx::kVRExternalVersion;
    data.size = sizeof(mozilla::gfx::VRExternalShmem);
    pthread_mutex_init(&(data.systemMutex), nullptr);
    pthread_mutex_init(&(data.browserMutex), nullptr);
    system.displayState.mIsConnected = true;
    system.displayState.mIsMounted = true;
    system.enumerationCompleted = true;
    const vrb::Matrix identity = vrb::Matrix::Identity();
    memcpy(&(system.sensorState.leftViewMatrix), identity.Data(), sizeof(system.sensorState.leftViewMatrix));
    memcpy(&(system.sensorState.rightViewMatrix), identity.Data(), sizeof(system.sensorState.rightViewMatrix));
  }

  ~State() {
    pthread_mutex_destroy(&(data.systemMutex));
    pthread_mutex_destroy(&(data.browserMutex));
  }
};

ExternalVRPtr
ExternalVR::Create() {
  return std::make_shared<vrb::ConcreteClass<ExternalVR, ExternalVR::State> >();
}

mozilla::gfx::VRExternalShmem*
ExternalVR::GetSharedData() {
  return &(m.data);
}

void
ExternalVR::SetDeviceName(const std::string& aName) {
  if (aName.length() == 0) {
    return;
  }
  strncpy(m.system.displayState.mDisplayName, aName.c_str(),
          mozilla::gfx::kVRDisplayNameMaxLen - 1);
  m.system.displayState.mDisplayName[mozilla::gfx::kVRDisplayNameMaxLen - 1] = '\0';
}

void
ExternalVR::SetCapabilityFlags(const device::CapabilityFlags aFlags) {
  uint16_t result = 0;
  if (device::Position & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_Position);
  }
  if (device::Orientation & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_Orientation);
  }
  if (device::Present & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_Present);
  }
  if (device::AngularAcceleration & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_AngularAcceleration);
  }
  if (device::LinearAcceleration & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_LinearAcceleration);
  }
  if (device::StageParameters & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_StageParameters);
  }
  if (device::MountDetection & aFlags) {
    result |= static_cast<uint16_t>(mozilla::gfx::VRDisplayCapabilityFlags::Cap_MountDetection);
  }
  m.deviceCapabilities = aFlags;
  m.system.displayState.mCapabilityFlags = static_cast<mozilla::gfx::VRDisplayCapabilityFlags>(result);
  m.system.sensorState.flags = m.system.displayState.mCapabilityFlags;
}

void
ExternalVR::SetFieldOfView(const device::Eye aEye, const double aLeftDegrees,
                           const double aRightDegrees,
                           const double aTopDegrees,
                           const double aBottomDegrees) {
  mozilla::gfx::VRDisplayState::Eye which = (aEye == device::Eye::Right
                                             ? mozilla::gfx::VRDisplayState::Eye_Right
                                             : mozilla::gfx::VRDisplayState::Eye_Left);
  m.system.displayState.mEyeFOV[which].upDegrees = aTopDegrees;
  m.system.displayState.mEyeFOV[which].rightDegrees = aRightDegrees;
  m.system.displayState.mEyeFOV[which].downDegrees = aBottomDegrees;
  m.system.displayState.mEyeFOV[which].leftDegrees = aLeftDegrees;
}

void
ExternalVR::SetEyeOffset(const device::Eye aEye, const float aX, const float aY, const float aZ) {
  mozilla::gfx::VRDisplayState::Eye which = (aEye == device::Eye::Right
                                             ? mozilla::gfx::VRDisplayState::Eye_Right
                                             : mozilla::gfx::VRDisplayState::Eye_Left);
  m.system.displayState.mEyeTranslation[which].x = aX;
  m.system.displayState.mEyeTranslation[which].y = aY;
  m.system.displayState.mEyeTranslation[which].z = aZ;
}

void
ExternalVR::SetEyeResolution(const int32_t aWidth, const int32_t aHeight) {
  m.system.displayState.mEyeResolution.width = aWidth;
  m.system.displayState.mEyeResolution.height = aHeight;
}

void
ExternalVR::SetHeadTransform(const vrb::Matrix& aTransform) {
  m.system.sensorState.inputFrameID++;
  memcpy(&(m.system.sensorState.orientation), vrb::Quaternion(aTransform).Data(),
         sizeof(m.system.sensorState.orientation));
  memcpy(&(m.system.sensorState.position), aTransform.GetTranslation().Data(),
         sizeof(m.system.sensorState.position));
}

void
ExternalVR::PushSystemState() {
  Lock lock(m.data.systemMutex);
  if (lock.IsLocked()) {
    memcpy(&(m.data.state), &(m.system), sizeof(mozilla::gfx::VRSystemState));
  }
}

void
ExternalVR::PullBrowserState() {
  Lock lock(m.data.browserMutex);
  if (lock.IsLocked()) {
    memcpy(&(m.browser), &(m.data.browserState), sizeof(mozilla::gfx::VRBrowserState));
  }
}

bool
ExternalVR::IsPresenting() const {
  return false; // m.browser.isPresenting;
}

void
ExternalVR::StopPresenting() {
  m.system.displayState.mPresentingGeneration++;
}

ExternalVR::ExternalVR(State& aState) : m(aState) {
  PushSystemState();
}

ExternalVR::~ExternalVR() {}

}