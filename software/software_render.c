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

static inline float my_fminf(float a, float b) {
    return (a < b) ? a : b;
}

float check_box_intersection(const uint8_t xp, const uint8_t yp, const uint8_t zp, struct Ray* ray) {

    // Parallel checks for each axis
    if (fabsf(ray->direction.x) < 1e-8f) {
        if (ray->origin.x < xp || ray->origin.x > xp + 1) return -1;
    }
    if (fabsf(ray->direction.y) < 1e-8f) {
        if (ray->origin.y < yp || ray->origin.y > yp + 1) return -1;
    }
    if (fabsf(ray->direction.z) < 1e-8f) {
        if (ray->origin.z < zp || ray->origin.z > zp + 1) return -1;
    }

    struct Vector low;
    low.x = (xp - ray->origin.x) / ray->direction.x;
    low.y = (yp - ray->origin.y) / -ray->direction.y;
    low.z = (zp - ray->origin.z) / ray->direction.z;

    struct Vector high;
    high.x = (xp + 1 - ray->origin.x) / ray->direction.x;
    high.y = (yp + 1 - ray->origin.y) / -ray->direction.y;
    high.z = (zp + 1 - ray->origin.z) / ray->direction.z;

    struct Vector close, far;
    close.x = my_fminf(low.x, high.x);
    close.y = my_fminf(low.y, high.y);
    close.z = my_fminf(low.z, high.z);

    far.x = my_fmaxf(low.x, high.x);
    far.y = my_fmaxf(low.y, high.y);
    far.z = my_fmaxf(low.z, high.z);

    float min_t = max_vec(close);
    float max_t = min_vec(far);

    if (max_t < 0) return -1;
    if (min_t > max_t) return -1;
    return min_t > 0 ? min_t : max_t;

}

void render_software() {

    // Ray-marching based implementation 
    /*
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
    */

    // Ray-casting based implementation
    uint8_t t_tracker[H_RESOLUTION*V_RESOLUTION] = {0};
    struct Ray ray_tracker[H_RESOLUTION*V_RESOLUTION];
    
    for(int x = 0; x < H_RESOLUTION; x++) 
        for(int y = 0; y < V_RESOLUTION; y++) {
            viewing_ray(x, y, &(ray_tracker[x*V_RESOLUTION + y]));
            *(uint16_t*)((uint32_t)(pixel_buffer_software) + (y << 10) + (x << 1)) = 0;
        }

    for(int x = 0; x < SIDE_LEN; x++) {
        for(int z = 0; z < SIDE_LEN; z++) {
            for(int y = 0; y < SIDE_LEN; y++) {
                uint8_t palette = *((uint8_t*)GRID_START + x + z*SIDE_LEN + y*SIDE_LEN*SIDE_LEN);
                if(palette == 0)
                    continue;
                
                for(int xs = 0; xs < H_RESOLUTION; xs++) {
                    for(int ys = 0; ys < V_RESOLUTION; ys++) {
                        float t = check_box_intersection(x, y, z, &(ray_tracker[xs*V_RESOLUTION + ys]));
                        if(t == -1)
                            continue;

                        if(t_tracker[xs*V_RESOLUTION + ys] != 0 && t_tracker[xs*V_RESOLUTION + ys] < t)
                            continue;

                        t_tracker[xs*V_RESOLUTION + ys] = t;

                        *(uint16_t*)((uint32_t)(pixel_buffer_software) + (ys << 10) + (xs << 1)) = palette_data[palette];
                        
                        
                    }
                }
            }
        }
    }
 
}