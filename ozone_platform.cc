#include "ui/ozone/platform/raspberry/ozone_platform.h"
#include "ui/ozone/platform/raspberry/surface_factory.h"

#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_ime_controller.h"

#include "ui/ozone/public/cursor_factory_ozone.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/gpu_platform_support.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/ozone/common/native_display_delegate_ozone.h"
#include "ui/ozone/common/stub_overlay_manager.h"
#include "ui/ozone/common/stub_client_native_pixmap_factory.h"

#include "ui/events/ozone/events_ozone.h"
#include "ui/events/ozone/device/device_manager.h"
#include "ui/events/ozone/evdev/event_factory_evdev.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"
#include "ui/events/ozone/layout/stub/stub_keyboard_layout_engine.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/events/ozone/evdev/cursor_delegate_evdev.h"
#include "ui/base/cursor/ozone/bitmap_cursor_factory_ozone.h"
#include "ui/views/widget/desktop_aura/desktop_factory_ozone.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host.h"
#include "ui/views/widget/native_widget_delegate.h"

#include "ui/aura/test/test_screen.h"

#include "base/strings/string16.h"

#include <iostream>

namespace ui {

  namespace {

    class RaspberryDesktopFactoryOzone :
        public views::DesktopFactoryOzone {

      public:
        RaspberryDesktopFactoryOzone() {
          views:: DesktopFactoryOzone::SetInstance(this);
        }

        ~RaspberryDesktopFactoryOzone() {
          views:: DesktopFactoryOzone::SetInstance(NULL);
        }

        views::DesktopWindowTreeHost* CreateWindowTreeHost(
          views::internal::NativeWidgetDelegate* native_widget_delegate,
          views::DesktopNativeWidgetAura* desktop_native_widget_aura) override {
          return views::DesktopWindowTreeHost::Create(native_widget_delegate, desktop_native_widget_aura);
        }

        gfx::Screen* CreateDesktopScreen() override {
          aura::TestScreen* screen = aura::TestScreen::Create(gfx::Size());
          gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, screen);
          return screen;
        }
    };

    class RaspberryImeController :
        public PlatformImeController {

      public:
        RaspberryImeController() {}
        ~RaspberryImeController() {}

        void UpdateTextInputState(const TextInputState&) {
          // Nothing to do here
        }

        void SetImeVisibility(bool visible) {
          // Nothing to do here
        }
    };

    class RaspberryWindow :
        public PlatformWindow,
        public PlatformEventDispatcher {
      public:
        static int current_id_;

        static gfx::AcceleratedWidget GetWindowId() {
          current_id_++;
          return (gfx::AcceleratedWidget)current_id_;
        };

        RaspberryWindow(PlatformWindowDelegate* delegate,
                       EventFactoryEvdev* event_factory,
                       const gfx::Rect& bounds)
          : delegate_(delegate),
            event_factory_(event_factory),
            bounds_(bounds),
            window_id_(RaspberryWindow::GetWindowId())
        {
          delegate_->OnAcceleratedWidgetAvailable(window_id_, 1.f);
          ui::PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
          ime_controller_.reset(new RaspberryImeController());
        };

        ~RaspberryWindow() {};

        // PlatformWindow:
        gfx::Rect GetBounds() {
          return bounds_;
        };

        void SetBounds(const gfx::Rect& bounds) {
          bounds_ = bounds;
          delegate_->OnBoundsChanged(bounds);
        };

        void Show() {}
        void Hide() {};
        void Close() {};
        void SetCapture() {};
        void ReleaseCapture() {};
        void ToggleFullscreen() {};
        void Maximize() {};
        void Minimize() {};
        void Restore() {};
        void SetCursor(PlatformCursor cursor) {};

        void MoveCursorTo(const gfx::Point& location) {
          event_factory_->WarpCursorTo(window_id_, gfx::PointF(location));
        };

        void ConfineCursorToBounds(const gfx::Rect& bounds) {};

        // PlatformEventDispatcher:
        bool CanDispatchEvent(const PlatformEvent& event) {
          return true;
        };

        uint32_t DispatchEvent(const PlatformEvent& native_event) {
          DispatchEventFromNativeUiEvent(
            native_event, base::Bind(&PlatformWindowDelegate::DispatchEvent,
                                     base::Unretained(delegate_)));

          return ui::POST_DISPATCH_STOP_PROPAGATION;
        }

        void SetTitle(const base::string16&) {
          // Nothing to do here
        }

        ui::PlatformImeController* GetPlatformImeController() {
          return ime_controller_.get();
        }

      private:
        PlatformWindowDelegate* delegate_;
        EventFactoryEvdev* event_factory_;
        gfx::Rect bounds_;
        gfx::AcceleratedWidget window_id_;
        scoped_ptr<RaspberryImeController> ime_controller_;

        DISALLOW_COPY_AND_ASSIGN(RaspberryWindow);
    };

