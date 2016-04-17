# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'internal_ozone_platform_deps': [
      'ozone_platform_raspberry',
    ],
    'internal_ozone_platforms': [
      'raspberry'
    ],
  },
  'targets': [
    {
      'target_name': 'ozone_platform_raspberry',
      'type': 'static_library',
      'defines': [
        'OZONE_IMPLEMENTATION',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../events/events.gyp:events',
        '../events/ozone/events_ozone.gyp:events_ozone_evdev',
        '../gfx/gfx.gyp:gfx',
      ],
      'sources': [
        'ozone_platform.cc',
        'ozone_platform.h',
        'surface_factory.cc',
        'surface_factory.h',
        'vsync_provider.cc',
        'vsync_provider.h'
      ],
      'link_settings': {
        'libraries': [
          '-lGLESv2 -lEGL -lbcm_host -lvcos -lvchiq_arm -lvchostif'
        ],
      },
    },
  ],
}
