/*****************************************************************************
 * vout.c
 *****************************************************************************
 * Copyright © 2010-2013 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <vlc/vlc.h>
#include <vlc_common.h>

#include <jni.h>
#include "vout.h"

#define THREAD_NAME "jni_vout"
extern int jni_attach_thread(JNIEnv **env, const char *thread_name);
extern void jni_detach_thread();


void *jni_LockAndGetSubtitlesSurface(android_surf_value_t *android_surface) {
    pthread_mutex_lock(&android_surface->vout_android_lock);
    while (android_surface->vout_android_subtitles_surf == NULL)
        pthread_cond_wait(&android_surface->vout_android_surf_attached, &android_surface->vout_android_lock);
    return android_surface->vout_android_subtitles_surf;
}

void *jni_LockAndGetAndroidSurface(android_surf_value_t *android_surface) {
    pthread_mutex_lock(&android_surface->vout_android_lock);
    while (android_surface->vout_android_surf == NULL)
        pthread_cond_wait(&android_surface->vout_android_surf_attached, &android_surface->vout_android_lock);
    return android_surface->vout_android_surf;
}

jobject jni_LockAndGetAndroidJavaSurface(android_surf_value_t *android_surface) {
    pthread_mutex_lock(&android_surface->vout_android_lock);
    while (android_surface->vout_android_java_surf == NULL)
        pthread_cond_wait(&android_surface->vout_android_surf_attached, &android_surface->vout_android_lock);
    return android_surface->vout_android_java_surf;
}

void jni_UnlockAndroidSurface(android_surf_value_t *android_surface) {
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

void jni_EventHardwareAccelerationError(android_surf_value_t *android_surface)
{
    if (android_surface->vout_android_gui == NULL)
        return;

    JNIEnv *env;
    jni_attach_thread(&env, THREAD_NAME);

    jclass cls = (*env)->GetObjectClass(env, android_surface->vout_android_gui);
    jmethodID methodId = (*env)->GetMethodID(env, cls, "eventHardwareAccelerationError", "()V");
    (*env)->CallVoidMethod(env, android_surface->vout_android_gui, methodId);

    (*env)->DeleteLocalRef(env, cls);
    jni_detach_thread();
}

void jni_SetAndroidSurfaceSizeEnv(JNIEnv *p_env, android_surf_value_t *android_surface, int width, int height, int visible_width, int visible_height, int sar_num, int sar_den)
{
    if (android_surface->vout_android_gui == NULL)
        return;

    jclass cls = (*p_env)->GetObjectClass (p_env, android_surface->vout_android_gui);
    jmethodID methodId = (*p_env)->GetMethodID (p_env, cls, "setSurfaceSize", "(IIIIII)V");

    (*p_env)->CallVoidMethod (p_env, android_surface->vout_android_gui, methodId, width, height, visible_width, visible_height, sar_num, sar_den);

    (*p_env)->DeleteLocalRef(p_env, cls);
}

void jni_SetAndroidSurfaceSize(android_surf_value_t *android_surface, int width, int height, int visible_width, int visible_height, int sar_num, int sar_den)
{
    JNIEnv *env;
    jni_attach_thread(&env, THREAD_NAME);

    jni_SetAndroidSurfaceSizeEnv(env, android_surface, width, height, visible_width, visible_height, sar_num, sar_den);
    jni_detach_thread();
}

bool jni_IsVideoPlayerActivityCreated(android_surf_value_t *android_surface) {
    pthread_mutex_lock(&android_surface->vout_android_lock);
    bool result = android_surface->vout_video_player_activity_created;
    pthread_mutex_unlock(&android_surface->vout_android_lock);
    return result;
}

void Java_org_videolan_libvlc_LibVLC_eventVideoPlayerActivityCreated(JNIEnv *env, jobject thiz, jboolean created) {
    android_surf_value_t * android_surface = (android_surf_value_t *)(intptr_t)getLong(env, thiz, "mAndroidSurfaceValue");

    pthread_mutex_lock(&android_surface->vout_android_lock);
    android_surface->vout_video_player_activity_created = created;
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

void Java_org_videolan_libvlc_LibVLC_attachSurface(JNIEnv *env, jobject thiz, jobject surf, jobject gui) {
    android_surf_value_t * android_surface = (android_surf_value_t *)(intptr_t)getLong(env, thiz, "mAndroidSurfaceValue");

    pthread_mutex_lock(&android_surface->vout_android_lock);
    jclass clz;
    jfieldID fid;

    clz = (*env)->FindClass(env, "org/videolan/libvlc/LibVlcUtil");
    jmethodID methodId = (*env)->GetStaticMethodID(env, clz, "isGingerbreadOrLater", "()Z");
    jboolean gingerbreadOrLater = (*env)->CallStaticBooleanMethod(env, clz, methodId);
    // Android 2.2 and under don't have ANativeWindow_fromSurface
    if(unlikely(!gingerbreadOrLater)) {
        clz = (*env)->GetObjectClass(env, surf);
        fid = (*env)->GetFieldID(env, clz, "mSurface", "I");
        if (fid == NULL) {
            jthrowable exp = (*env)->ExceptionOccurred(env);
            if (exp) {
                (*env)->DeleteLocalRef(env, exp);
                (*env)->ExceptionClear(env);
            }
            fid = (*env)->GetFieldID(env, clz, "mNativeSurface", "I");
        }
        android_surface->vout_android_surf = (void*)(*env)->GetIntField(env, surf, fid);
        (*env)->DeleteLocalRef(env, clz);
    }
    android_surface->vout_android_gui = (*env)->NewGlobalRef(env, gui);
    android_surface->vout_android_java_surf = (*env)->NewGlobalRef(env, surf);
    pthread_cond_signal(&android_surface->vout_android_surf_attached);
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

void Java_org_videolan_libvlc_LibVLC_detachSurface(JNIEnv *env, jobject thiz) {
    android_surf_value_t * android_surface = (android_surf_value_t *)(intptr_t)getLong(env, thiz, "mAndroidSurfaceValue");

    pthread_mutex_lock(&android_surface->vout_android_lock);
    android_surface->vout_android_surf = NULL;
    if (android_surface->vout_android_gui != NULL)
        (*env)->DeleteGlobalRef(env, android_surface->vout_android_gui);
    if (android_surface->vout_android_java_surf != NULL)
        (*env)->DeleteGlobalRef(env, android_surface->vout_android_java_surf);
    android_surface->vout_android_gui = NULL;
    android_surface->vout_android_java_surf = NULL;
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

void Java_org_videolan_libvlc_LibVLC_attachSubtitlesSurface(JNIEnv *env, jobject thiz, jobject surf) {
    android_surf_value_t * android_surface = (android_surf_value_t *)(intptr_t)getLong(env, thiz, "mAndroidSurfaceValue");

    pthread_mutex_lock(&android_surface->vout_android_lock);
    android_surface->vout_android_subtitles_surf = (*env)->NewGlobalRef(env, surf);
    pthread_cond_signal(&android_surface->vout_android_surf_attached);
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

void Java_org_videolan_libvlc_LibVLC_detachSubtitlesSurface(JNIEnv *env, jobject thiz) {
    android_surf_value_t * android_surface = (android_surf_value_t *)(intptr_t)getLong(env, thiz, "mAndroidSurfaceValue");

    pthread_mutex_lock(&android_surface->vout_android_lock);
    if (android_surface->vout_android_subtitles_surf != NULL)
        (*env)->DeleteGlobalRef(env, android_surface->vout_android_subtitles_surf);
    android_surface->vout_android_subtitles_surf = NULL;
    pthread_mutex_unlock(&android_surface->vout_android_lock);
}

static int mouse_x = -1;
static int mouse_y = -1;
static int mouse_button = -1;
static int mouse_action = -1;

void Java_org_videolan_libvlc_LibVLC_sendMouseEvent(JNIEnv* env, jobject thiz, jint action, jint button, jint x, jint y)
{
    mouse_x = x;
    mouse_y = y;
    mouse_button = button;
    mouse_action = action;
}

void jni_getMouseCoordinates(int *action, int *button, int *x, int *y)
{
    *x = mouse_x;
    *y = mouse_y;
    *button = mouse_button;
    *action = mouse_action;

    mouse_button = mouse_action = mouse_x = mouse_y = -1;
}
