LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mikmod

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_CFLAGS := -g -O2 -pthread -finline-functions -funroll-loops -ffast-math -Wall -D_REENTRANT -Dunix -DHAVE_CONFIG_H

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := 	loaders/load_669.c \
					loaders/load_amf.c \
					loaders/load_dsm.c \
					loaders/load_far.c \
					loaders/load_gdm.c \
					loaders/load_imf.c \
					loaders/load_it.c \
					loaders/load_m15.c \
					loaders/load_med.c \
					loaders/load_mod.c \
					loaders/load_mtm.c \
					loaders/load_okt.c \
					loaders/load_s3m.c \
					loaders/load_stm.c \
					loaders/load_stx.c \
					loaders/load_ult.c \
					loaders/load_uni.c \
					loaders/load_xm.c \
					mmio/mmalloc.c \
					mmio/mmerror.c \
					mmio/mmio.c \
					playercode/mdreg.c \
					playercode/mdriver.c \
					playercode/mdulaw.c \
					playercode/mloader.c \
					playercode/mlreg.c \
					playercode/mlutil.c \
					playercode/mplayer.c \
					playercode/munitrk.c \
					playercode/mwav.c \
					playercode/npertab.c \
					playercode/sloader.c \
					playercode/virtch.c \
					playercode/virtch2.c \
					playercode/virtch_common.c \
					posix/memcmp.c \
					posix/strcasecmp.c \
					posix/strdup.c \
					posix/strstr.c \
					drivers/drv_nos.c \
					drivers/drv_osles.c \
					readers/reader_asset.c \
					mikmod-jni.c

LOCAL_SHARED_LIBRARIES := 
LOCAL_STATIC_LIBRARIES :=
# for dynamic linking
LOCAL_LDLIBS += -ldl
# for native audio
LOCAL_LDLIBS += -lOpenSLES
# for logging
LOCAL_LDLIBS += -llog
# for android assets
LOCAL_LDLIBS += -landroid

include $(BUILD_SHARED_LIBRARY)

