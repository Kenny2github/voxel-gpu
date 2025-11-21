package gpu;

  typedef struct {
    logic signed [31:0] x;
    logic signed [31:0] y;
    logic signed [31:0] z;
  } vec3;

  typedef struct {
    vec3 pos;
    vec3 look0;
    vec3 look1;
    vec3 look2;
    vec3 look3;
  } camera;

endpackage