    class OzoneCursorDelegateRaspberry : public ui::CursorDelegateEvdev {
      public:
        OzoneCursorDelegateRaspberry() : x_(0), y_(0) {}
        virtual ~OzoneCursorDelegateRaspberry() {}

        // Move the cursor.
        virtual void MoveCursor(const gfx::Vector2dF& delta) {
          x_ += delta.x();
          y_ += delta.y();
        }

        virtual void MoveCursorTo(gfx::AcceleratedWidget widget,
                                  const gfx::PointF& location) {
          x_ = location.x();
          y_ = location.y();
        }

        virtual void MoveCursorTo(const gfx::PointF& location){
          x_ = location.x();
          y_ = location.y();
        }

        // Location in screen.
        virtual gfx::PointF GetLocation() {
          return gfx::PointF(x_, y_);
        }

        // Cursor visibility.
        virtual bool IsCursorVisible() {
          return false;
        }

        // The bounds that the cursor is confined to.
        virtual gfx::Rect GetCursorConfinedBounds() {
          return gfx::Rect(0, 0, 1240, 720);
        }

      private:

        float x_;
        float y_;

    };

    int RaspberryWindow::current_id_ = 0;

    class OzonePlatformRaspberry : public OzonePlatform {
      public:
        OzonePlatformRaspberry()
          : cursor_delegate_(new OzoneCursorDelegateRaspberry()) {
        }

        virtual ~OzonePlatformRaspberry() {
          delete cursor_delegate_;
        }

        // OzonePlatform:
        virtual ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() {
          return surface_factory_ozone_.get();
        }

        virtual CursorFactoryOzone* GetCursorFactoryOzone() {
          return cursor_factory_ozone_.get();
        }

        virtual GpuPlatformSupport* GetGpuPlatformSupport() {
          return gpu_platform_support_.get();
        }
        virtual GpuPlatformSupportHost* GetGpuPlatformSupportHost() {
          return gpu_platform_support_host_.get();
        }

        virtual scoped_ptr<ui::NativeDisplayDelegate> CreateNativeDisplayDelegate() {
          return scoped_ptr<ui::NativeDisplayDelegate>(new NativeDisplayDelegateOzone());
        }

        virtual void InitializeUI() {
          device_manager_ = CreateDeviceManager();
          KeyboardLayoutEngineManager::SetKeyboardLayoutEngine(
            make_scoped_ptr(new StubKeyboardLayoutEngine()));
          event_factory_ozone_.reset(
            new EventFactoryEvdev(/*cursor_delegate_*/NULL, device_manager_.get(), KeyboardLayoutEngineManager::GetKeyboardLayoutEngine()));
          surface_factory_ozone_.reset(new SurfaceFactoryRaspberry());
          cursor_factory_ozone_.reset(new CursorFactoryOzone());
          gpu_platform_support_host_.reset(CreateStubGpuPlatformSupportHost());
          overlay_manager_.reset(new StubOverlayManager());
        }

        virtual void InitializeGPU() {
          if(!surface_factory_ozone_)
            surface_factory_ozone_.reset(new SurfaceFactoryRaspberry());
          gpu_platform_support_.reset(CreateStubGpuPlatformSupport());
        }

        virtual ui::InputController* GetInputController() {
          return event_factory_ozone_->input_controller();
        }

        virtual scoped_ptr<SystemInputInjector> CreateSystemInputInjector() {
          return event_factory_ozone_->CreateSystemInputInjector();
        }

        virtual scoped_ptr<PlatformWindow> CreatePlatformWindow(
          PlatformWindowDelegate* delegate,
          const gfx::Rect& bounds) {
          return make_scoped_ptr<PlatformWindow>(new RaspberryWindow(delegate, event_factory_ozone_.get(), bounds));
        }

        ui::OverlayManagerOzone* GetOverlayManager() {
          return overlay_manager_.get();
        }

        base::ScopedFD OpenClientNativePixmapDevice() const {
          return base::ScopedFD();
        }

      private:

        scoped_ptr<DeviceManager> device_manager_;
        scoped_ptr<EventFactoryEvdev> event_factory_ozone_;
        scoped_ptr<SurfaceFactoryRaspberry> surface_factory_ozone_;
        scoped_ptr<CursorFactoryOzone> cursor_factory_ozone_;
        scoped_ptr<GpuPlatformSupport> gpu_platform_support_;
        scoped_ptr<GpuPlatformSupportHost> gpu_platform_support_host_;
        scoped_ptr<ui::OverlayManagerOzone> overlay_manager_;
        ui::CursorDelegateEvdev* cursor_delegate_;
        RaspberryDesktopFactoryOzone desktop_factory_;

        DISALLOW_COPY_AND_ASSIGN(OzonePlatformRaspberry);
    };

  }  // namespace

  OzonePlatform* CreateOzonePlatformRaspberry() {
    OzonePlatformRaspberry* platform = new OzonePlatformRaspberry;
    return platform;
  }

  ClientNativePixmapFactory* CreateClientNativePixmapFactoryRaspberry() {
    return CreateStubClientNativePixmapFactory();
  }

}  // namespace ui
