#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/float.h"

#include "hardware/pio.h"
// Our assembled program:
#include "st7920_4bp.pio.h"

#include "pico_st7920_pio_driver.h"
#include "u8g2_hal_rpi_pico.h"
#include "st7920.h"
#include "picture.h"

const uint E_PIN = 15;
const uint RS_PIN = 14;
const uint RST_PIN = 26;
const uint D4_PIN = 6;

extern "C"
{
#include "u8g2.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"
}

const uint LED_PIN = 25;
const uint SET_PIN = 21;
const uint DOWN_PIN = 20;
const uint UP_PIN = 22;
const uint INTR_PIN = 27;

volatile uint8_t upButtonEvent = 0;
volatile uint8_t setButtonEvent = 0;
volatile uint8_t blinkSelect = 0;
volatile uint8_t setTimeCount = 0;

enum whichDigitToBlink
{
  blinkColon = 0,
  blinkHour,
  blinkMinute,
  blinkDay,
  blinkMonth,
  blinkYear,
  blinkDOTW,
  blinkNone,
};

inline void resetSetTimeCount()
{
  setTimeCount = 0;
}

inline void resetBlinkCount()
{
  setButtonEvent = 0;
  setTimeCount = 0;
  blinkSelect = blinkColon;
}

volatile uint8_t debounceCount = 0;
volatile uint32_t interruptTime = 0;
volatile uint32_t lastInterruptTime = 0;

inline void rtc_adjustForDay(datetime_t& t)
{
  if ((t.month == 1) || (t.month == 3) || (t.month == 5) || (t.month == 7) ||
      (t.month == 8) || (t.month == 10) || (t.month == 12))
  {
    if (t.day > 31)
      t.day = 1;
  }
  else
  {
    if (t.day > 30)
      t.day = 1;
  }
  if ((t.month == 2) && !(t.year % 4))
  {
    if (t.day > 29)
      t.day = 1;
  }
  if ((t.month == 2) && (t.year % 4))
  {
    if (t.day > 28)
      t.day = 1;
  }
}

void gpio_callback_gpio(uint gpio, uint32_t events)
{
  // Debouncing code from https://forum.arduino.cc/index.php?topic=45000.0:
  interruptTime = time_us_32();

  if ((interruptTime - lastInterruptTime) > 250000)
  {
    // This is using one interrupt pin and the wiring trick from https://www.gammon.com.au/forum/?id=11091
    if (!gpio_get(SET_PIN))
    {
      setButtonEvent = 1;
      resetSetTimeCount();
      blinkSelect++;
      if (blinkSelect > blinkDOTW)
      {
        resetBlinkCount();
      }
    }

    if (!gpio_get(UP_PIN))
    {
      resetSetTimeCount();
      if (setButtonEvent)
      {
        datetime_t t;
        rtc_get_datetime(&t);

        switch (blinkSelect)
        {
        case blinkHour:
          t.hour = t.hour + 1;
          if (t.hour > 23)
            t.hour = 0;
          rtc_set_datetime(&t);
          break;
        case blinkMinute:
          t.min = t.min + 1;
          if (t.min > 59)
            t.min = 0;
          rtc_set_datetime(&t);
          break;
        case blinkDay:
          t.day = t.day + 1;
          rtc_adjustForDay(t);
          rtc_set_datetime(&t);
          break;
        case blinkMonth:
          t.month = t.month + 1;
          if (t.month > 12)
            t.month = 1;
          rtc_adjustForDay(t);
          rtc_set_datetime(&t);
          break;
        case blinkYear:
          t.year = t.year + 1;
          if (t.year > 2050)
            t.year = 2020;
          rtc_adjustForDay(t);
          rtc_set_datetime(&t);
          break;
        case blinkDOTW:
          t.dotw = t.dotw + 1;
          if (t.dotw > 6)
            t.dotw = 0;
          rtc_set_datetime(&t);
          break;
        default:
          break;
        }
      }
    }
  }

  lastInterruptTime = interruptTime;
}

