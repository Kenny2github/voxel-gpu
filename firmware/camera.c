#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include <math.h>
#include "software/vector_math.h"

void set_camera_settings(uint16_t _fov_degrees, uint16_t _focal_length) {
    /* ideally we dont use tan(x) from math.h as its kinda slow but i havent figured it out yet */
    clip_plane_x = (uint16_t)(tan((_fov_degrees / 2.0f) * (M_PI / 180.0f)) * _focal_length);
    clip_plane_y = clip_plane_x / ASPECT_RATIO;
    focal_length = _focal_length;

    GPU->do_render = 1;
}

void set_camera(cam_pos cam, cam_pos look_at, cam_pos up) {
    GPU->camera.pos.x = (uint32_t)cam.x;
    GPU->camera.pos.y = (uint32_t)cam.y;
    GPU->camera.pos.z = (uint32_t)cam.z;

    /* right unit vector on the clipping plane */
    /* look_at and up are already normalized, so cross product is also normalized */
    cam_pos right = {
        .x = look_at.y * up.z - look_at.z * up.y,
        .y = look_at.x * up.z - look_at.z * up.x,
        .z = look_at.x * up.y - look_at.y * up.x
    };

    /* top left */
    GPU->camera.look[0].x = look_at.x * focal_length - right.x * clip_plane_x + up.x * clip_plane_y;
    GPU->camera.look[0].y = look_at.y * focal_length - right.y * clip_plane_x + up.y * clip_plane_y;
    GPU->camera.look[0].z = look_at.z * focal_length - right.z * clip_plane_x + up.z * clip_plane_y;

    /* top right */
    GPU->camera.look[1].x = look_at.x * focal_length + right.x * clip_plane_x + up.x * clip_plane_y;
    GPU->camera.look[1].y = look_at.y * focal_length + right.y * clip_plane_x + up.y * clip_plane_y;
    GPU->camera.look[1].z = look_at.z * focal_length + right.z * clip_plane_x + up.z * clip_plane_y;

    /* bottom left */
    GPU->camera.look[2].x = look_at.x * focal_length - right.x * clip_plane_x - up.x * clip_plane_y;
    GPU->camera.look[2].y = look_at.y * focal_length - right.y * clip_plane_x - up.y * clip_plane_y;
    GPU->camera.look[2].z = look_at.z * focal_length - right.z * clip_plane_x - up.z * clip_plane_y;

    /* bottom right */
    GPU->camera.look[3].x = look_at.x * focal_length + right.x * clip_plane_x - up.x * clip_plane_y;
    GPU->camera.look[3].y = look_at.y * focal_length + right.y * clip_plane_x - up.y * clip_plane_y;
    GPU->camera.look[3].z = look_at.z * focal_length + right.z * clip_plane_x - up.z * clip_plane_y;

    GPU->do_render = 1;
}