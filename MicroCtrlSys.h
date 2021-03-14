#ifndef MICROCTRLSYS_H
#define MICROCTRLSYS_H

#include <iostream>
#include <string>

class MicroCtrlSys {
public:
    MicroCtrlSys() {}
    virtual void init();
    virtual void data(uint32_t inputData);
    virtual void command(uint32_t inputCommand);
private:
};

#endif
