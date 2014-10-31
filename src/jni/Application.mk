APP_PLATFORM := android-10
APP_ABI := all #This would be for production builds
#APP_ABI := armeabi

APP_STL := gnustl_static
NDK_TOOLCHAIN_VERSION := 4.8
APP_CPPFLAGS += -std=c++11