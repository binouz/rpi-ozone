#ifndef UI_OZONE_PLATFORM_RASPBERRY_OZONE_PLATFORM_H_
#define UI_OZONE_PLATFORM_RASPBERRY_OZONE_PLATFORM_H_

namespace ui {

class OzonePlatform;

// Constructor hook for use in ozone_platform_list.cc
OzonePlatform* CreateOzonePlatformRaspberry();

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_RASPBERRY_OZONE_PLATFORM_H_
