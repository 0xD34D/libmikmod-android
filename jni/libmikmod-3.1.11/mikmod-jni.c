/**
 * @file
 * @author  Clark Scheff <clark@scheffsblend.com>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * JNI glue for using libmikmod with android
 */

#include <jni.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "mikmod.h"
#include "logging.h"
#include "mikmod-jni.h"

/**
 * Used for image update
 */
jclass jNativesCls;
jmethodID jOnMusicEnd;
jmethodID jOnMusicUpdate;
jmethodID jOnMusicLoaded;
static JavaVM *g_VM;

MODULE *module;
static BOOL requestStop = 0;

/**
 * used for loading files from assets
 */
AAsset* asset;
extern MREADER reader_asset;


/**
 * Function first called when the library is loaded
 */
jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	g_VM = vm;

	(*g_VM)->GetEnv(vm, &env, JNI_VERSION_1_4);

	jNativesCls = (*env)->NewGlobalRef(env, (*env)->FindClass(env, MM_CLASS));
	if (!jNativesCls) {
		LOGE("Unable to find native java class!");
		return JNI_ERR;
	}

	jOnMusicEnd = (*env)->GetStaticMethodID(env, jNativesCls, MM_CLASS_OME_CB, MM_CLASS_OME_SIG);
	if (!jOnMusicEnd) {
		LOGE("Unable to get %s method ID", MM_CLASS_OME_CB);
		return JNI_ERR;
	}

	jOnMusicUpdate = (*env)->GetStaticMethodID(env, jNativesCls, MM_CLASS_OMU_CB, MM_CLASS_OMU_SIG);
	if (!jOnMusicEnd) {
		LOGE("Unable to get %s method ID", MM_CLASS_OMU_CB);
		return JNI_ERR;
	}

	jOnMusicLoaded = (*env)->GetStaticMethodID(env, jNativesCls, MM_CLASS_OML_CB, MM_CLASS_OML_SIG);
	if (!jOnMusicLoaded) {
		LOGE("Unable to get %s method ID", MM_CLASS_OML_CB);
		return JNI_ERR;
	}

	LOGI("Library libmikmod %d.%d.%d loaded...", LIBMIKMOD_VERSION_MAJOR,
			LIBMIKMOD_VERSION_MINOR, LIBMIKMOD_REVISION);

	return JNI_VERSION_1_4;
}

/**
 * Initializes and sets up libmikmod to be used and
 * sets up the OpenSL ES sound system for playback
 */
jboolean Java_com_scheffsblend_mikmod_MikMod_initMikMod(JNIEnv* env, jclass clazz)
{
	/* register the OpenSL ES driver */
	MikMod_RegisterDriver(&drv_osles);

	/* register all the module loaders */
	MikMod_RegisterAllLoaders();

	/* initialize the library */
	md_mode |= DMODE_SOFT_MUSIC;
	if (MikMod_Init(""))
	{
		LOGE("Could not initialize sound, reason: %s\n",
				MikMod_strerror(MikMod_errno));
		return 0;
	}

	return 1;
}

/**
 * Thread for playing back a mod loaded in memory.
 */
void *play_mod( void *ptr )
{
	Player_Start(module);
	Player_SetPosition(0);
	LOGI("Playing %s", module->songname);

	while (!requestStop && Player_Active())
	{
		/* we're playing!!! */
		MikMod_Update();
		jni_onMusicUpdate(module->sngtime, module->sngpos, module->numpos);
		usleep(10000);
	}
	// call back to the java side to let them know the music is done
	jni_onMusicEnd();

	Player_Stop();
}

/**
 * Starts playback of mod loaded in memory.
 */
void Java_com_scheffsblend_mikmod_MikMod_playMod(JNIEnv* env, jclass clazz)
{
    if (module)
    {
    	pthread_t playThread;
    	pthread_create( &playThread, NULL, play_mod, NULL);
    	requestStop = 0;
    }
    else
    {
    	LOGW("No module loaded!");
    }
}

/**
 * Loads the specified mod into memory.
 * @param mod - mod to load including the path
 * @return true if module was loaded, otherwise false
 */
jboolean Java_com_scheffsblend_mikmod_MikMod_loadMod(JNIEnv* env, jclass clazz,
		jstring mod)
{
    const char *utf8 = (*env)->GetStringUTFChars(env, mod, 0);
    assert(NULL != utf8);

    module = Player_Load(utf8, 128, 0);
    (*env)->ReleaseStringUTFChars(env, mod, utf8);
    if (!module)
    {
    	LOGE("Could not load module, reason: %s\n",
    			MikMod_strerror(MikMod_errno));
    	return 0;
    }

    return 1;
}

/**
 * Loads the specified mod into memory.
 * @param mod - mod to load including the path
 * @return true if module was loaded, otherwise false
 */
jboolean Java_com_scheffsblend_mikmod_MikMod_loadModFromAsset(JNIEnv* env, jclass clazz,
		jobject assetManager, jstring filename)
{
    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr)
    	return 0;

    // convert Java string to UTF-8
	const char *utf8 = (*env)->GetStringUTFChars(env, filename, 0);
    if(NULL == utf8)
    	return 0;

    asset = AAssetManager_open(mgr, (const char*)utf8, AASSET_MODE_RANDOM);
    (*env)->ReleaseStringUTFChars(env, filename, utf8);
    if (NULL == asset)
    	return 0;
    module = Player_LoadGeneric(&reader_asset, 128, 0);
    AAsset_close(asset);
    if (!module)
    {
    	LOGE("Could not load module, reason: %s\n",
    			MikMod_strerror(MikMod_errno));
    	return 0;
    }

    return 1;
}

