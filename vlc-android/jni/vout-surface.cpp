/*#include <vlc/vlc.h>
#include <vlc_common.h>
*/

#include <pthread.h>
#include <semaphore.h>
#include <tr1/unordered_map>

#include <jni.h>

typedef struct vout_android_surface_s {

	pthread_mutex_t vout_android_lock;
	pthread_cond_t vout_android_surf_attached;
	void *vout_android_surf;
	void *vout_android_gui;
	jobject vout_android_java_surf;
	jobject vout_android_subtitles_surf;
	bool vout_video_player_activity_created;

} vout_android_surface_t;

std::tr1::unordered_map<int, vout_android_surface_t*> vout_android_surface_map;
typedef std::tr1::unordered_map<int, vout_android_surface_t*>::const_iterator vout_android_surface_map_item;
sem_t vout_android_surface_map_lock = {};
#define MAP_LOCK_MAX 32

static vout_android_surface_t* getInstanceSurface(int instanceId) {
	sem_wait(&vout_android_surface_map_lock);
	vout_android_surface_map_item item = vout_android_surface_map.find(instanceId);
	if (item == vout_android_surface_map.end()) {
		return NULL;
	}

	return item->second;
}

void *jni_LockAndGetSubtitlesSurface(int instanceId) {
	vout_android_surface_t *surf = getInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

    pthread_mutex_lock(&surf->vout_android_lock);
    while (surf->vout_android_subtitles_surf == NULL)
        pthread_cond_wait(&surf->vout_android_surf_attached, &surf->vout_android_lock);
    return surf->vout_android_subtitles_surf;
}

void *jni_LockAndGetAndroidSurface(int instanceId) {
	vout_android_surface_t *surf = getInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

    pthread_mutex_lock(&surf->vout_android_lock);
    while (surf->vout_android_surf == NULL)
        pthread_cond_wait(&surf->vout_android_surf_attached, &surf->vout_android_lock);
    return surf->vout_android_surf;
}

jobject jni_LockAndGetAndroidJavaSurface(int instanceId) {
	vout_android_surface_t *surf = getInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

	pthread_mutex_lock(&surf->vout_android_lock);
    while (surf->vout_android_java_surf == NULL)
        pthread_cond_wait(&surf->vout_android_surf_attached, &surf->vout_android_lock);
    return surf->vout_android_java_surf;
}

void jni_UnlockAndroidSurface(int instanceId) {
	vout_android_surface_map_item item = vout_android_surface_map.find(instanceId);
	if (item == vout_android_surface_map.end()) {
		return;
	}
    pthread_mutex_unlock(&item->second->vout_android_lock);
    sem_post(&vout_android_surface_map_lock);
}
