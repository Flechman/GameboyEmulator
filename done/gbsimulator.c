
#include "sidlib.h"
#include "lcdc.h"
#include <stdint.h>
#include <stdio.h>
#include "gameboy.h"
#include "error.h"
#include "sys/time.h"
#include "joypad.h"
// Key press bits
#define MY_KEY_UP_BIT     0x01
#define MY_KEY_DOWN_BIT   0x02
#define MY_KEY_RIGHT_BIT  0x04
#define MY_KEY_LEFT_BIT   0x08
#define MY_KEY_A_BIT      0x10
#define MY_KEY_Z_BIT      0x20
#define MY_KEY_START_BIT  0x40
#define MY_KEY_SELECT_BIT 0x80

gameboy_t gb;
struct timeval start;
struct timeval paused;

uint64_t get_time_in_GB_cyles_since(struct timeval* from);

// ======================================================================
static void set_grey(guchar* pixels, int row, int col, int width, guchar grey)
{
    const size_t i = (size_t) (3 * (row * width + col)); // 3 = RGB
    pixels[i+2] = pixels[i+1] = pixels[i] = grey;
}

// ======================================================================
static void generate_image(guchar* pixels, int height, int width)
{
    uint64_t cycles = get_time_in_GB_cyles_since(&start);
    gameboy_run_until(&gb, cycles);
    for (int x=0; x<width; ++x) {
        for (int y=0; y<height; ++y) {
            uint8_t grey=0;
            image_get_pixel(&grey, &(gb.screen.display) , x/3, y/3);
            set_grey(pixels, y, x, width, (255 - 85 * grey));
        }
    }
}

// ======================================================================
#define do_key(X) \
    do { \
        if (! (psd->key_status & MY_KEY_ ## X ##_BIT)) { \
            psd->key_status |= MY_KEY_ ## X ##_BIT; \
            puts(#X " key pressed"); \
        } \
    } while(0)

static gboolean keypress_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return joypad_key_pressed(&gb.pad, UP_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return joypad_key_pressed(&gb.pad, DOWN_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return joypad_key_pressed(&gb.pad, RIGHT_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return joypad_key_pressed(&gb.pad, LEFT_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'L':
    case 'l':
        do_key(SELECT);
        return joypad_key_pressed(&gb.pad, SELECT_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'K':
    case 'k':
        do_key(START);
        return joypad_key_pressed(&gb.pad, START_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_space:
        if(psd->timeout_id > 0) {
            gettimeofday(&paused, NULL);
        } else {
            struct timeval timeofday;
            gettimeofday(&timeofday, NULL);
            timersub(&timeofday, &paused, &paused);
            timeradd(&start, &paused, &start);
            timerclear(&paused);
        }
        return ds_simple_key_handler(keyval, data);

    case 'Z':
    case 'z':
        do_key(Z);
        return joypad_key_pressed(&gb.pad, B_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return joypad_key_pressed(&gb.pad, A_KEY) != ERR_NONE ? FALSE : TRUE;
    }

    return ds_simple_key_handler(keyval, data);
}
#undef do_key

// ======================================================================
#define do_key(X) \
    do { \
        if (psd->key_status & MY_KEY_ ## X ##_BIT) { \
          psd->key_status &= (unsigned char) ~MY_KEY_ ## X ##_BIT; \
            puts(#X " key released"); \
        } \
    } while(0)

static gboolean keyrelease_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return joypad_key_released(&gb.pad, UP_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return joypad_key_released(&gb.pad, DOWN_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return joypad_key_released(&gb.pad, RIGHT_KEY) != ERR_NONE ? FALSE : TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return joypad_key_released(&gb.pad, LEFT_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'L':
    case 'l':
        do_key(SELECT);
        return joypad_key_released(&gb.pad, SELECT_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'K':
    case 'k':
        do_key(START);
        return joypad_key_released(&gb.pad, START_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'Z':
    case 'z':
        do_key(Z);
        return joypad_key_released(&gb.pad, B_KEY) != ERR_NONE ? FALSE : TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return joypad_key_released(&gb.pad, A_KEY) != ERR_NONE ? FALSE : TRUE;
    }

    return FALSE;
}
#undef do_key

uint64_t get_time_in_GB_cyles_since(struct timeval* from){
    struct timeval current;
    if(gettimeofday(&current,NULL)==-1){return ERR_MEM; };
    
    if (timercmp(&current,from,>) !=0){
        struct timeval delta;
        timersub(&current, from,&delta); 
        return delta.tv_sec * GB_CYCLES_PER_S +  (delta.tv_usec * GB_CYCLES_PER_S) / 1000000;
    }
    return 0;
    
}
// ======================================================================
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "please provide input_file\n");
        return 1;
    }
    gettimeofday(&start, NULL);
    timerclear(&paused);

    const char* const filename = argv[1];
    M_REQUIRE_NO_ERR(gameboy_create(&gb, filename));
        sd_launch(&argc, &argv,
                  sd_init("gameboy emulateur", 3*LCD_WIDTH  , 3*LCD_HEIGHT, 40,
                          generate_image, keypress_handler, keyrelease_handler));
    
    gameboy_free(&gb);
    return 0;
}

