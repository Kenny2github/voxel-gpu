#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "software/vector_math.h"

static float clip_plane_x, clip_plane_y;
static float focal_length;

void set_camera_settings(float _fov_degrees, float _focal_length) {
    float angle = (_fov_degrees / 2.0f) * (M_PI / 180.0f);
    clip_plane_x = sinf(angle) / cosf(angle) * _focal_length;
    clip_plane_y = clip_plane_x / ASPECT_RATIO;
    focal_length = _focal_length;

    render();
}

void set_camera(cam_pos cam, cam_pos look_at, cam_pos up) {
    GPU->camera.pos.x = (uint32_t)convert_float_to_fixed(cam.x);
    GPU->camera.pos.y = (uint32_t)convert_float_to_fixed(cam.y);
    GPU->camera.pos.z = (uint32_t)convert_float_to_fixed(cam.z);

    /* right unit vector on the clipping plane */
    /* look_at and up are already normalized, so cross product is also normalized */
    cam_pos right = {
        .x = look_at.y * up.z - look_at.z * up.y,
        .y = look_at.x * up.z - look_at.z * up.x,
        .z = look_at.x * up.y - look_at.y * up.x
    };

    /* TODO: optimize? */
    /* top left */
    GPU->camera.look[0].x = (uint32_t)convert_float_to_fixed(look_at.x * focal_length - right.x * clip_plane_x + up.x * clip_plane_y);
    GPU->camera.look[0].y = (uint32_t)convert_float_to_fixed(look_at.y * focal_length - right.y * clip_plane_x + up.y * clip_plane_y);
    GPU->camera.look[0].z = (uint32_t)convert_float_to_fixed(look_at.z * focal_length - right.z * clip_plane_x + up.z * clip_plane_y);

    /* top right */
    GPU->camera.look[1].x = (uint32_t)convert_float_to_fixed(look_at.x * focal_length + right.x * clip_plane_x + up.x * clip_plane_y);
    GPU->camera.look[1].y = (uint32_t)convert_float_to_fixed(look_at.y * focal_length + right.y * clip_plane_x + up.y * clip_plane_y);
    GPU->camera.look[1].z = (uint32_t)convert_float_to_fixed(look_at.z * focal_length + right.z * clip_plane_x + up.z * clip_plane_y);

    /* bottom left */
    GPU->camera.look[2].x = (uint32_t)convert_float_to_fixed(look_at.x * focal_length - right.x * clip_plane_x - up.x * clip_plane_y);
    GPU->camera.look[2].y = (uint32_t)convert_float_to_fixed(look_at.y * focal_length - right.y * clip_plane_x - up.y * clip_plane_y);
    GPU->camera.look[2].z = (uint32_t)convert_float_to_fixed(look_at.z * focal_length - right.z * clip_plane_x - up.z * clip_plane_y);

    /* bottom right */
    GPU->camera.look[3].x = (uint32_t)convert_float_to_fixed(look_at.x * focal_length + right.x * clip_plane_x - up.x * clip_plane_y);
    GPU->camera.look[3].y = (uint32_t)convert_float_to_fixed(look_at.y * focal_length + right.y * clip_plane_x - up.y * clip_plane_y);
    GPU->camera.look[3].z = (uint32_t)convert_float_to_fixed(look_at.z * focal_length + right.z * clip_plane_x - up.z * clip_plane_y);

    render();
}