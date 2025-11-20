#define RESOLUTION_X 320
#define RESOLUTION_Y 240

#define CENTER_X 160
#define CENTER_Y 120

#define CHAR_COL 80
#define CHAR_ROW 60

#define CENTER_COL 40
#define CENTER_ROW 30

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        (volatile long*) 0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   (volatile int*)0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define PS2_BASE              0xFF200100

#define PS2_IRQ               79
  
#define ICCEOIR                (volatile int*)0xFFFEC110
#define ICCIAR                 (volatile int*)0xFFFEC10C

#define BOX_SIZE 5
#define CANVAS_SIZE 28

#define BORDER_LEFT 16
#define BORDER_RIGHT 302

#define BORDER_TOP 16
#define BORDER_BOTTOM 222

#define SENSITIVITY 0.20

typedef struct {
    int x;
    int y;
} Position;

typedef enum {
    true = 1,
    false = 0
} bool;

void plot_pixel(int x, int y, short int line_color);
void drawCursor(int mx, int my);
void wait_for_vsync();
void clear_screen();
void mouseInput();
void config_GIC(void);
void config_PS2(void);
void set_A9_IRQ_stack(void);
void enable_A9_interrupts(void);
void config_interrupt(int N, int CPU_target);
void __attribute__((interrupt)) __cs3_isr_irq(void);
void __attribute__((interrupt)) __cs3_reset(void);
void __attribute__((interrupt)) __cs3_isr_undef(void);
void __attribute__((interrupt)) __cs3_isr_swi(void);
void __attribute__((interrupt)) __cs3_isr_pabort(void);

/************************************************************************************
*                                                                                   *
*   Global Variables                                                                *
*                                                                                   *
*************************************************************************************/

short int cursor[9][9] = {{0x0,0x0,0x0,0x0,0x0,0x0,0xf000,0xf000,0x0},{0x0,0x0,0x0,0x0,0x0,0xf000,0xf000,0xf000,0xf000},{0x0,0x0,0x0,0x0,0xf000,0x0,0xf000,0xf000,0xf000},{0x0,0x0,0x0,0xf000,0x0,0x0,0x0,0xf000,0x0},{0x0,0x0,0xf000,0x0,0x0,0x0,0xf000,0x0,0x0},{0x0,0xf000,0x0,0x0,0x0,0xf000,0x0,0x0,0x0},{0xf000,0x0,0x0,0x0,0xf000,0x0,0x0,0x0,0x0},{0xf000,0xf000,0x0,0xf000,0x0,0x0,0x0,0x0,0x0},{0xf000,0xf000,0xf000,0x0,0x0,0x0,0x0,0x0,0x0}};