void drawAnalogAndDigitalClock(u8g2_t u8g2, datetime_t *inputDateTime, ST7920 *inputST7920, uint8_t *inputFrameBufferPtr, uint8_t setBlinkDigit)
{

  uint8_t x1, x2, y1, y2, x, y;
  const uint8_t CLOCK_CENTER_X = 31;
  const uint8_t CLOCK_CENTER_Y = 31;
  float angle;

  char time_str[16];
  char date_str[16];
  char day_str[4];

  const char *dayOfWeek3letter[8] = {
      "MON",
      "TUE",
      "WED",
      "THU",
      "FRI",
      "SAT",
      "SUN"};

  u8g2_ClearBuffer(&u8g2);
  u8g2_SetBitmapMode(&u8g2, 0);

  rtc_get_datetime(inputDateTime);

  uint8_t hour12 = (inputDateTime->hour > 12) ? abs(inputDateTime->hour - 12) : inputDateTime->hour;

  // Draw the outline of the clock!
  u8g2_DrawCircle(&u8g2, CLOCK_CENTER_X, CLOCK_CENTER_Y, 31, U8G2_DRAW_ALL);

  for (uint16_t idx = 0; idx < 360; idx = idx + 30)
  {
    // https://github.com/xpress-embedo/AnalogClockOLED/
    // Begin at 0 degree and stop at 360 degree
    // To convert degree into radians, we have to multiply degree by (pi/180)
    // degree (in radians) = degree * pi / 180
    // degree (in radians) = degree / (180/pi)
    // pi = 57.29577951 ( Python math.pi value )
    angle = (float)idx;
    angle = (angle / 57.29577951); //Convert degrees to radians
    x1 = (CLOCK_CENTER_X + (sin(angle) * 31));
    y1 = (CLOCK_CENTER_Y - (cos(angle) * 31));
    x2 = (CLOCK_CENTER_X + (sin(angle) * (31 - 5)));
    y2 = (CLOCK_CENTER_Y - (cos(angle) * (31 - 5)));
    u8g2_DrawLine(&u8g2, x1, y1, x2, y2);
  }
  // display minute hand
  int16_t temp = inputDateTime->min * 6u;
  angle = ((float)temp / 57.29577951); //Convert degrees to radians
  x = (CLOCK_CENTER_X + (sin(angle) * (22)));
  y = (CLOCK_CENTER_Y - (cos(angle) * (22)));
  u8g2_DrawLine(&u8g2, CLOCK_CENTER_X, CLOCK_CENTER_Y, x, y);

  // display hour hand
  angle = hour12 * 30 + ((inputDateTime->min * 6) / 12);
  angle = (angle / 57.29577951); //Convert degrees to radians
  x = (CLOCK_CENTER_X + (sin(angle) * (15)));
  y = (CLOCK_CENTER_Y - (cos(angle) * (15)));
  u8g2_DrawLine(&u8g2, CLOCK_CENTER_X, CLOCK_CENTER_Y, x, y);

  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_lubR14_tn);
  snprintf(time_str, sizeof(time_str), "%02d:%02d", inputDateTime->hour, inputDateTime->min);
  u8g2_DrawStr(&u8g2, 68, 20, time_str);

  // visible separating line:
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_DrawLine(&u8g2, 68, 24, 122, 24);

  u8g2_SetFont(&u8g2, u8g2_font_lubR08_tn);
  snprintf(date_str, sizeof(date_str), "%02d-%02d-%02d",
           inputDateTime->day, inputDateTime->month, inputDateTime->year - 2000);
  u8g2_DrawStr(&u8g2, 70, 38, date_str);

  u8g2_SetFont(&u8g2, u8g2_font_logisoso16_tr);
  snprintf(day_str, sizeof(day_str), "%s", dayOfWeek3letter[inputDateTime->dotw]);
  u8g2_DrawStr(&u8g2, 66, 58, day_str);

  u8g2_SetDrawColor(&u8g2, 0);
  switch (setBlinkDigit)
  {
  case blinkHour:
    u8g2_DrawBox(&u8g2, 68, 5, 23, 16);
    break;

  case blinkMinute:
    u8g2_DrawBox(&u8g2, 98, 5, 23, 16);
    break;

  case blinkDay:
    u8g2_DrawBox(&u8g2, 70, 28, 14, 10);
    break;

  case blinkMonth:
    u8g2_DrawBox(&u8g2, 87, 28, 15, 10);
    break;

  case blinkYear:
    u8g2_DrawBox(&u8g2, 105, 28, 15, 10);
    break;

  case blinkDOTW:
    u8g2_DrawBox(&u8g2, 66, 40, 30, 20);
    break;

  case blinkColon:
    u8g2_DrawBox(&u8g2, 93, 5, 3, 16);
    break;

  default:
    break;
  }

  inputST7920->fillBitmap(inputFrameBufferPtr);
}

