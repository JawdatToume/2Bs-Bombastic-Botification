# Install script for directory: C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/BasicSc2Bot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2main.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2main.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2main.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/bin/SDL2main.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_assert.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_atomic.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_audio.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_bits.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_blendmode.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_clipboard.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_android.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_iphoneos.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_macosx.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_minimal.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_pandora.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_psp.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_windows.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_winrt.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_config_wiz.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_copying.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_cpuinfo.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_egl.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_endian.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_error.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_events.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_filesystem.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_gamecontroller.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_gesture.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_haptic.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_hints.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_joystick.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_keyboard.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_keycode.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_loadso.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_log.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_main.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_messagebox.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_mouse.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_mutex.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_name.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengl.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengl_glext.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles2.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles2_gl2.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles2_gl2ext.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles2_gl2platform.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_opengles2_khrplatform.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_pixels.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_platform.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_power.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_quit.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_rect.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_render.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_revision.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_rwops.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_scancode.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_shape.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_stdinc.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_surface.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_system.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_syswm.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_assert.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_common.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_compare.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_crc32.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_font.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_fuzzer.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_harness.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_images.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_log.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_md5.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_test_random.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_thread.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_timer.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_touch.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_types.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_version.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/SDL_video.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/begin_code.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/cpp-sc2/contrib/SDL-mirror/include/close_code.h"
    "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/contrib/SDL-mirror/include/SDL_config.h"
    )
endif()

