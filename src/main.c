/**
 * SH1106 / EC11 with CH32V203 using ch32fun
 *
 * Board connections:
 *
 * PB6: OLED's SCL
 * PB7: OLED's SDA
 * PA6: Encoder's TRA
 * PA7: Encoder's TRB
 */
#include "ch32fun.h"
#include <stdio.h>
#include <sh1106.h>

// -------------------------------------------------------------------------------------------------

uint8_t jiggle[] = { // /\/
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
};
uint8_t *jigglepos = jiggle;
const uint8_t *endjiggle = jiggle + 16;

void update_jiggle(int8_t delta)
{
  jigglepos += delta;

  if (jigglepos < jiggle) jigglepos = (uint8_t*)endjiggle + delta;
  else if (jigglepos >= endjiggle) jigglepos -= 16;

  for (uint8_t i = 0; i < (sizeof(sh1106_buffer) / 16); i++)
    memcpy(sh1106_buffer + (i * 16), jigglepos, 16);

  sh1106_refresh();
}

// -------------------------------------------------------------------------------------------------

int main()
{
  SystemInit();
  funGpioInitAll(); // Enable GPIOs

  funPinMode( PA0, GPIO_CFGLR_IN_PUPD ); // Set PA0 to input (board's KEY pushbutton)
  funPinMode( PA6, GPIO_CFGLR_IN_FLOAT ); // Set PA6 to floating input
  funPinMode( PA7, GPIO_CFGLR_IN_FLOAT ); // Set PA7 to floating input

  // Enable TIM3
  RCC->APB1PCENR |= RCC_APB1Periph_TIM3;

  // Reset TIM3 to init all regs
  RCC->APB1PRSTR |= RCC_APB1Periph_TIM3;
  RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM3;

  // SMCFGR: set encoder mode SMS=011b
  TIM3->SMCFGR |= TIM_EncoderMode_TI12;

  // initialize timer
  TIM3->SWEVGR |= TIM_UG;

  // set count to about mid-scale to avoid wrap-around
  TIM3->CNT = 0x8FFF;

  // Enable TIM3
  TIM3->CTLR1 |= TIM_CEN;

  sh1106_init();

  update_jiggle(0);

  // uint16_t initial_count = TIM3->CNT;
  uint16_t last_count = TIM3->CNT;

  for (;;)
  {
    Delay_Ms(50);
    uint16_t count = TIM3->CNT;
    if( count != last_count) {
      update_jiggle(count - last_count);
      /*
      printf(
        "Position relative=%ld absolute=%u delta=%ld\n",
        (int32_t)count - initial_count,
        count,
        (int32_t)count-last_count
      );
      */
      last_count = count;
    }
  }
}
