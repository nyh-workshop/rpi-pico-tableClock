#ifndef PICO_ST7920_PIO_DRIVER_H
#define PICO_ST7920_PIO_DRIVER_H

#include "MicroCtrlSys.h"

class pp_st7920 : public MicroCtrlSys {
public:
    pp_st7920(uint D4_PIN, uint E_PIN, uint RS_PIN, uint RST_PIN);
    void command(uint32_t inputCommand);
    void data(uint32_t inputData);
    void init();

protected:
    uint D4_PIN_;
    uint E_PIN_;
    uint RS_PIN_;
    uint RST_PIN_;
};

#endif
