
APP_STL := stlport_static # required by Regal

ifeq ($(NDK_DEBUG),1)
  $(warning NDK_DEBUG set, enabling debug build.)
  APP_OPTIM := debug
endif
