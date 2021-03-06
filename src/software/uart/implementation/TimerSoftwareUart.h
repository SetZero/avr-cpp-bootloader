//
// Created by Sebastian on 21.05.2019.
//

#pragma once

#include <util/delay.h>
#include "SoftwareUart.h"
#include "../../../hal/concepts/Pin.h"
#include "../../Literals.h"


namespace lib::software {
    template<typename mcu, auto pinNumber>
        requires pin::isAbstractPin<pin::Pin<mcu, pinNumber>>
    class SoftwareUart<mcu, pinNumber, SoftUartMethod::Timer> {
    private:
        static int16_t bitcellLength;
        using timer = AbstractTimer<mcu>;
        static constexpr auto praeamble = 0x55;
        static constexpr auto sync = 0xCC;

        static constexpr auto isHigh() {
            using pin = pin::Pin<mcu, 0>::value;
            return (pin::get() != 0);
        }

        template<typename T> requires concepts::UnsignedType<T>

        static constexpr T calculateTime(T startValue, T endValue) {
            if constexpr(timer::timerBitCount() == sizeof(T) * 8) {
                // value needs to be unsigned to also work for wrapped around differences
                return endValue - startValue;
            } else {
                return (endValue - startValue) & (utils::getMaxValueOfBitcount<timer::timerBitCount()>());
            }
        }
    public:
        template<Baud minBaud, Baud maxBaud>
        static constexpr void init() {
            using namespace lib::software::literals;
            constexpr auto timerClockSpeed = 250000_hz;
            constexpr auto ullMinBaud = static_cast<unsigned long long>(minBaud);
            constexpr auto ullMaxBaud = static_cast<unsigned long long>(maxBaud);

            timer::template init<timerClockSpeed>();
            constexpr auto realTimerValue = static_cast<unsigned long long>(timer::template getRealClockValue<timerClockSpeed>());
            constexpr long double timerOffset = (static_cast<unsigned long long>(timerClockSpeed) /
                                                 (long double) realTimerValue);
            constexpr long double TimerTicksPerBaud = realTimerValue / ullMaxBaud;

            static_assert(timerOffset <= 1.1 && timerOffset >= 0.9, "Timer Value not within acceptable error margin (change timerClockSpeed)!");
            static_assert(realTimerValue / ullMinBaud > 0, "Timer too slow for minimum baud rate!");
            static_assert(TimerTicksPerBaud < 1000, "Timer too slow or baud rate too high!");
            static_assert(realTimerValue / ullMinBaud <= utils::getMaxValueOfBitcount<timer::timerBitCount()>(),
                          "Timer will overflow at minimum baud rate!");
            static_assert((realTimerValue / (long double) ullMinBaud) -
                          utils::math::floor(realTimerValue / (long double) ullMinBaud) <= 0.1,
                          "Minimum Baud not within acceptable error margin!");
            static_assert((realTimerValue / (long double) ullMaxBaud) -
                          utils::math::floor(realTimerValue / (long double) ullMaxBaud) <= 0.1,
                          "Maximum Baud not within acceptable error margin!");
            static_assert(ullMinBaud <= ullMaxBaud, "Minimum Baud has to be below or equal to maximum baud!");


            pin::setDirection<::pin::Pin<mcu, pinNumber>, pin::Direction::INPUT>();
            pin::setInputState<::pin::Pin<mcu, pinNumber>, pin::InputState::PULLUP>();
        }

        static void waitForSync() {
            while (true) {
                while (isHigh()) {};            //skip first high

                auto startValue = timer::readValue();
                while (!isHigh()) { }        //measure first low time

                auto endValue = timer::readValue();
                bitcellLength = calculateTime(startValue, endValue);

                while (isHigh()) {};         //wait for 2nd low

                startValue = timer::readValue();
                while (!isHigh()) {
                }
                endValue = timer::readValue();
                bitcellLength -= calculateTime(startValue, endValue);


                if (bitcellLength < 0) {
                    while (isHigh()) {}
                    while (!isHigh()) {}      //repeat until in sync
                } else {
                    break;                  //sync
                }
            }
            while (receiveData() != praeamble) {}
        }

        static auto receiveData() {
            uint8_t buffer = 0;
            uint8_t i = 0;                      // this position is mandatory
            while (isHigh()) {}                   // skip everything before start (this will keep the sync)
            for (; i < 9; i++) {                  // 8-N-1 (will overwrite start bit)
                auto startValue = timer::readValue();
                for(int tmp = bitcellLength / 2; tmp > 0; tmp--) { asm volatile(""); }
                buffer /= 2;                    // lshift
                if (isHigh()) {
                    buffer |= (1u << 7u);
                }
                while (calculateTime(startValue, timer::readValue()) < bitcellLength) {}
            }
            while (!isHigh()) {}                  // skip last low (stop bit)
            return buffer;
        }
    };

    template<typename mcu, auto pinNumber>
    int16_t SoftwareUart<mcu, pinNumber, SoftUartMethod::Timer>::bitcellLength = 0;
}



