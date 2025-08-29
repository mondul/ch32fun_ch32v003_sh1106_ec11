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
#include "my_txt.h"

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
// ISRs

// -----------------------------------------------
// PA0 change interrupt service routine
void EXTI0_IRQHandler(void) __attribute__((interrupt));
void EXTI0_IRQHandler(void)
{
  // Clear flag (reference manual says write 1 to clear it)
  EXTI->INTFR |= EXTI_INTF_INTF0;

  funDigitalWrite(PB2, funDigitalRead(PB2) ? FUN_LOW : FUN_HIGH);
}

// -------------------------------------------------------------------------------------------------

int main()
{
  SystemInit();
  funGpioInitAll(); // Enable GPIOs

  funPinMode( PA0, GPIO_CFGLR_IN_PUPD ); // Set PA0 to input (board's KEY pushbutton)
  funPinMode( PA6, GPIO_CFGLR_IN_FLOAT ); // Set PA6 to floating input
  funPinMode( PA7, GPIO_CFGLR_IN_FLOAT ); // Set PA7 to floating input

  funPinMode( PB2, GPIO_CFGLR_OUT_10Mhz_PP ); // Set PB2 to output
  funPinMode( PB12, GPIO_CFGLR_OUT_10Mhz_PP ); // Set PB12 to output

  // Relay is active low
  funDigitalWrite(PB12, FUN_HIGH);

  // Enable PA0 change interrupt on rising edge
  EXTI->INTENR |= EXTI_INTENR_MR0;
  EXTI->RTENR |= EXTI_RTENR_TR0;
  NVIC_EnableIRQ(EXTI0_IRQn);

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

  sh1106_clrbuf();

  // uint16_t initial_count = TIM3->CNT;
  uint16_t last_count = TIM3->CNT;

  uint8_t y = 2;
  my_txt(0, y, "Hello");
  y += char_rows;
  my_txt(0, y, "World!");
  y += char_rows;
  my_txt(0, y, "_{y}_ fj yy");
  y += char_rows;
  my_txt(0, y, "Lorem ipsum");
  y += char_rows;
  my_txt(0, y, "dolor sit amet");
  sh1106_refresh();

  for (;;)
  {
    Delay_Ms(50);
    uint16_t count = TIM3->CNT;
    if( count != last_count)
    {
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
