//
// Created by Sebastian on 18.06.2019.
//

#pragma once

#include "implementation/SoftwareUart.h"
#include "implementation/TimingBasedUart.h"
#include "implementation/TimerSoftwareUart.h"
#include "implementation/vendorspecific/InlineAssemblerSoftwareUart.h"
#include "implementation/vendorspecific/AssemblerSoftwareUart.h"
#include "implementation/vendorspecific/external/Sync.h"
#include "../../utils/custom_limits.h"

namespace lib::software {
    template<typename mcu, auto pinNumber, SoftUartMethod method>
    class AbstractSoftwareUart {
    private:
        using softUart = SoftwareUart<mcu, pinNumber, method>;
        using selectedPin = pin::Pin<mcu, pinNumber>::value;
    public:
        static uint16_t getWord() {
            softUart::waitForSync();
            return softUart::receiveData() | (static_cast<uint16_t>(softUart::receiveData()) << 8u);
        }

        template<auto N>
        [[nodiscard]] static auto getBytes()
        requires utils::is_arithmetic<decltype(N)>::value
        && N <= 2 {
            using type = utils::byte_type<N>::value_type;
            type value = 0;
            softUart::waitForSync();


            for (typename mcu::mem_width i = 0; i < N; i++) {
                value |= static_cast<type>(softUart::receiveData()) << (8u * i);
            }
            return value;
        }

        template<auto N>
        [[nodiscard]] static const utils::array<typename mcu::mem_width, N> *getBytes()
        requires utils::is_arithmetic<decltype(N)>::value
        && N > 2 && N <= 255 {
            static utils::array<typename mcu::mem_width, N> value;
            softUart::waitForSync();
            for (typename mcu::mem_width i = 0; i < N; i++) {
                value[i] = softUart::receiveData();
            }
            return &value;
        }

        template<auto minBaud, auto maxBaud>
        static constexpr void init() {
             softUart::template init<minBaud, maxBaud>();
        }

        template<typename T> requires utils::is_arithmetic<T>::value
        static bool gotSignalBeforeTimout() {
            using pin = pin::Pin<mcu, pinNumber>::value;
            for(T i = 0; i < utils::numeric_limits<T>::max(); i++) { // utils::numeric_limits<T>::max()
                asm volatile("");
                if(pin::get() == 0) {
                    return true;
                }
            }
            return false;
        }
    };
}



