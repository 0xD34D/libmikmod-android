#ifndef _MIKMOD_JNI_
#define _MIKMOD_JNI_

#define MM_CLASS "com/scheffsblend/mikmod/MikMod"

/**
 * OnMusicEnd callback
 */
#define MM_CLASS_OME_CB  "OnMusicEnd"
#define MM_CLASS_OME_SIG  "()V"

/**
 * OnMusicUpdate callback
 */
#define MM_CLASS_OMU_CB  "OnMusicUpdate"
#define MM_CLASS_OMU_SIG  "(FII)V"

/**
 * OnMusicLoaded callback
 */
#define MM_CLASS_OML_CB  "OnMusicLoaded"
#define MM_CLASS_OML_SIG  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)V"


void jni_onMusicEnd(void);
void jni_onMusicUpdate(ULONG time, UWORD sngpos, UWORD numpos);
void jni_onMusicLoaded(char* name, char* type, char* comment,
		UWORD bpm, UWORD tracks);

#endif
