#include "software/software_render.h"
#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/palette.h"

#define H_RESOLUTION 256
#define V_RESOLUTION 192

static struct Camera camera;
static float clip_plane_x, clip_plane_y;
static float focal_length;
unsigned char* pixel_buffer_software;

void setup_pixel_buffer_software() {
    pixel_buffer_software = PIXEL_BUF_CTRL->back_buffer;
}

void wait_for_vsync_software() {
    *((int*)PIXEL_BUF_CTRL) = 1;
    while ((*((int*)PIXEL_BUF_CTRL + 3) & 0x1) != 0);

    pixel_buffer_software = PIXEL_BUF_CTRL->back_buffer;

}

void set_camera_default_software(struct Vector pos, struct Vector look, struct Vector up) {
    camera = (struct Camera){
        pos,
        look,
        up,
        {0, 0, 1}
    };

    cross_product(&(camera.look), &(camera.up), &(camera.right));
    normalize(&(camera.look));
    normalize(&(camera.up));
    normalize(&(camera.right));

}

void set_camera_software(struct Camera* cam) {
    camera.look = cam->look;
    camera.pos = cam->pos;
    camera.up = cam->up;
    camera.right = cam->right;
}

void set_camera_settings_software(float _fov_degrees, float _focal_length) {
    float angle = (_fov_degrees / 2.0f) * (M_PI / 180.0f);
    clip_plane_x = sinf(angle) / cosf(angle) * _focal_length;
    clip_plane_y = clip_plane_x / ASPECT_RATIO;
    focal_length = _focal_length;
}

void viewing_ray(
    int i,
    int j,
    struct Ray* ray
) {
    ray->origin = camera.pos;
    float x_frac = 2.0f * (float)i / H_RESOLUTION - 1.0f;
    float y_frac = 2.0f * (float)j / V_RESOLUTION - 1.0f;
    ray->direction.x = camera.look.x * focal_length + camera.right.x * clip_plane_x * x_frac - camera.up.x * clip_plane_y * y_frac;
    ray->direction.z = camera.look.z * focal_length + camera.right.z * clip_plane_x * x_frac - camera.up.z * clip_plane_y * y_frac;
    ray->direction.y = camera.look.y * focal_length + camera.right.y * clip_plane_x * x_frac - camera.up.y * clip_plane_y * y_frac;
}

void clear_screen_software() {
    for(int x = 0; x < H_RESOLUTION; x++)
        for(int y = 0; y < V_RESOLUTION; y++) 
            *(uint16_t*)((uint32_t)(pixel_buffer_software) + (y << 10) + (x << 1)) = 0xFFFF;
}

static inline float my_fmaxf(float a, float b) {
    return (a > b) ? a : b;
}

void render_software() {

    // Ray-casting based implementation 
    for(int x = 0; x < H_RESOLUTION; x++) {
        for(int y = 0; y < V_RESOLUTION; y++) {
            struct Ray camera_ray;
            viewing_ray(x, y, &camera_ray);
            
            struct Vector add_vec = divide_vector(camera_ray.direction, my_fmaxf(max_vec(camera_ray.direction), fabsf(min_vec(camera_ray.direction))));

            struct Vector curr_pos = add_vector(camera_ray.origin, camera_ray.direction);
            uint8_t palette = 0;
            int step = 0;
            while(max_vec(curr_pos) < SIDE_LEN && min_vec(curr_pos) >= 0 && palette == 0 && step++ < 5) {
                int xi = (int)(curr_pos.x), yi = (int)(curr_pos.y+0.5f), zi = (int)(curr_pos.z);
                curr_pos = add_vector(curr_pos, add_vec);
                palette = *((uint8_t*)GRID_START + xi + zi*SIDE_LEN + yi*SIDE_LEN*SIDE_LEN);
            }

            if(palette > 0) {
                *(uint16_t*)((uint32_t)(pixel_buffer_software) + (y << 10) + (x << 1)) = palette_data[palette];
            } else {
                *(uint16_t*)((uint32_t)(pixel_buffer_software) + (y << 10) + (x << 1)) = 0x0;

            }
            
        }
    }
}