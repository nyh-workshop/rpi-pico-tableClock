#include "MicroCtrlSys.h"
#include "pico_st7920_pio_driver.h"
#include "st7920.h"
// Our assembled program:
#include "st7920_4bp.pio.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"

pp_st7920::pp_st7920(uint D4_PIN, uint E_PIN, uint RS_PIN, uint RST_PIN) {
    D4_PIN_ = D4_PIN;
    E_PIN_ = E_PIN;
    RS_PIN_ = RS_PIN;
    RST_PIN_ = RST_PIN;
    init();
}

void pp_st7920::command(uint32_t inputCommand) {
    sleep_us(500);
    gpio_put(RS_PIN_, 0);
    pio_sm_put_blocking(pio0, 0, inputCommand);
    sleep_us(500);
}

void pp_st7920::data(uint32_t inputData) {
    gpio_put(RS_PIN_, 1);
    pio_sm_put_blocking(pio0, 0, inputData);
}

void pp_st7920::init() {

    gpio_init(RST_PIN_);
    gpio_set_dir(RST_PIN_, GPIO_OUT);

    gpio_init(RS_PIN_);
    gpio_set_dir(RS_PIN_, GPIO_OUT);
    gpio_put(RS_PIN_, 0);

    // Choose which PIO instance to use (there are two instances)
    PIO pio = pio0;

    // Our assembled program needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint offset = pio_add_program(pio, &st7920_4bp_program);

    st7920_4bp_program_init(pio, 0, offset, E_PIN_, D4_PIN_);
    gpio_put(RST_PIN_, 1);
    sleep_ms(100);
    gpio_put(RST_PIN_, 0);

    sleep_ms(50);

    command(0b00000010);// 4-bit mode.
    sleep_ms(10);
	command(0b00000010);// 4-bit mode.
    sleep_ms(10);

    command(ST7920_DISPLAYON);// Cursor and blinking cursor disabled.
    sleep_ms(1);
    command(ST7920_CLS);// Clears screen.
    sleep_ms(10);
    command(ST7920_ADDRINC);// Cursor moves right, no display shift. 
    command(ST7920_HOME);// Returns to home. Cursor moves to starting point.
    sleep_ms(20);
}