unsigned char seg[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

// To keep track of mouse movements and events
unsigned char mousePackets[3] = {0, 0, 0}; // click = 0, x = 1, y = 2
Position mouse = { BORDER_LEFT, BORDER_TOP+9};
bool leftClick = false;

// To store every handle event to process 
int handleNum[3] = {-1, -1, -1}; 
int numOfHandles = 0;

bool leftHold = false;


/************************************************************************************
*                                                                                   *
*   Handle Event Rendering                                                          *
*                                                                                   *
*************************************************************************************/

/* Global Handle */
void noHandle() {
    return;
}

/************************************************************************************
*                                                                                   *
*   Mouse Input Functions                                                           *
*                                                                                   *
*************************************************************************************/

volatile int pixel_buffer_start;

typedef struct {
    int ySize;
    int xSize;
} Size;

void wait_for_vsync() {
	volatile int* buffer = PIXEL_BUF_CTRL_BASE;
    
	*buffer = 1;
	register int status = *(buffer+3);
	
	while((status & 0x01) != 0)
		status = *(buffer+3);
}

void plot_pixel(int x, int y, short int line_color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen() {
	for(int x = BORDER_LEFT; x < BORDER_RIGHT; x++) 
		for(int y = BORDER_TOP; y < BORDER_BOTTOM; y++)
			plot_pixel(x, y, 0x0);	
}

void drawCursor(int mx, int my) {
    Size cursorSize = {sizeof(cursor) / sizeof(cursor[0]), sizeof(cursor[0]) / sizeof(cursor[0][0])};
    for(int y = 0; y < cursorSize.ySize; y++)
        for(int x = 0; x < cursorSize.xSize; x++)
            if(cursor[y][x] != 0x0)
                plot_pixel(mx + x, my - cursorSize.ySize + y, cursor[y][x]);
}

void removeCursor(Position mousePosition) {
    Size cursorSize = {sizeof(cursor) / sizeof(cursor[0]), sizeof(cursor[0]) / sizeof(cursor[0][0])};
    for(int y = 0; y < cursorSize.ySize; y++)
        for(int x = 0; x < cursorSize.xSize; x++)
            if(cursor[y][x] != 0x0)
                plot_pixel(mousePosition.x + x, mousePosition.y - cursorSize.ySize + y, 0x0);
}


typedef enum {
    DEFAULT,
    WAIT_ACKNOWLEDGE,
    REPORTING,
} Status;
Status currentStatus = DEFAULT;

void mouseInput() {
    volatile int * PS2_ptr = (int *) PS2_BASE;
    int PS2_data, RVALID;

    int numOfBytes = 0;

    *(PS2_ptr) = 0xFF;

    while (numOfBytes < 3) {
        PS2_data = *(PS2_ptr);
        RVALID = (PS2_data & 0x8000);

        if (RVALID) {

            mousePackets[0] = mousePackets[1];
            mousePackets[1] = mousePackets[2];
            mousePackets[2] = PS2_data & 0xFF;

            if(currentStatus == REPORTING)
                numOfBytes++;


            if(currentStatus == DEFAULT && mousePackets[1] == (unsigned char)0xAA && mousePackets[2] == (unsigned char)0x00) {
                currentStatus = WAIT_ACKNOWLEDGE;
                *(PS2_ptr) = 0xF4;
            } 

            if(currentStatus == WAIT_ACKNOWLEDGE && mousePackets[2] == 0xFA) {
                currentStatus = REPORTING;
                continue;
            }
        }

    }

    struct {
        signed int x : 9;
        signed int y : 9;
    } signedPos;

    signedPos.x = ((mousePackets[0] & 0b10000) << 4) | (mousePackets[1]);
    signedPos.y = ((mousePackets[0] & 0b100000) << 3) | (mousePackets[2]);


    mouse.x += signedPos.x * SENSITIVITY;
    mouse.y -= signedPos.y * SENSITIVITY;
    
    if(mouse.x > BORDER_RIGHT - 9)
        mouse.x = BORDER_RIGHT - 9;
    if(mouse.y > BORDER_BOTTOM)
        mouse.y = BORDER_BOTTOM;

    if(mouse.x < BORDER_LEFT)
        mouse.x = BORDER_LEFT;
    if(mouse.y < BORDER_TOP + 9)
        mouse.y = BORDER_TOP + 9;

    leftClick = mousePackets[0] & 0b1;

    if(leftClick != 1)
        leftHold = false;
}


/************************************************************************************
*                                                                                   *
*   Interrupt Configurations                                                        *
*                                                                                   *
*************************************************************************************/

void config_GIC(void) {
    config_interrupt(79, 0); // configure the FPGA PS2 interrupt (79)
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
    // priorities
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of
    // interrupts
    *((int *) 0xFFFEC100) = 1;
    // Configure the Distributor Control Register (ICDDCR) to send pending
    // interrupts to CPUs
    *((int *) 0xFFFED000) = 1;
}

void config_PS2(void) {
    volatile int* ptr = (volatile int*)0xFF200100;
    *(ptr + 0x1) = 0x1;
}

void set_A9_IRQ_stack(void) {
    int stack, mode;
    stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r"(stack));
    /* go back to SVC mode before executing subroutine return! */
    mode =  0b11010011;

    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
}

void enable_A9_interrupts(void) {
    int status =  0b01010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

void disable_A9_interrupts(void) {
    int status = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

void config_interrupt(int N, int CPU_target) {
    int reg_offset, index, value, address;
    /* Configure the Interrupt Set-Enable Registers (ICDISERn).
    * reg_offset = (integer_div(N / 32) * 4
    * value = 1 << (N mod 32) */
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    /* Now that we know the register address and value, set the appropriate bit */
    *(int *)address |= value;
    /* Configure the Interrupt Processor Targets Register (ICDIPTRn)
    * reg_offset = integer_div(N / 4) * 4
    * index = N mod 4 */
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    /* Now that we know the register address and value, write to (only) the
    * appropriate byte */
    *(char *)address = (char)(1 << CPU_target);
}


/************************************************************************************
*                                                                                   *
*   Interrupt Handler FUnctions                                                     *
*                                                                                   *
*************************************************************************************/
volatile int key_pressed = 4;
 void pushbutton_ISR( void )
 {
    volatile int * KEY_ptr = (int *) 0xFF200050;
    int press;
    press = *(KEY_ptr + 3);
    *(KEY_ptr + 3) = press;
    if (press & 0x1)
        key_pressed = 0;
    else if (press & 0x2)
        key_pressed = 1;
    else if (press & 0x4)
        key_pressed = 2;
    else
        key_pressed = 3;
    return;
 }


// Define the IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void)
{
    // Read the ICCIAR from the processor interface
    volatile int* address = ICCIAR;
    int int_ID = *(address);

    if (int_ID == PS2_IRQ) // check if interrupt is from the HPS timer
        mouseInput();
    else
        while (1); // if unexpected, then stay here

    // Write to the End of Interrupt Register (ICCEOIR)
    address = (volatile int*)ICCEOIR;
    *address = int_ID;
    return;
}

// Define the remaining exception handlers
void __attribute__((interrupt)) __cs3_reset(void)
{
while (1);
}
void __attribute__((interrupt)) __cs3_isr_undef(void)
{
while (1);
}
void __attribute__((interrupt)) __cs3_isr_swi(void)
{
while (1);
}
void __attribute__((interrupt)) __cs3_isr_pabort(void)
{
while (1);
}
void __attribute__((interrupt)) __cs3_isr_dabort(void)
{
while (1);
}
void __attribute__((interrupt)) __cs3_isr_fiq(void)
{
while (1);
}


/************************************************************************************
*                                                                                   *
*   Main Function                                                                   *
*                                                                                   *
*************************************************************************************/


int main() {
    disable_A9_interrupts();
    set_A9_IRQ_stack();
    config_GIC();
    config_PS2();
    enable_A9_interrupts();

    /* Set up buffers */
    volatile int * pixel_ctrl_ptr = (int*) PIXEL_BUF_CTRL_BASE;

    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE; 
    wait_for_vsync();
    
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    // Previous mouse position of first buffer (FPGA_ONCHIP_BASE)
    Position mousePrevOne = {mouse.x, mouse.y};

    // Previous mouse position of second buffer (SDRAM_BASE)
    Position mousePrevTwo = {mouse.x, mouse.y};

    Position* mousePrevCurrent;

    if(pixel_buffer_start == FPGA_ONCHIP_BASE)
        mousePrevCurrent = &mousePrevOne;
    else if (pixel_buffer_start == SDRAM_BASE)
        mousePrevCurrent = &mousePrevTwo;
    else {
        exit(1);
    }

    while (1)
    {   

        removeCursor(*mousePrevCurrent);
        int mouseXshot = mouse.x, mouseYshot = mouse.y;
        drawCursor(mouseXshot, mouseYshot);

        // Update previous mouse positions;
        (*mousePrevCurrent).x = mouseXshot;
        (*mousePrevCurrent).y = mouseYshot;


        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

        if(pixel_buffer_start == FPGA_ONCHIP_BASE)
            mousePrevCurrent = &mousePrevOne;
        else if (pixel_buffer_start == SDRAM_BASE)
            mousePrevCurrent = &mousePrevTwo;
        else {
            exit(1);
        }
    }
}