#ifdef AVR
#undef AVR
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "src/hal/avr/mcus/mega328/Mega328.h"
#include "src/abstraction/uart/AbstractUart.h"
#include "src/software/uart/SoftwareUart.h"
#include "src/software/uart/TimerSoftwareUart.h"
#include "src/software/uart/AssemblerSoftwareUart.h"
#include "src/software/uart/external/Sync.h"


using mcu = lib::avr::ATMega328;

int main() {
    using namespace lib::software::literals;
    using uart = lib::software::Uart<mcu>;
    using softUart = lib::software::SoftwareUart<mcu, lib::software::SoftUartMethod::Timer>;

    uart::init<19200_baud>();
    char buffer [33];

    uart::sendData("abc");
    while(true) {
        uart::sendChar((unsigned char)syncAndReciveByte());
    }

    softUart::init<0, 9600_baud, 19200_baud>();
    while(softUart::receiveData() != softUart::preamble) {}

    while(true) {
        auto data = softUart::receiveData();
        uart::sendChar(data);
    }
}