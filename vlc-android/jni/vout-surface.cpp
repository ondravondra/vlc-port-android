#include <pthread.h>
#include <semaphore.h>
#include <tr1/unordered_map>

#include <jni.h>

#include "vout-surface.h"

std::tr1::unordered_map<int, vout_android_surface_t*> vout_android_surface_map;
int vout_android_surface_map_seq = 0;
typedef std::tr1::unordered_map<int, vout_android_surface_t*>::const_iterator vout_android_surface_map_item;
#define MAP_LOCK_MAX 32
sem_t vout_android_surface_map_lock = { MAP_LOCK_MAX };

pthread_mutex_t vout_android_surface_map_lock_lock;
static inline void writeLockMap() {
	pthread_mutex_lock(&vout_android_surface_map_lock_lock);
	for (int i = 0; i < MAP_LOCK_MAX; i ++) {
		sem_wait(&vout_android_surface_map_lock);
	}
	pthread_mutex_unlock(&vout_android_surface_map_lock_lock);
}

static inline void writeUnlockMap() {
	for (int i = 0; i < MAP_LOCK_MAX; i ++) {
		sem_post(&vout_android_surface_map_lock);
	}
}

vout_android_surface_t* useVoutAndroidInstanceSurface(int instanceId) {
	sem_wait(&vout_android_surface_map_lock);
	vout_android_surface_map_item item = vout_android_surface_map.find(instanceId);
	if (item == vout_android_surface_map.end()) {
		return NULL;
	}

	return item->second;
}

int createVoutAndroidInstance() {
	writeLockMap();
	int i = ++vout_android_surface_map_seq;
	vout_android_surface_t *surf = new vout_android_surface_t();
	vout_android_surface_map.insert(std::make_pair<int, vout_android_surface_t*>(i, surf));
	writeUnlockMap();
	return i;
}

void *jni_LockAndGetSubtitlesSurface(int instanceId) {
	vout_android_surface_t *surf = useVoutAndroidInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

    pthread_mutex_lock(&surf->vout_android_lock);
    while (surf->vout_android_subtitles_surf == NULL)
        pthread_cond_wait(&surf->vout_android_surf_attached, &surf->vout_android_lock);
    return surf->vout_android_subtitles_surf;
}

void *jni_LockAndGetAndroidSurface(int instanceId) {
	vout_android_surface_t *surf = useVoutAndroidInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

    pthread_mutex_lock(&surf->vout_android_lock);
    while (surf->vout_android_surf == NULL)
        pthread_cond_wait(&surf->vout_android_surf_attached, &surf->vout_android_lock);
    return surf->vout_android_surf;
}

jobject jni_LockAndGetAndroidJavaSurface(int instanceId) {
	vout_android_surface_t *surf = useVoutAndroidInstanceSurface(instanceId);
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
    releaseVoutAndroidInstanceSurface();
}

void releaseVoutAndroidInstanceSurface() {
	sem_post(&vout_android_surface_map_lock);
}

bool jni_IsVideoPlayerActivityCreated(int instanceId) {
	vout_android_surface_t *surf = useVoutAndroidInstanceSurface(instanceId);
	if (!surf) {
		return NULL;
	}

    pthread_mutex_lock(&surf->vout_android_lock);
    bool result = surf->vout_video_player_activity_created;
    pthread_mutex_unlock(&surf->vout_android_lock);
    releaseVoutAndroidInstanceSurface();
    return result;
}
