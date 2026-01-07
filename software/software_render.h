#ifndef SOFTWARE_RENDER_H
#define SOFTWARE_RENDER_H

#include "software/controls.h"
#include "software/vector_math.h"

struct Ray {
    struct Vector origin, direction;
};

void set_camera_software(struct Camera* cam);

void set_camera_default_software(struct Vector pos, struct Vector look, struct Vector up);

void set_camera_settings_software(float _fov_degrees, float _focal_length);

void viewing_ray(
    int i,
    int j,
    struct Ray* ray
);

void wait_for_vsync_software();

void clear_screen_software();

void render_software();

#endif