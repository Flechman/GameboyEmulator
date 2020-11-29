#pragma once

#include "cpu.h"
#include "component.h"
#include "memory.h"
#include "bit.h"
#include "image.h"

typedef struct gameboy_ gameboy_t;

#ifdef __cplusplus
extern "C" {
#endif

// LCDC registers

#define REG_LCDC 0xFF40
#define REG_STAT 0xFF41
#define REG_SCY  0xFF42
#define REG_SCX  0xFF43
#define REG_LY   0xFF44
#define REG_LYC  0xFF45
#define REG_DMA  0xFF46
#define REG_BGP  0xFF47
#define REG_OBP0 0xFF48
#define REG_OBP1 0xFF49
#define REG_WY   0xFF4A
#define REG_WX   0xFF4B

// Misc constants

#define LCD_WIDTH  160
#define LCD_HEIGHT 144

#define VBLANK_LINES 10

#define LINE_MODE_2_CYCLES 20
#define LINE_MODE_3_CYCLES 43
#define LINE_MODE_0_CYCLES 51

#define LINE_MODE_2_START_CYCLE   0
#define LINE_MODE_3_START_CYCLE  (LINE_MODE_2_START_CYCLE + LINE_MODE_2_CYCLES)
#define LINE_MODE_0_START_CYCLE  (LINE_MODE_3_START_CYCLE + LINE_MODE_3_CYCLES)

#define LINE_TOTAL_CYCLES (LINE_MODE_2_CYCLES + LINE_MODE_3_CYCLES + LINE_MODE_0_CYCLES)

// This should be 17556
#define FRAME_TOTAL_CYCLES ((LCD_HEIGHT + VBLANK_LINES) * LINE_TOTAL_CYCLES)


// LCDC register bits

#define LCDC_REG_BG_MASK          0x01
#define LCDC_REG_OBJ_MASK         0x02
#define LCDC_REG_OBJ_SIZE_MASK    0x04
#define LCDC_REG_BG_AREA_MASK     0x08
#define LCDC_REG_TILE_SOURCE_MASK 0x10
#define LCDC_REG_WIN_MASK         0x20
#define LCDC_REG_WIN_AREA_MASK    0x40
#define LCDC_REG_LCD_STATUS_MASK  0x80


// STAT register

#define STAT_REG_MODE_MASK 0x03

#define STAT_REG_LYC_EQ_LY_BIT 2
#define STAT_REG_INT_LYC_BIT   6


// Tiles

#define TILE_ADDR_BASE_LOW  0x9800
#define TILE_ADDR_BASE_HIGH 0x9C00

#define TILE_SRC_ADDR_LOW  0x8000
#define TILE_SRC_ADDR_HIGH 0x8800

#define TILE_SIZE 16      // tile size (in bytes)

#define TILE_LINE_SIZE    32
#define VISIBLE_LINE_SIZE 20


// Window

#define WINDOW_OFFSET_X  7

// ======================================================================
/**
 * @brief lcdc type
 */
typedef struct {
    cpu_t* cpu;
    bit_t on;
    uint64_t next_cycle;
    uint64_t on_cycle;
    addr_t   DMA_from;
    addr_t   DMA_to;
    image_t  display;
    data_t   window_y;
} lcdc_t;


/**
 * @brief Initiates a LCD controler
 *
 * @param gb  Game Boy, the screen of which has to be initalized
 * @return error code
 */
int lcdc_init(gameboy_t* gb);


/**
 * @brief Frees a LCD controler
 * @param lcd LCD controler to free
 */
void lcdc_free(lcdc_t* lcd);


/**
 * @brief Plugs a LCD controler onto the bus
 * @param lcd LCD controler to plug
 * @param bus bus to plug onto
 * @return error code
 */
int lcdc_plug(lcdc_t* lcd, bus_t bus);


/**
 * @brief Run one LCD controler cycle
 *
 * @param lcd LCD controler to cycle
 * @param cycle the current cycle number
 * @return error code
 */
int lcdc_cycle(lcdc_t* lcd, uint64_t cycle);


/**
 * @brief LCD controler bus listening handler
 *
 * @param lcd LCD controler
 * @param address trigger address
 * @return error code
 */
int lcdc_bus_listener(lcdc_t* lcd, addr_t addr);

#ifdef __cplusplus
}
#endif
