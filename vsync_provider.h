#ifndef UI_OZONE_PLATFORM_RASPBERRY_VSYNC_PROVIDER_H_
#define UI_OZONE_PLATFORM_RASPBERRY_VSYNC_PROVIDER_H_

#include "base/memory/weak_ptr.h"
#include "ui/gfx/vsync_provider.h"

namespace ui {

class RaspberryVSyncProvider : public gfx::VSyncProvider {
 public:
  RaspberryVSyncProvider();
  virtual ~RaspberryVSyncProvider();

  virtual void GetVSyncParameters(const UpdateVSyncCallback& callback);

 private:

  DISALLOW_COPY_AND_ASSIGN(RaspberryVSyncProvider);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_RASPBERRY_VSYNC_PROVIDER_H_