extern "C" int main()
{
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(SET_PIN);
  gpio_set_dir(SET_PIN, GPIO_IN);
  gpio_pull_up(SET_PIN);

  gpio_init(UP_PIN);
  gpio_set_dir(UP_PIN, GPIO_IN);
  gpio_pull_up(UP_PIN);

  gpio_init(DOWN_PIN);
  gpio_set_dir(DOWN_PIN, GPIO_IN);
  gpio_pull_up(DOWN_PIN);

  gpio_init(INTR_PIN);
  gpio_set_dir(INTR_PIN, GPIO_IN);
  gpio_pull_up(INTR_PIN);

  gpio_set_irq_enabled_with_callback(INTR_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_gpio);

  // Example from the Pico SDK manual!
  datetime_t t = {
      .year = 2021,
      .month = 02,
      .day = 22,
      .dotw = 6,
      .hour = 21,
      .min = 00,
      .sec = 00};

  rtc_init();
  rtc_set_datetime(&t);

  u8g2_t u8g2;

  pp_st7920 pico_pio_st7920(D4_PIN, E_PIN, RS_PIN, RST_PIN);
  ST7920 st7920(&pico_pio_st7920);

  u8g2_Setup_st7920_p_128x64_f(&u8g2, U8G2_R0, cb_byte_pio_hw, cb_gpio_delay_rpi);

  u8g2_InitDisplay(&u8g2);

  // Getting the frame buffer pointer to manually fill screen.
  uint8_t *frameBufferPtr = u8g2_GetBufferPtr(&u8g2);

  st7920.init();
  st7920.command(0x01);
  st7920.graphics(true);
  st7920.clearScreen();

  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFontMode(&u8g2, 0);
  u8g2_SetDrawColor(&u8g2, 1);

  st7920.fillBitmap((uint8_t *)frameBufferPtr);
  u8g2_SetFont(&u8g2, bdf_font);

  while (1)
  {
    // gpio_put(LED_PIN, 1);
    // sleep_ms(500);
    // gpio_put(LED_PIN, 0);
    // sleep_ms(500);

    if (setButtonEvent)
    {
      while (setTimeCount < 5)
      {
        drawAnalogAndDigitalClock(u8g2, &t, &st7920, frameBufferPtr, blinkSelect);
        sleep_ms(500);
        drawAnalogAndDigitalClock(u8g2, &t, &st7920, frameBufferPtr, blinkNone);
        sleep_ms(500);
        setTimeCount++;
      }
      resetBlinkCount();
    }
    else
    {
      drawAnalogAndDigitalClock(u8g2, &t, &st7920, frameBufferPtr, blinkColon);
      sleep_ms(500);
      drawAnalogAndDigitalClock(u8g2, &t, &st7920, frameBufferPtr, blinkNone);
      sleep_ms(500);
    }
  }

  return 0;
}
