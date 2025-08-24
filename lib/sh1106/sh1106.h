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

static const uint8_t sh1106_init_commands[] = {
  // --- Power-up safe: display off ---
  0xAE, // Display OFF                        

  // --- Display clock / oscillator ---
  0xD5, // Set display clock divide / osc freq
  0x80, // Recommended default (or 0xF0 if you prefer)

  // --- Multiplex ratio (height - 1) ---
  0xA8, // Set multiplex
  0x3F, // 0x3F = 63 -> 64MUX for 64 rows

  // --- Display offset & start line ---
  0xD3, // Set display offset
  0x00, // 0 offset
  0x40, // Set display start line = 0

  // --- Enable internal DC-DC (charge pump) [SH1106-specific] ---
  0xAD, // DC-DC control mode
  0x8B, // Built-in DC-DC ON (0x8A = OFF if VPP external)

  // --- Orientation ---
  0xA1, // Segment remap: mirror X (A0 normal, A1 mirror X)
  0xC8, // COM scan dir: mirror Y  (C0 normal, C8 mirror Y)

  // --- COM pins hardware config ---
  0xDA, // Set COM pins
  0x12, // Alt COM, no L/R remap (typical for 128x64)

  // --- Analog settings: contrast, precharge, VCOMH ---
  0x81, // Contrast
  0x80, // Tweak 0x00..0xFF as desired
  0xD9, // Pre-charge period
  0x22, // Common default (many panels also use 0xF1)
  0xDB, // VCOMH deselect level
  0x20, // ~0.77*Vcc (0x20..0x40 typical)

  // --- Follow RAM, normal (non-inverted) ---
  0xA4, // Entire display ON = follow RAM
  0xA6, // Normal display (A7 = inverted)
};

// Internal 1:1 framebuffer of display mem
uint8_t sh1106_buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

// -------------------------------------------------------------------------------------------------

// Send OLED command byte
uint8_t sh1106_cmd(uint8_t cmd)
{
	return ssd1306_pkt_send(&cmd, 1, 1);
}

// Send the frame buffer
void sh1106_refresh(void)
{
  uint8_t page = SH1106_HEIGHT / 8;
  uint8_t *buf = sh1106_buffer;
  // Needed to fill pages from the last one due flipped screen
  do
	{
    page--;
		sh1106_cmd(0xB0 | page); // SH1103_SETPAGEADDR
    // The SH1106 display requires a small offset into memory
		sh1106_cmd(0x02); // SH1103_SETLOWCOLUMN + 2
		sh1106_cmd(0x10); // SH1103_SETHIGHCOLUMN

    for (uint8_t i = 0; i < (SH1106_WIDTH / SH1106_MAXBUF); i++)
    {
      ssd1306_pkt_send(buf, SH1106_MAXBUF, 0);
      buf += SH1106_MAXBUF;
    }
	}
  while (page);
}

// Module initialization
void sh1106_init()
{
  sh1106_comm_init();
  // pulse reset
  ssd1306_rst();
  ssd1306_pkt_send(sh1106_init_commands, sizeof(sh1106_init_commands), 1);
  Delay_Ms(100); // 100ms delay recommended
  sh1106_cmd(0xAF); // SH1103_DISPLAYON
  // If I don't do this it keeps it as normal
  sh1106_cmd(0xA1); // Segment remap: mirror X (A0 normal, A1 mirror X)
}

#endif
