#ifndef SKYBLOCK_H
#define SKYBLOCK_H

#include "firmware/firmware.h"


void load_skyblock() {
	set_voxel((v_pos){0, 0, 0}, 1);
	set_voxel((v_pos){1, 0, 0}, 1);
	set_voxel((v_pos){2, 0, 0}, 1);
	set_voxel((v_pos){3, 0, 0}, 1);
	set_voxel((v_pos){4, 0, 0}, 1);
	set_voxel((v_pos){5, 0, 0}, 1);
	set_voxel((v_pos){0, 0, 1}, 1);
	set_voxel((v_pos){1, 0, 1}, 1);
	set_voxel((v_pos){2, 0, 1}, 1);
	set_voxel((v_pos){3, 0, 1}, 1);
	set_voxel((v_pos){4, 0, 1}, 1);
	set_voxel((v_pos){5, 0, 1}, 1);
	set_voxel((v_pos){0, 0, 2}, 1);
	set_voxel((v_pos){1, 0, 2}, 1);
	set_voxel((v_pos){2, 0, 2}, 1);
	set_voxel((v_pos){3, 0, 2}, 1);
	set_voxel((v_pos){4, 0, 2}, 1);
	set_voxel((v_pos){5, 0, 2}, 1);

	set_voxel((v_pos){3, 0, 3}, 1);
	set_voxel((v_pos){4, 0, 3}, 1);
	set_voxel((v_pos){5, 0, 3}, 1);
	set_voxel((v_pos){3, 0, 4}, 1);
	set_voxel((v_pos){4, 0, 4}, 1);
	set_voxel((v_pos){5, 0, 4}, 1);
	set_voxel((v_pos){3, 0, 5}, 1);
	set_voxel((v_pos){4, 0, 5}, 1);
	set_voxel((v_pos){5, 0, 5}, 1);

	set_voxel((v_pos){0, 1, 0}, 1);
	set_voxel((v_pos){1, 1, 0}, 1);
	set_voxel((v_pos){2, 1, 0}, 1);
	set_voxel((v_pos){3, 1, 0}, 1);
	set_voxel((v_pos){4, 1, 0}, 1);
	set_voxel((v_pos){5, 1, 0}, 1);
	set_voxel((v_pos){0, 1, 1}, 1);
	set_voxel((v_pos){5, 1, 1}, 1);
	set_voxel((v_pos){0, 1, 2}, 1);
	set_voxel((v_pos){1, 1, 2}, 1);
	set_voxel((v_pos){2, 1, 2}, 1);
	set_voxel((v_pos){5, 1, 2}, 1);

	set_voxel((v_pos){3, 1, 3}, 1);
	set_voxel((v_pos){4, 1, 3}, 1);
	set_voxel((v_pos){5, 1, 3}, 1);
	set_voxel((v_pos){3, 1, 4}, 1);
	set_voxel((v_pos){4, 1, 4}, 1);
	set_voxel((v_pos){5, 1, 4}, 1);
	set_voxel((v_pos){3, 1, 5}, 1);
	set_voxel((v_pos){4, 1, 5}, 1);
	set_voxel((v_pos){5, 1, 5}, 1);

	set_voxel((v_pos){0, 2, 0}, 2);
	set_voxel((v_pos){1, 2, 0}, 2);
	set_voxel((v_pos){2, 2, 0}, 2);
	set_voxel((v_pos){3, 2, 0}, 2);
	set_voxel((v_pos){4, 2, 0}, 2);
	set_voxel((v_pos){5, 2, 0}, 2);
	set_voxel((v_pos){0, 2, 1}, 2);
	set_voxel((v_pos){1, 2, 1}, 2);
	set_voxel((v_pos){2, 2, 1}, 2);
	set_voxel((v_pos){3, 2, 1}, 2);
	set_voxel((v_pos){4, 2, 1}, 2);
	set_voxel((v_pos){5, 2, 1}, 2);
	set_voxel((v_pos){0, 2, 2}, 2);
	set_voxel((v_pos){1, 2, 2}, 2);
	set_voxel((v_pos){2, 2, 2}, 2);
	set_voxel((v_pos){3, 2, 2}, 2);
	set_voxel((v_pos){4, 2, 2}, 2);
	set_voxel((v_pos){5, 2, 2}, 2);

	set_voxel((v_pos){3, 2, 3}, 2);
	set_voxel((v_pos){4, 2, 3}, 2);
	set_voxel((v_pos){5, 2, 3}, 2);
	set_voxel((v_pos){3, 2, 4}, 2);
	set_voxel((v_pos){5, 2, 4}, 2);
	set_voxel((v_pos){3, 2, 5}, 2);
	set_voxel((v_pos){4, 2, 5}, 2);
	set_voxel((v_pos){5, 2, 5}, 2);

	set_voxel((v_pos){4, 4, 4}, 1);
	set_voxel((v_pos){4, 5, 4}, 1);
	set_voxel((v_pos){3, 6, 4}, 3);
	set_voxel((v_pos){5, 6, 4}, 3);
	set_voxel((v_pos){4, 6, 3}, 3);
	set_voxel((v_pos){4, 6, 5}, 3);
	set_voxel((v_pos){4, 7, 4}, 3);
}