/**
 * Returns an array of strings with the following information, from
 * the module currently loaded in memory, in the order given below
 * 		Song title
 * 		Type of tracker file
 * 		Comments embedded in file
 * 		Beats per minute
 * 		Number of positions/tracks
 */
jobjectArray Java_com_scheffsblend_mikmod_MikMod_getModInfo(JNIEnv* env, jclass clazz)
{
	char tmp[256];
	jobjectArray ret;
	int i = 0;

	ret = (jobjectArray)(*env)->NewObjectArray(env, 5,
			(*env)->FindClass(env, "java/lang/String"),
			(*env)->NewStringUTF(env, ""));

	if (!module)
		return ret;

	(*env)->SetObjectArrayElement(env, ret,
			i++, (*env)->NewStringUTF(env, module->songname));
	(*env)->SetObjectArrayElement(env, ret,
			i++, (*env)->NewStringUTF(env, module->modtype));
	(*env)->SetObjectArrayElement(env, ret,
			i++, (*env)->NewStringUTF(env, module->comment));

	sprintf(tmp, "%d", module->bpm);
	(*env)->SetObjectArrayElement(env, ret,
			i++, (*env)->NewStringUTF(env, tmp));

	sprintf(tmp, "%d", module->numpos);
	(*env)->SetObjectArrayElement(env, ret,
			i++, (*env)->NewStringUTF(env, tmp));

	return ret;
}

/**
 * This should be called when you are finished using the library so
 * that all necessary clean up and releasing of memory can be done
 */
void Java_com_scheffsblend_mikmod_MikMod_shutdownMikMod(JNIEnv* env, jclass clazz)
{
	if (module)
		Player_Free(module);
	MikMod_Exit();
}

/**
 * Lets the playback thread know to stop playing and exit.
 */
void Java_com_scheffsblend_mikmod_MikMod_stopMod(JNIEnv* env, jclass clazz)
{
	requestStop = 1;
}

/**
 * Returns true if playback is paused, false otherwise
 */
jboolean Java_com_scheffsblend_mikmod_MikMod_isPaused(JNIEnv* env, jclass clazz)
{
	if (!Player_Active())
		return 0;

	return Player_Paused();
}

/**
 * Pauses playback of current module being played.
 */
void Java_com_scheffsblend_mikmod_MikMod_pauseMod(JNIEnv* env, jclass clazz)
{
	if (!Player_Paused())
		Player_TogglePause();
}

/**
 * Resumes playback of currently paused module
 */
void Java_com_scheffsblend_mikmod_MikMod_resumeMod(JNIEnv* env, jclass clazz)
{
	if (Player_Paused())
		Player_TogglePause();
}

/**
 * Skip to previous track
 */
void Java_com_scheffsblend_mikmod_MikMod_previousTrack(JNIEnv* env, jclass clazz)
{
	if (!module)
		return;
	if (Player_Active() && (module->sngpos > 0))
		Player_PrevPosition();
}

/**
 * Skip to next track
 */
void Java_com_scheffsblend_mikmod_MikMod_nextTrack(JNIEnv* env, jclass clazz)
{
	if (!module)
		return;
	if (Player_Active() && (module->sngpos < module->numpos))
		Player_NextPosition();
}

/**
 * Skip to track specified by pos
 * @param pos - track to skip to
 */
void Java_com_scheffsblend_mikmod_MikMod_setTrack(JNIEnv* env, jclass clazz, jint pos)
{
	if (!module)
		return;
	if (Player_Active() && (pos >=0 && pos <= module->numpos))
		Player_SetPosition((UWORD)pos);
}

/**
 * Sets mod track back to zero
 */
void Java_com_scheffsblend_mikmod_MikMod_restartMod(JNIEnv* env, jclass clazz)
{
	if (!module)
		return;
	if (Player_Active())
		Player_SetPosition(0);
}

/**
 * Returns true if the player is currently active.  This means a module is
 * loaded and either being played or is paused.
 */
jboolean Java_com_scheffsblend_mikmod_MikMod_isActive(JNIEnv* env, jclass clazz)
{
	return Player_Active();
}


/**
 * Call back to java letting us know that the music has finished playing.
 */
void jni_onMusicEnd(void)
{
	JNIEnv* env;
	int status;

	status = (*g_VM)->AttachCurrentThread(g_VM, (void **) &env, NULL);
	if(status < 0) {
		LOGE("Unable to attach current thread.");
		return;
	}

    (*env)->CallStaticVoidMethod(env, jNativesCls, jOnMusicEnd);

    (*g_VM)->DetachCurrentThread(g_VM);
}

/**
 * Call back to java to provide current playback information about song
 * currently being played.
 */
void jni_onMusicUpdate(ULONG time, UWORD sngpos, UWORD numpos)
{
	float t = (float)(time) * 0.000976562f;
	JNIEnv* env;
	int status;

	status = (*g_VM)->AttachCurrentThread(g_VM, (void **) &env, NULL);
	if(status < 0) {
		LOGE("Unable to attach current thread.");
		return;
	}

    (*env)->CallStaticVoidMethod(env, jNativesCls, jOnMusicUpdate,
    		t, sngpos, numpos);

    (*g_VM)->DetachCurrentThread(g_VM);
}

void jni_onMusicLoaded(char* name, char* type, char* comment,
		UWORD bpm, UWORD tracks)
{
	JNIEnv* env;

	(*g_VM)->GetEnv(g_VM, &env, NULL);
	(*env)->CallStaticVoidMethod(env, jNativesCls, jOnMusicLoaded,
			(*env)->NewStringUTF(env, name),
			(*env)->NewStringUTF(env, type),
			(*env)->NewStringUTF(env, comment),
			bpm,
			tracks);
}
