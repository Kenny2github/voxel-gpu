`timescale 1 ps / 1 ps

typedef struct {
  logic [31:0] x;
  logic [31:0] y;
  logic [31:0] z;
} vec3;

typedef struct {
  vec3 pos;
  vec3 look0;
  vec3 look1;
  vec3 look2;
  vec3 look3;
} camera;
