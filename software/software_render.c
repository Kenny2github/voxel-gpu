
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

typedef struct { int x, y; } Point;

void setup_pixel_buffer_software() {
    pixel_buffer_software = PIXEL_BUF_CTRL->buffer;
}

void wait_for_vsync_software() {
    PIXEL_BUF_CTRL->buffer = 0x1;
    while (PIXEL_BUF_CTRL->status.s);

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
    struct Vector* ray
) {
    float x_frac = 2.0f * (float)i / H_RESOLUTION - 1.0f;
    float y_frac = 2.0f * (float)j / V_RESOLUTION - 1.0f;
    ray->x = camera.look.x * focal_length + camera.right.x * clip_plane_x * x_frac - camera.up.x * clip_plane_y * y_frac;
    ray->z = camera.look.z * focal_length + camera.right.z * clip_plane_x * x_frac - camera.up.z * clip_plane_y * y_frac;
    ray->y = camera.look.y * focal_length + camera.right.y * clip_plane_x * x_frac - camera.up.y * clip_plane_y * y_frac;
}

void plot_pixel(int x, int y, short int line_color) {
    *(short int *)(pixel_buffer_software + (y << 10) + (x << 1)) = line_color;
}

void clear_screen_software() {
    for(int x = 0; x < H_RESOLUTION; x++)
        for(int y = 0; y < V_RESOLUTION; y++) 
            plot_pixel(x, y, 0x0);
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
    float inv_x = 1 / ray->direction.x;
    float inv_y = -1 / ray->direction.y;
    float inv_z = 1 / ray->direction.z;
    low.x = (xp - ray->origin.x) * inv_x;
    low.y = (yp - ray->origin.y) * inv_y;
    low.z = (zp - ray->origin.z) * inv_z;

    struct Vector high;
    high.x = (xp + 1 - ray->origin.x) * inv_x;
    high.y = (yp + 1 - ray->origin.y) * inv_y;
    high.z = (zp + 1 - ray->origin.z) * inv_z;

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


// Recursive partitioning and flood fill for voxel hits
// Returns 1 if a hit was found and filled, 0 otherwise
typedef struct { int x0, y0, x1, y1; } Partition;

static int partition_and_fill(
    int x0, int y0, int x1, int y1,
    int voxel_x, int voxel_y, int voxel_z, uint8_t palette,
    struct Ray* cameraRay,
    struct Vector* topLeft, struct Vector* horizontalVec, struct Vector* verticalVec,
    uint8_t* t_tracker, const uint16_t* palette_data
) {
    int q_start = 0, q_end = 0;
    int found = 0;
    int cx = 0, cy = 0;
    
    Partition* queue = (Partition*)malloc(H_RESOLUTION * V_RESOLUTION * sizeof(Partition));
    queue[q_end++] = (Partition){x0, y0, x1, y1};
    while (!found && q_start < q_end) {
        Partition part = queue[q_start++];
        if (part.x0 > part.x1 || part.y0 > part.y1) continue;
        cx = (part.x0 + part.x1) / 2;
        cy = (part.y0 + part.y1) / 2;
        cameraRay->direction = add_vector(add_vector(*topLeft, multiply_vector(*horizontalVec, cx)), multiply_vector(*verticalVec, V_RESOLUTION-1 - cy));
        float t = check_box_intersection(voxel_x, voxel_y, voxel_z, cameraRay);
        if (t == -1) {
            // No hit, subdivide if possible
            if (part.x0 == part.x1 && part.y0 == part.y1) continue;
            int mx = (part.x0 + part.x1) / 2;
            int my = (part.y0 + part.y1) / 2;
            if (part.x0 < part.x1 || part.y0 < part.y1) {
                queue[q_end++] = (Partition){part.x0, part.y0, mx, my};
                queue[q_end++] = (Partition){mx+1, part.y0, part.x1, my};
                queue[q_end++] = (Partition){part.x0, my+1, mx, part.y1};
                queue[q_end++] = (Partition){mx+1, my+1, part.x1, part.y1};
            }
            continue;
        } else {
            found = 1;
        }
    }
    free(queue);

    if(!found)
        return 0;

    // If hit, flood fill within this section (across the whole screen)
    Point* stack = (Point*)malloc(H_RESOLUTION * V_RESOLUTION * sizeof(Point));
    int stack_size = 0;
    stack[stack_size++] = (Point){cx, cy};
    while (stack_size > 0) {
        Point p = stack[--stack_size];
        int xs = p.x, ys = p.y;
        if (xs < 0 || xs >= H_RESOLUTION || ys < 0 || ys >= V_RESOLUTION)
            continue;
        int idx = xs * V_RESOLUTION + ys;
        if (t_tracker[idx] != 0)
            continue;
        cameraRay->direction = add_vector(add_vector(*topLeft, multiply_vector(*horizontalVec, xs)), multiply_vector(*verticalVec, V_RESOLUTION-1 - ys));
        float t2 = check_box_intersection(voxel_x, voxel_y, voxel_z, cameraRay);
        if (t2 == -1)
            continue;
        t_tracker[idx] = t2;
        plot_pixel(xs, ys, palette_data[palette]);
        stack[stack_size++] = (Point){xs+1, ys};
        stack[stack_size++] = (Point){xs-1, ys};
        stack[stack_size++] = (Point){xs, ys+1};
        stack[stack_size++] = (Point){xs, ys-1};
    }
    free(stack);
    return 1;
}

const int faces[6][4] = {
    {0,1,3,2}, // front
    {4,5,7,6}, // back
    {0,1,5,4}, // top
    {2,3,7,6}, // bottom
    {0,2,6,4}, // left
    {1,3,7,5}  // right
};

const struct Vector normal[6] = {
    {0.0f, 0.0f, 1.0f},   // front
    {0.0f, 0.0f, -1.0f},  // back
    {0.0f, -1.0f, 0.0f},   // top
    {0.0f, 1.0f, 0.0f},  // bottom
    {-1.0f, 0.0f, 0.0f},  // left
    {1.0f, 0.0f, 0.0f}    // right
};

void render_software() {
    // Update software camera before render
    update_camera();

    int res_offset = (PIXEL_BUF_CTRL->x_resolution == 160) ? 1 : 0;
    int x_factor = 0x1 << res_offset;
    int y_factor = 0x1 << res_offset;

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
    /*
    uint8_t t_tracker[H_RESOLUTION*V_RESOLUTION] = {0};
    struct Ray ray_tracker[H_RESOLUTION*V_RESOLUTION];
    
    // TODO: Create a ray for four corners, and during the loop, compute the ray
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
    */

    // Ray-casting based implementation, optimization with 3-corner-camera-ray interpolation and partition-and-floodfill method 
    // uint8_t t_tracker[H_RESOLUTION*V_RESOLUTION] = {0};
    // struct Ray cameraRay;
    // cameraRay.origin = camera.pos;
    
    // struct Vector topLeft, bottomLeft, topRight;
    // viewing_ray(0, 0, &topLeft);
    // viewing_ray(H_RESOLUTION - 1, 0, &topRight);
    // viewing_ray(0, V_RESOLUTION - 1, &bottomLeft);

    // struct Vector horizontalVec = divide_vector(sub_vector(topRight, topLeft), H_RESOLUTION - 1);
    // struct Vector verticalVec = divide_vector(sub_vector(bottomLeft, topLeft), V_RESOLUTION - 1);

    // for(int x = 0; x < SIDE_LEN; x++) {
    //     for(int z = 0; z < SIDE_LEN; z++) {
    //         for(int y = 0; y < SIDE_LEN; y++) {
    //             uint8_t palette = *((uint8_t*)GRID_START + x + z*SIDE_LEN + y*SIDE_LEN*SIDE_LEN);
    //             if(palette == 0)
    //                 continue;
                    
    //             partition_and_fill(
    //                 0, 0, H_RESOLUTION-1, V_RESOLUTION-1,
    //                 x, y, z, palette,
    //                 &cameraRay, &topLeft, &horizontalVec, &verticalVec,
    //                 t_tracker, palette_data
    //             );
    //         }
    //     }
    // }
    
    // Rasterization, object-centric algorithm

    /*
    Rasterization method:
    - Project the 8 corners of the cube into the clip plane
    - Take the min and max of the screen's X and Y
    - Iterate through a pixels between min and max of X and Y
    - Check if pixel is inside the projected faces (Simply brute force and consider all 6 faces. Can optimize based on normal of face and camera look vector)
    - If in pixel, color. 
    */
    const float frac_x_const = focal_length / clip_plane_x;
    const float frac_y_const = focal_length / clip_plane_y;

    const float V_RESOLUTION_HALF = (V_RESOLUTION >> 1);
    const float H_RESOLUTION_HALF = (H_RESOLUTION >> 1);

    // Compute if a face is pointing at the camera or not
    //     const int faces[6][4] = {
    //     {0,1,3,2}, // front
    //     {4,5,7,6}, // back
    //     {0,1,5,4}, // top
    //     {2,3,7,6}, // bottom
    //     {0,2,6,4}, // left
    //     {1,3,7,5}  // right
    // };


    for(uint8_t x = 0; x < SIDE_LEN; x++) {
        for(uint8_t z = 0; z < SIDE_LEN; z++) {
            for(uint8_t y = 0; y < SIDE_LEN; y++) {
                uint8_t palette = *((uint8_t*)GRID_START + x + z*SIDE_LEN + y*SIDE_LEN*SIDE_LEN);
                if (palette == 0) continue;

                struct Vector corners[8];
                corners[0] = (struct Vector){x,     y,     z+1};     // top-left-front
                corners[1] = (struct Vector){x+1,   y,     z+1};     // top-right-front
                corners[2] = (struct Vector){x,     y+1,   z+1};     // bottom-left-front
                corners[3] = (struct Vector){x+1,   y+1,   z+1};     // bottom-right-front
                corners[4] = (struct Vector){x,     y,     z};   // top-left-back
                corners[5] = (struct Vector){x+1,   y,     z};   // top-right-back
                corners[6] = (struct Vector){x,     y+1,   z};   // bottom-left-back
                corners[7] = (struct Vector){x+1,   y+1,   z};   // bottom-right-back

                float screen_x[8], screen_y[8];
                for (int i = 0; i < 8; i++) {
                    struct Vector diff;
                    diff.x = corners[i].x - camera.pos.x;
                    diff.y = corners[i].y - camera.pos.y;
                    diff.z = corners[i].z - camera.pos.z;

                    float cam_z = diff.x * camera.look.x + diff.y * camera.look.y + diff.z * camera.look.z;
                    float cam_x = diff.x * camera.right.x + diff.y * camera.right.y + diff.z * camera.right.z;
                    float cam_y = diff.x * camera.up.x + diff.y * camera.up.y + diff.z * camera.up.z;

                    float cam_z_inv = 1 / cam_z;
                    float x_frac = (cam_x * frac_x_const) * cam_z_inv;
                    float y_frac = -(cam_y * frac_y_const) * cam_z_inv;

                    screen_x[i] = (x_frac + 1.0f) * H_RESOLUTION_HALF;
                    screen_y[i] = (y_frac + 1.0f) * V_RESOLUTION_HALF;
                }

                float min_px = screen_x[0], max_px = screen_x[0];
                float min_py = screen_y[0], max_py = screen_y[0];
                for (int i = 1; i < 8; i++) {
                    if (screen_x[i] < min_px) min_px = screen_x[i];
                    if (screen_x[i] > max_px) max_px = screen_x[i];
                    if (screen_y[i] < min_py) min_py = screen_y[i];
                    if (screen_y[i] > max_py) max_py = screen_y[i];
                }

                int min_px_i = (int)(min_px);
                int max_px_i = (int)(max_px);
                int min_py_i = (int)(min_py);
                int max_py_i = (int)(max_py);

                if (min_px_i < 0) min_px_i = 0;
                if (max_px_i >= H_RESOLUTION) max_px_i = H_RESOLUTION - 1;
                if (min_py_i < 0) min_py_i = 0;
                if (max_py_i >= V_RESOLUTION) max_py_i = V_RESOLUTION - 1;
                
                uint8_t face_enable[6] = {0};

                for(int i = 0; i < 6; i++) {
                    struct Vector diff = {x - camera.pos.x, y - camera.pos.y, z - camera.pos.z};

                    diff.x += normal[i].x == 1;
                    diff.y += normal[i].y == 1;
                    diff.z += normal[i].z == 1;

                    float dot = diff.x*normal[i].x + diff.y*normal[i].y + diff.z*normal[i].z;
                    face_enable[i] = dot < 0;
                }

                for (int py = min_py_i; py <= max_py_i; py++) {
                    int left = min_px_i, right = max_px_i;
                    int found_left = 0, found_right = 0;
                    // Find leftmost inside pixel
                    for (; left <= max_px_i; left++) {
                        int inside = 0;
                        for (int f = 0; f < 6; f++) {
                        
                            
                            if(!face_enable[f])
                                continue;

            
                            int i0 = faces[f][0], i1 = faces[f][1], i2 = faces[f][2], i3 = faces[f][3];
                            float sx[4] = {screen_x[i0], screen_x[i1], screen_x[i2], screen_x[i3]};
                            float sy[4] = {screen_y[i0], screen_y[i1], screen_y[i2], screen_y[i3]};
                            int sign = 0;
                            uint8_t set_sign = 0;
                            for (int e = 0; e < 4; e++) {
                                int next = (e + 1) % 4;
                                float ex = sx[next] - sx[e];
                                float ey = sy[next] - sy[e];
                                float pxex = left - sx[e];
                                float pyey = py - sy[e];
                                float cross = ex * pyey - ey * pxex;
                                if (cross == 0) continue;
                                if (set_sign == 0 && ++set_sign) sign = (cross > 0) ? 1 : -1;
                                else if ((cross > 0 && sign < 0) || (cross < 0 && sign > 0)) {
                                    sign = 0;
                                    break;
                                }
                            }
                            if (sign != 0) {
                                inside = 1;
                                break;
                            }
                        }
                        if (inside) { found_left = 1; break; }
                    }
                    
                    // Find rightmost inside pixel
                    for (; right >= min_px_i; right--) {
                        int inside = 0;
                        for (int f = 0; f < 6; f++) {
                            
                            if(!face_enable[f])
                                continue;

                            int i0 = faces[f][0], i1 = faces[f][1], i2 = faces[f][2], i3 = faces[f][3];
                            float sx[4] = {screen_x[i0], screen_x[i1], screen_x[i2], screen_x[i3]};
                            float sy[4] = {screen_y[i0], screen_y[i1], screen_y[i2], screen_y[i3]};
                            int sign = 0;
                            uint8_t set_sign = 0;
                            for (int e = 0; e < 4; e++) {
                                int next = (e + 1) % 4;
                                float ex = sx[next] - sx[e];
                                float ey = sy[next] - sy[e];
                                float pxex = right - sx[e];
                                float pyey = py - sy[e];
                                float cross = ex * pyey - ey * pxex;
                                if (cross == 0) continue;
                                if (set_sign == 0 && ++set_sign) sign = (cross > 0) ? 1 : -1;
                                else if ((cross > 0 && sign < 0) || (cross < 0 && sign > 0)) {
                                    sign = 0;
                                    break;
                                }
                            }
                            if (sign != 0) {
                                inside = 1;
                                break;
                            }
                        }
                        if (inside) { found_right = 1; break; }
                    }
                    if (found_left && found_right && left <= right) {
                        for (int px = left; px <= right; px++) {
                            plot_pixel(px, py, palette_data[palette]);
                        }
                    }
                }
            }
        }
    }

 
}
