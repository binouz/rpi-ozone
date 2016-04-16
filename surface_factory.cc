#include "base/files/file_path.h"
#include "base/native_library.h"
#include "ui/ozone/platform/raspberry/surface_factory.h"
#include "ui/ozone/public/surface_ozone_egl.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#include "ui/gfx/vsync_provider.h"

#include <EGL/egl.h>
#include <EGL/fbdev_window.h>
#include <sys/ioctl.h>
#include <fcntl.h>

namespace ui {

  namespace {

    class SurfaceOzoneRaspberry : public SurfaceOzoneEGL {

      public:

        SurfaceOzoneRaspberry(gfx::AcceleratedWidget window_id) {
          DISPMANX_UPDATE_HANDLE_T dispman_update;
          VC_RECT_T dst_rect;
          VC_RECT_T src_rect;;

          graphics_get_display_size(0, &native_window_.width, &native_window_.height);

          dst_rect.x = 0;
          dst_rect.y = 0;
          dst_rect.width = native_window_.width;
          dst_rect.height = native_window_.height;

          src_rect.x = 0;
          src_rect.y = 0;
          src_rect.width = native_window_.width /*<< 16*/;
          src_rect.height = native_window_.height /*<< 16*/;

          dispman_display_ = vc_dispmanx_display_open(0);
          dispman_update = vc_dispmanx_update_start(0);

          native_window_.element = vc_dispmanx_element_add(
            dispman_update,
            dispman_display_,
            0,
            &dst_rect,
            0,
            &src_rect,
            DISPMANX_PROTECTION_NONE,
            0,
            0,
            0);

          vc_dispmanx_update_submit_sync(dispman_update);
        }

        virtual ~SurfaceOzoneRaspberry() {
          DISPMANX_UPDATE_HANDLE_T dispman_update  = vc_dispmanx_update_start(0);
          vc_dispmanx_element_remove(dispman_update, native_window_.element);
          vc_dispmanx_update_submit_sync(dispman_update);
          vc_dispmanx_display_close(dispman_display_);
        }

        virtual intptr_t GetNativeWindow()
        {
          return (intptr_t)(&native_window_);
        }

        virtual bool OnSwapBuffers()
        {
          return true;
        }

        virtual bool ResizeNativeWindow(const gfx::Size& viewport_size) {
          /* TODO */
          return true;
        }

        virtual void OnSwapBuffersAsync(const SwapCompletionCallback& callback) {
        }

      private:
        EGL_DISPMANX_WINDOW_T native_window_;
        DISPMANX_DISPLAY_HANDLE_T dispman_display_;
    };

  }  // namespace

  SurfaceFactoryRaspberry::SurfaceFactoryRaspberry()
  {
  }
  SurfaceFactoryRaspberry::~SurfaceFactoryRaspberry()
  {
  }

  intptr_t SurfaceFactoryRaspberry::GetNativeDisplay() {
    return (intptr_t)EGL_DEFAULT_DISPLAY;
  }

  scoped_ptr<SurfaceOzoneEGL>
  SurfaceFactoryRaspberry::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget widget) {
    return make_scoped_ptr<SurfaceOzoneEGL>(
      new SurfaceOzoneRaspberry(widget));
  }

  bool SurfaceFactoryRaspberry::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback setprocaddress) {
    base::NativeLibraryLoadError error;
    base::NativeLibrary gles_library = base::LoadNativeLibrary(
      base::FilePath("libGLESv2.so.2"), &error);

    if (!gles_library) {
      LOG(WARNING) << "Failed to load GLES library: " << error.ToString();
      return false;
    }

    base::NativeLibrary egl_library = base::LoadNativeLibrary(
      base::FilePath("libEGL.so.1"), &error);

    if (!egl_library) {
      LOG(WARNING) << "Failed to load EGL library: " << error.ToString();
      base::UnloadNativeLibrary(gles_library);
      return false;
    }

    GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
        base::GetFunctionPointerFromNativeLibrary(
          egl_library, "eglGetProcAddress"));

    if (!get_proc_address) {
      LOG(ERROR) << "eglGetProcAddress not found.";
      base::UnloadNativeLibrary(egl_library);
      base::UnloadNativeLibrary(gles_library);
      return false;
    }

    setprocaddress.Run(get_proc_address);
    add_gl_library.Run(egl_library);
    add_gl_library.Run(gles_library);
    return true;
  }

}  // namespace ui
