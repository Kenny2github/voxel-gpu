#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "software/vector_math.h"

static float clip_plane_x, clip_plane_y;
static float fov_degrees, focal_length;
static float tanf_angle;

void set_camera_settings(float _fov_degrees, float _focal_length) {
    /* reduce need to invoke sinf/cosf */
    if (_fov_degrees != fov_degrees) {
        fov_degrees = _fov_degrees;
        float angle = (_fov_degrees / 2.0f) * (M_PI / 180.0f);
        tanf_angle = sinf(angle) / cosf(angle);
    }

    clip_plane_x = tanf_angle * _focal_length;
    clip_plane_y = clip_plane_x / ASPECT_RATIO;
    focal_length = _focal_length;
}


// TODO: Split set_camera component-wise for small optimization, minimizing calls
void set_camera(struct Camera* cam) {
    GPU->camera.pos = (struct _vec3){
        (uint32_t)convert_float_to_fixed(cam->pos.x),
        (uint32_t)convert_float_to_fixed(cam->pos.y),
        (uint32_t)convert_float_to_fixed(cam->pos.z)
    };

    /* right unit vector on the clipping plane */
    /* cam->look and up are already normalized, so cross product is also normalized */

    /* TODO: optimize? */
    const float look_x = cam->look.x * focal_length;
    const float look_y = cam->look.y * focal_length;
    const float look_z = cam->look.z * focal_length;
    const float right_x = cam->right.x * clip_plane_x;
    const float right_y = cam->right.y * clip_plane_x;
    const float right_z = cam->right.z * clip_plane_x;
    const float up_x = cam->up.x * clip_plane_y;
    const float up_y = cam->up.y * clip_plane_y;
    const float up_z = cam->up.z * clip_plane_y;

    /* top left */
    GPU->camera.look[0] = (struct _vec3){
        (uint32_t)convert_float_to_fixed(look_x - right_x + up_x),
        (uint32_t)convert_float_to_fixed(look_y - right_y + up_y),
        (uint32_t)convert_float_to_fixed(look_z - right_z + up_z)
    };

    /* top right */
    GPU->camera.look[1] = (struct _vec3){
        (uint32_t)convert_float_to_fixed(look_x + right_x + up_x),
        (uint32_t)convert_float_to_fixed(look_y + right_y + up_y),
        (uint32_t)convert_float_to_fixed(look_z + right_z + up_z)
    };

    /* bottom left */
    GPU->camera.look[2] = (struct _vec3){
        (uint32_t)convert_float_to_fixed(look_x - right_x - up_x),
        (uint32_t)convert_float_to_fixed(look_y - right_y - up_y),
        (uint32_t)convert_float_to_fixed(look_z - right_z - up_z)
    };

    /* bottom right */
    GPU->camera.look[3] = (struct _vec3){
        (uint32_t)convert_float_to_fixed(look_x + right_x - up_x),
        (uint32_t)convert_float_to_fixed(look_y + right_y - up_y),
        (uint32_t)convert_float_to_fixed(look_z + right_z - up_z)
    };

}