//
// Created by Sebastian on 12.05.2019.
//

#pragma once

#include "../../utils/AvrUtils.h"
#include "tiny2313.h"
#include "../../pin/Control.h"
#include "../../../concepts/Pin.h"

//Arduino Pinout
namespace pin {
    using mcu = lib::avr::ATTiny2313;

    using PortA = lib::Hal::Port<lib::avr::A, mcu>;
    using PortB = lib::Hal::Port<lib::avr::B, mcu>;
    using PortD = lib::Hal::Port<lib::avr::D, mcu>;
//using PortE = lib::Hal::Port<lib::avr::E, mcu>;

    using PinB0 = lib::Hal::Pin<PortB, 0>;

    using PinA0 = lib::Hal::Pin<PortA, 0>;

    using PinD0 = lib::Hal::Pin<PortD, 0>;

//------- [ D ] ------

    template<>
    struct Pin<mcu, 0> {
        using value = lib::Hal::Pin<PortD, 0>;
    };

    template<>
    struct Pin<mcu, 1> {
        using value = lib::Hal::Pin<PortD, 1>;
    };

    template<>
    struct Pin<mcu, 2> {
        using value = lib::Hal::Pin<PortD, 2>;
    };

    template<>
    struct Pin<mcu, 3> {
        using value = lib::Hal::Pin<PortD, 3>;
    };

    template<>
    struct Pin<mcu, 4> {
        using value = lib::Hal::Pin<PortD, 4>;
    };

    template<>
    struct Pin<mcu, 5> {
        using value = lib::Hal::Pin<PortD, 5>;
    };

    template<>
    struct Pin<mcu, 6> {
        using value = lib::Hal::Pin<PortD, 6>;
    };

//------- [ B ] ------

    template<>
    struct Pin<mcu, 7> {
        using value = lib::Hal::Pin<PortB, 0>;
    };

    template<>
    struct Pin<mcu, 8> {
        using value = lib::Hal::Pin<PortB, 1>;
    };

    template<>
    struct Pin<mcu, 9> {
        using value = lib::Hal::Pin<PortB, 2>;
    };

    template<>
    struct Pin<mcu, 10> {
        using value = lib::Hal::Pin<PortB, 3>;
    };

    template<>
    struct Pin<mcu, 11> {
        using value = lib::Hal::Pin<PortB, 4>;
    };

    template<>
    struct Pin<mcu, 12> {
        using value = lib::Hal::Pin<PortB, 6>;
    };

    template<>
    struct Pin<mcu, 13> {
        using value = lib::Hal::Pin<PortB, 7>;
    };

//------- [ A ] ------

    template<>
    struct Pin<mcu, 14> {
        using value = lib::Hal::Pin<PortA, 0>;
    };

    template<>
    struct Pin<mcu, 15> {
        using value = lib::Hal::Pin<PortA, 1>;
    };

    template<>
    struct Pin<mcu, 16> {
        using value = lib::Hal::Pin<PortA, 2>;
    };
}