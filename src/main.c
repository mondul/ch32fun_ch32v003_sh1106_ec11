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
#include <stdlib.h>
#include "tiny_talk_font.h"

// -------------------------------------------------------------------------------------------------
// ISRs

// Display refresh flag
volatile uint8_t refresh = 0;

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

void show_menu(uint8_t selected_option)
{
  sh1106_clrbuf();

  my_set_font(&tiny_talk_font);
  my_txt(80, 0, "May 26");
  my_txt(111, 0, "12:43");

  my_set_font(NULL); // Set default font
  uint8_t y = 4;
  my_txt(1, y, "Hello");
  y += my_font->char_rows;
  my_txt(1, y, "World!");
  y += my_font->char_rows;
  my_txt(1, y, "_{y}_ fj yy");
  y += my_font->char_rows;
  my_txt(1, y, "Lorem ipsum");
  y += my_font->char_rows;
  my_txt(1, y, "dolor sit amet");

  uint8_t start_y = 4 + (selected_option * my_font->char_rows);

  sh1106_xor_rect(0, start_y, (SH1106_WIDTH - 1), start_y + my_font->char_rows);
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

  // Capture module stuff for the encoder

  // Enable TIM3
  RCC->APB1PCENR |= RCC_APB1Periph_TIM3;

  // Reset TIM3 to init all regs
  RCC->APB1PRSTR |= RCC_APB1Periph_TIM3;
  RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM3;

  // set TIM3 clock prescaler If you want to reduce the resolution of the encoder
  // TIM3->PSC = 0;

  // SMCFGR: set encoder mode SMS=011b
  TIM3->SMCFGR |= TIM_EncoderMode_TI12;

  // initialize timer
  TIM3->SWEVGR |= TIM_UG;

  // set count to about mid-scale to avoid wrap-around
  TIM3->CNT = 0x8FFF;

  // Enable TIM3
  TIM3->CTLR1 |= TIM_CEN;

  uint16_t last_count = TIM3->CNT;

  // OLED stuff

  int8_t menu_pos = 0;

  sh1106_init();
  show_menu(menu_pos);
  sh1106_refresh();

  for (;;)
  {
    Delay_Ms(50);
    uint16_t count = TIM3->CNT;
    if (count != last_count)
    {
      int16_t delta = count - last_count;
      if (abs(delta) > 3) // Debounce filter
      {
        if (delta > 0)
        {
          menu_pos++;
          if (menu_pos == 5) menu_pos = 0;
        }
        else
        {
          menu_pos--;
          if (menu_pos == -1) menu_pos = 4;
        }
        show_menu(menu_pos);
        refresh = 1;
        last_count = count;
      }
    }

    if (refresh)
    {
      sh1106_refresh();
      refresh = 0;
    }
  }
}
