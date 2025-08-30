/**
 * SH1106 OLED display single-file library
 */
#ifndef _SH1106_H
#define _SH1106_H

#if defined(SH1106_SPI)
#include "ssd1306_spi.h"
#define sh1106_comm_init ssd1306_spi_init
 #elif defined(SH1106_I2C_BITBANG)
#include "ssd1306_i2c_bitbang.h"
#define sh1106_comm_init ssd1306_i2c_setup
#else
#include "ssd1306_i2c.h"
#define sh1106_comm_init ssd1306_i2c_init
#endif

#ifndef SH1106_WIDTH
#define SH1106_WIDTH  128
#endif

#ifndef SH1106_HEIGHT
#define SH1106_HEIGHT 64
#endif

// ssd1306_pkt_send max available buffer
#define SH1106_MAXBUF 32

// Taken from the Adafruit SH110X Arduino library
static const uint8_t sh1106_init_commands[] = {
  0xAE,       // SH110X_DISPLAYOFF
  0xD5, 0x80, // SH110X_SETDISPLAYCLOCKDIV, 0x80,
  0xA8, 0x3F, // SH110X_SETMULTIPLEX, 0x3F,
  0xD3, 0x00, // SH110X_SETDISPLAYOFFSET, 0x00,
  0x40,       // SH110X_SETSTARTLINE
  0xAD, 0x8B, // SH110X_DCDC (DC/DC on)
  0xA1,       // SH110X_SEGREMAP + 1
  0xC8,       // SH110X_COMSCANDEC
  0xDA, 0x12, // SH110X_SETCOMPINS, 0x12,
  0x81, 0xFF, // SH110X_SETCONTRAST, 0xFF
  0xD9, 0x1F, // SH110X_SETPRECHARGE, 0x1F,
  0xDB, 0x40, // SH110X_SETVCOMDETECT, 0x40,
  0x33,       // Set VPP to 9V
  0xA6,       // SH110X_NORMALDISPLAY
  0x20, 0x10, // SH110X_MEMORYMODE, 0x10 (could not find this one in the datasheet)
  0xA4,       // SH110X_DISPLAYALLON_RESUME
};

// Internal 1:1 framebuffer of display mem
uint8_t sh1106_buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

// -------------------------------------------------------------------------------------------------

// Send OLED command byte
uint8_t sh1106_cmd(uint8_t cmd)
{
	return ssd1306_pkt_send(&cmd, 1, 1);
}

// Clear the screen buffer
void sh1106_clrbuf(void)
{
  memset(sh1106_buffer, 0, sizeof(sh1106_buffer));
}

// Set pixel
void sh1106_set_pixel(uint8_t x, uint8_t y)
{
  if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

  sh1106_buffer[x + (SH1106_WIDTH * (y >> 3))] |= (1 << (y % 8));
}

// Invert colors inside a selection rectangle
void sh1106_xor_rect(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y)
{
  if (start_x > end_x) { uint8_t tmp = start_x; start_x = end_x; end_x = tmp; }
  if (start_y > end_y) { uint8_t tmp = start_y; start_y = end_y; end_y = tmp; }

  for (uint8_t y = start_y; y <= end_y && y < SH1106_HEIGHT; y++)
  {
    uint16_t offset = SH1106_WIDTH * (y >> 3);
    uint8_t bit_mask = (1 << (y % 8));

    for (uint8_t x = start_x; x <= end_x && x < SH1106_WIDTH; x++)
    {
      sh1106_buffer[offset + x] ^= bit_mask;
    }
  }
}

// Send the frame buffer
void sh1106_refresh(void)
{
  uint8_t *ptr = sh1106_buffer;
  for (uint8_t page = 0; page < (SH1106_HEIGHT / 8); page++)
	{
		sh1106_cmd(0xB0 | page); // SH1103_SETPAGEADDR
    // The SH1106 display requires a small offset into memory
		sh1106_cmd(0x02); // SH1103_SETLOWCOLUMN + 2
		sh1106_cmd(0x10); // SH1103_SETHIGHCOLUMN

    for (uint8_t i = 0; i < (SH1106_WIDTH / SH1106_MAXBUF); i++)
    {
      ssd1306_pkt_send(ptr, SH1106_MAXBUF, 0);
      ptr += SH1106_MAXBUF;
    }
	}
}

// Module initialization
void sh1106_init()
{
  sh1106_comm_init();
  // pulse reset
  ssd1306_rst();
  for (uint8_t i = 0; i < sizeof(sh1106_init_commands); i++)
  {
    sh1106_cmd(sh1106_init_commands[i]);
  }
  Delay_Ms(100); // 100ms delay recommended
  sh1106_cmd(0xAF); // SH1103_DISPLAYON
}

#endif
