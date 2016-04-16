#include "ui/ozone/platform/raspberry/vsync_provider.h"
#include "base/time/time.h"

namespace ui {

RaspberryVSyncProvider::RaspberryVSyncProvider() {
}

RaspberryVSyncProvider::~RaspberryVSyncProvider() {}

void RaspberryVSyncProvider::GetVSyncParameters(const UpdateVSyncCallback& callback) {
  base::TimeTicks timebase = base::TimeTicks::Now();
  base::TimeDelta interval = base::TimeDelta::FromMilliseconds(40);

  callback.Run(timebase, interval);
}

}  // namespace ui
