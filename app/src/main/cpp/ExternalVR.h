#ifndef VRBROWSER_EXTERNALVR_H
#define VRBROWSER_EXTERNALVR_H

#include "vrb/MacroUtils.h"
#include "DeviceDelegate.h"
#include <memory>
#include <string>

namespace mozilla { namespace gfx { struct VRExternalShmem; } }

namespace crow {

class ExternalVR;
typedef std::shared_ptr<ExternalVR> ExternalVRPtr;

class ExternalVR : public ImmersiveDisplay {
public:

  static ExternalVRPtr Create();
  mozilla::gfx::VRExternalShmem* GetSharedData();
  // DeviceDisplay interface
  void SetDeviceName(const std::string& aName) override;
  void SetCapabilityFlags(const device::CapabilityFlags aFlags) override;
  void SetFieldOfView(const device::Eye aEye, const double aLeftDegrees,
                      const double aRightDegrees,
                      const double aTopDegrees,
                      const double aBottomDegrees) override;
  void SetEyeOffset(const device::Eye aEye, const float aX, const float aY, const float aZ) override;
  void SetEyeResolution(const int32_t aX, const int32_t aY) override;
  // ExternalVR interface
  void SetHeadTransform(const vrb::Matrix& aTransform);
  void PushSystemState();
  void PullBrowserState();
  bool IsPresenting() const;
  bool IsFrameReady() const;
  void StopPresenting();
protected:
  struct State;
  ExternalVR(State& aState);
  ~ExternalVR();
private:
  State& m;
  ExternalVR() = delete;
  VRB_NO_DEFAULTS(ExternalVR)
};

}

#endif //VRBROWSER_EXTERNALVR_H
