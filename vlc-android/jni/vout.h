/*****************************************************************************
 * vout.h
 *****************************************************************************
 * Copyright © 2011-2013 VLC authors and VideoLAN
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

#ifndef LIBVLCJNI_VOUT_H
#define LIBVLCJNI_VOUT_H

#include <pthread.h>

typedef struct android_surf_value_t {
    pthread_mutex_t vout_android_lock;
    pthread_cond_t vout_android_surf_attached;
    void *vout_android_surf;
    void *vout_android_gui;
    jobject vout_android_java_surf;
    jobject vout_android_subtitles_surf;
    bool vout_video_player_activity_created;
} android_surf_value_t;

#endif // LIBVLCJNI_VOUT_H
