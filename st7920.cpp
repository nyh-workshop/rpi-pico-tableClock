#include <stdio.h>
#include "st7920.h"

#include "MicroCtrlSys.h"

ST7920::ST7920(MicroCtrlSys* inputMicroCtrlSys) {
    s = inputMicroCtrlSys;
}

void ST7920::command(uint8_t inputCommand) {
    s->command(inputCommand);
 }

void ST7920::data(uint8_t inputData) {
    s->data(inputData);
}

void ST7920::fillBitmap(uint8_t* inputBitmap) {

    for (int y = 0; y < 64; y++) {
        if (y < 32) {
            command(0x80 | y);
            command(0x80);
            for (int i = 0; i < 16; i++)
                data(inputBitmap[i + y*16]);
        }
        else {
            command(0x80 | y - 32);
            command(0x88);
            for (int i = 0; i < 16; i++)
                data(inputBitmap[i + y*16]);
        }
    }
}

void ST7920::clearScreen() {

    for (int y = 0; y < 64; y++) {
        if (y < 32) {
            command(0x80 | y);
            command(0x80);
            for (int i = 0; i < 16; i++)
                data(0x00);
        }
        else {
            command(0x80 | y - 32);
            command(0x88);
            for (int i = 0; i < 16; i++)
                data(0x00);
        }
    }
}

void ST7920::graphics(bool enable) {

    if (enable) {
        command(ST7920_BASIC);
        command(ST7920_EXTEND);
        command(ST7920_GFXMODE);
    } else {
        command(ST7920_BASIC);
    }
}

void ST7920::writeText(uint8_t col, uint8_t row, char* string)
{

    switch (row) {
        case 0:
            col |= ST7920_LINE0;
            break;
        case 1:
            col |= ST7920_LINE1;
            break;
        case 2:
            col |= ST7920_LINE2;
            break;
        case 3:
            col |= ST7920_LINE3;
            break;
        default:
            col |= ST7920_LINE0;
            break;
    }

    //sleep_us(500);
    command(col);
    //sleep_us(500);

    while (*string)
        data(*string++);
}

void ST7920::init() {

    s->init();
}
