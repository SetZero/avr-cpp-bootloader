//
// Created by Sebastian on 21.05.2019.
//

#pragma once

#include "SoftwareUart.h"
volatile uint8_t receiveBuffer;

namespace lib::software {
    template<typename mcu>
    requires mcu::family == MCUFamilies::AVR
    class SoftwareUart<mcu, SoftUartMethod::Assembler> {
    private:
        static constexpr auto preamble = 0x55;
        static constexpr auto RXBIT = 0;
        static constexpr auto RXPIN = 0x09;
        static auto receiveData() {
            asm volatile(R"(
                receiveByte:
                        sbic %1,%2
                        rjmp receiveByte
                        ldi r23, lo8(8)
                CL9:
                        movw xl, r26
                        lsr xh
                        ror xl
                        ldi r23, 9
                rxb3:
                        rcall WaitBitcell
                        lsr r22
                        sbic %1, %2
                        ori r22, 128
                        dec r23
                        brne rxb3
                        rjmp LEND
                WaitBitcell:
                        movw xl, r26
                wbc0:
                        sbiw xl, 4
                        brcc wbc0
                wbcx:
                        sts %0,r22
                        ret
                LEND:
            )"
            : "=m" (receiveBuffer)
            : "n" (RXPIN), "n" (RXBIT));
        }

        static auto waitForSync() {
            asm volatile(R"(
                rjmp waitForSyncASM
                skipHigh:
                    sbic %0,%1
                    rjmp skipHigh
                skipLow:
                        sbis %0,%1
                        rjmp skipLow
                waitForSyncASM:
                        clr r27
                        clr r26
                CL2:
                        sbic %0,%1
                        rjmp CL2
                CL3:
                        adiw r26,5
                        sbis %0,%1
                        rjmp CL3
                CL4:
                        sbic %0,%1
                        rjmp CL4
                CL5:
                        sbiw r26,5
                        sbis %0,%1
                        rjmp CL5
                        brmi skipHigh
            )"
            :
            : "n" (RXPIN), "n" (RXBIT));
        }
    public:
        static auto syncAndReceiveBytes(unsigned char *input, uint8_t elements) {
            waitForSync();
            //while(receiveData() != preamble) {}
            //for(uint8_t i=0; i < elements; i++) {
                input[elements - 1] = receiveBuffer;
            //}
            return input;
        }

        template<auto pinNumber, auto minBaud, auto maxBaud>
        static constexpr void init() {
            pin::setDirection<pin::Pin<mcu, pinNumber>, pin::Direction::INPUT>();
            pin::setInputState<pin::Pin<mcu, pinNumber>, pin::InputState::PULLUP>();

            //waitForSync();
        }
    };
}