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
}


// TODO: Split set_camera component-wise for small optimization, minimizing calls
void set_camera(struct Camera* cam) {
    GPU->camera.pos.x = (uint32_t)convert_float_to_fixed(cam->pos.x);
    GPU->camera.pos.y = (uint32_t)convert_float_to_fixed(cam->pos.y);
    GPU->camera.pos.z = (uint32_t)convert_float_to_fixed(cam->pos.z);

    /* right unit vector on the clipping plane */
    /* cam->look and up are already normalized, so cross product is also normalized */

    /* TODO: optimize? */
    /* top left */
    GPU->camera.look[0].x = (uint32_t)convert_float_to_fixed(cam->look.x * focal_length - cam->right.x * clip_plane_x + cam->up.x * clip_plane_y);
    GPU->camera.look[0].y = (uint32_t)convert_float_to_fixed(cam->look.y * focal_length - cam->right.y * clip_plane_x + cam->up.y * clip_plane_y);
    GPU->camera.look[0].z = (uint32_t)convert_float_to_fixed(cam->look.z * focal_length - cam->right.z * clip_plane_x + cam->up.z * clip_plane_y);

    /* top right */
    GPU->camera.look[1].x = (uint32_t)convert_float_to_fixed(cam->look.x * focal_length + cam->right.x * clip_plane_x + cam->up.x * clip_plane_y);
    GPU->camera.look[1].y = (uint32_t)convert_float_to_fixed(cam->look.y * focal_length + cam->right.y * clip_plane_x + cam->up.y * clip_plane_y);
    GPU->camera.look[1].z = (uint32_t)convert_float_to_fixed(cam->look.z * focal_length + cam->right.z * clip_plane_x + cam->up.z * clip_plane_y);

    /* bottom left */
    GPU->camera.look[2].x = (uint32_t)convert_float_to_fixed(cam->look.x * focal_length - cam->right.x * clip_plane_x - cam->up.x * clip_plane_y);
    GPU->camera.look[2].y = (uint32_t)convert_float_to_fixed(cam->look.y * focal_length - cam->right.y * clip_plane_x - cam->up.y * clip_plane_y);
    GPU->camera.look[2].z = (uint32_t)convert_float_to_fixed(cam->look.z * focal_length - cam->right.z * clip_plane_x - cam->up.z * clip_plane_y);

    /* bottom right */
    GPU->camera.look[3].x = (uint32_t)convert_float_to_fixed(cam->look.x * focal_length + cam->right.x * clip_plane_x - cam->up.x * clip_plane_y);
    GPU->camera.look[3].y = (uint32_t)convert_float_to_fixed(cam->look.y * focal_length + cam->right.y * clip_plane_x - cam->up.y * clip_plane_y);
    GPU->camera.look[3].z = (uint32_t)convert_float_to_fixed(cam->look.z * focal_length + cam->right.z * clip_plane_x - cam->up.z * clip_plane_y);

}