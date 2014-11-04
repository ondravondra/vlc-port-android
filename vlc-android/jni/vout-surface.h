#ifndef VLC_ANDROID_JNI_VOUT_SURFACE_H_
#define VLC_ANDROID_JNI_VOUT_SURFACE_H_

typedef struct vout_android_surface_s {

	pthread_mutex_t vout_android_lock;
	pthread_cond_t vout_android_surf_attached;
	void *vout_android_surf;
	void *vout_android_gui;
	jobject vout_android_java_surf;
	jobject vout_android_subtitles_surf;
	bool vout_video_player_activity_created;

} vout_android_surface_t;

int createVoutAndroidInstance();
vout_android_surface_t* getVoutAndroidInstanceSurface(int instanceId);

#endif /* VLC_ANDROID_JNI_VOUT_SURFACE_H_ */
