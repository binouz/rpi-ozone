#ifndef UI_OZONE_PLATFORM_RASPBERRY_SURFACE_FACTORY_H_
#define UI_OZONE_PLATFORM_RASPBERRY_SURFACE_FACTORY_H_

#include "base/memory/scoped_ptr.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace gfx {
  class SurfaceOzone;
}

namespace ui {

  class SurfaceFactoryRaspberry : public ui::SurfaceFactoryOzone {
    public:
      SurfaceFactoryRaspberry();
      virtual ~SurfaceFactoryRaspberry();

      // SurfaceFactoryOzone:
      virtual intptr_t GetNativeDisplay();
      virtual scoped_ptr<SurfaceOzoneEGL> CreateEGLSurfaceForWidget(
        gfx::AcceleratedWidget widget);
      virtual bool LoadEGLGLES2Bindings(
        AddGLLibraryCallback add_gl_library,
        SetGLGetProcAddressProcCallback setprocaddress);
  };

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_RASPBERRY_SURFACE_FACTORY_H_
