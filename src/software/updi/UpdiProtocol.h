//
// Created by Sebastian on 31.07.2019.
//

#pragma once


#include "AbstractSoftwareUPDI.h"
#include "constants/UPDIConstants.h"
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
#include "../../hal/avr/utils/bootloader/mega/UpdiBootloader.h"
#endif
#ifdef __AVR_ATtiny2313__
#include "../../hal/avr/utils/bootloader/tiny/UpdiBootloader.h"
#endif

using namespace lib::software::literals;
namespace lib::software {
    struct UPDIStore {
        uint16_t ptr = 0;
        uint16_t repeats = 0;
        bool flash_write_error = false;
    };


    template<typename mcu, auto pinNumber>
    class UPDIProtocol {
    private:
        using softUPDI = lib::software::AbstractSoftwareUPDI<mcu, pinNumber, lib::software::SoftUPDIMethod::UPDITimingBased>;
        using bootloader = lib::avr::boot::UPDIBootloader<mcu>;

        [[noreturn]] static constexpr void (*start_pgm)() = 0x0000;
        static UPDIStore store;
    public:
        static void start() {
            if(bootloader::resetReason() == lib::avr::boot::ResetReason::NOT_EXT_RESET) {
                start_pgm();
            }

            softUPDI::template init<9600_baud, 9600_baud>();
            if (!softUPDI::template gotSignalBeforeTimout<uint32_t>()) {
                start_pgm();
            }

            while(true) {
                uint8_t data = softUPDI::getByte();
                updiStateMachine(data);
            }
        }

        static void updiStateMachine(uint8_t data) {
            switch (data & 0xF0) {
                case updi::UPDI_STCS: {
                    uint8_t value = softUPDI::getByteWithoutSync();
                    switch (data & 0x0F) {
                        case updi::UPDI_ASI_RESET_REQ:
                            if (value == updi::UPDI_RESET_REQ_VALUE) {
                                start_pgm();
                            }
                    }
                    break;
                }
                case updi::UPDI_LDCS:
                    switch (data & 0x0F) {
                        case updi::UPDI_CS_STATUSA:
                            softUPDI::sendChar(0x30); // is this ack?
                            break;
                        case updi::UPDI_ASI_SYS_STATUS:
                            sendStatusInfo();
                            break;
                    }
                    break;
                case updi::UPDI_KEY:
                    switch (data & 0x0F) {
                        case updi::UPDI_KEY_SIB | updi::UPDI_SIB_16BYTES:
                            softUPDI::sendString(updi::DEVICE_STRING);
                    }
                    break;
                case updi::UPDI_ST:
                    if (data & updi::UPDI_PTR_ADDRESS) {
                        if (data & updi::UPDI_DATA_16) {
                            //16 Bit Data
                            store.ptr = getWordValue();
                            store.repeats = 1;
                        } else {
                            // 8 Bit
                            store.ptr = softUPDI::getByte();
                        }
                        softUPDI::sendChar(updi::UPDI_PHY_ACK);
                    } else if (data & updi::UPDI_PTR_INC) {
                        if (data & updi::UPDI_DATA_16) {
                            if (store.ptr >= updi::UPDI_ADDRESS_OFFSET) {
                                writeToFlashBuffer();
                            }
                        }
                    }
                    break;
                case updi::UPDI_STS: {
                    if (data & updi::UPDI_ADDRESS_16) {
                        getWordValue();
                    }
                    softUPDI::sendChar(updi::UPDI_PHY_ACK);
                    executeSTSControlCommand();
                    break;
                }
                case updi::UPDI_LD:
                    if (data & updi::UPDI_PTR_INC) {
                        if (!(data & updi::UPDI_DATA_16)) {
                            auto ptr_data = lookup_ptr();
                            for (uint8_t i = 0; i < store.repeats; i++) {
                                softUPDI::sendChar(ptr_data[i]);
                            }
                        } else {
                            if (store.ptr >= updi::UPDI_ADDRESS_OFFSET) {
                                uint16_t start_address = store.ptr - updi::UPDI_ADDRESS_OFFSET;
                                for (uint8_t i = 0; i < store.repeats * 2; i++) {
                                    softUPDI::sendChar(bootloader::readFlash(start_address + i));
                                }
                            }
                        }
                    }
                    break;
                case updi::UPDI_LDS:
                    if (data & updi::UPDI_ADDRESS_16) {
                        store.ptr = getWordValue();
                        store.repeats = 1;
                        if(store.ptr == updi::NVMCTRL_ADDRESS + updi::UPDI_NVMCTRL_STATUS) {
                            if(store.flash_write_error) {
                                softUPDI::sendChar(1 << updi::UPDI_NVM_STATUS_WRITE_ERROR);
                            } else {
                                if(bootloader::flashBusy()) {
                                    softUPDI::sendChar(1 << updi::UPDI_NVM_STATUS_FLASH_BUSY);
                                } else {
                                    softUPDI::sendChar(updi::UPDI_NVM_STATUS_READY);
                                    bootloader::enableFlash();
                                }
                            }
                        } else {
                            softUPDI::sendChar(0x0);
                        }
                    }
                    break;
                case updi::UPDI_REPEAT:
                    if (data & updi::UPDI_REPEAT_WORD) {
                        store.repeats = getWordValue() + 1;
                    } else {
                        store.repeats = softUPDI::getByteWithoutSync() + 1u;
                    }
                    break;
            }
        }

        static void executeSTSControlCommand() {
            uint8_t flash_cmd = softUPDI::getByteWithoutSync();
            if (flash_cmd == updi::UPDI_NVMCTRL_CTRLA_CHIP_ERASE) {
                // Erase Chip
            } else if (flash_cmd == updi::UPDI_NVMCTRL_CTRLA_PAGE_BUFFER_CLR) {
                // Clear Page Buffer
            } else if (flash_cmd == updi::UPDI_NVMCTRL_CTRLA_WRITE_PAGE) {
                uint16_t startAddress = store.ptr - updi::UPDI_ADDRESS_OFFSET;
                bootloader::writeFlashPage(startAddress);
            }
            softUPDI::sendChar(updi::UPDI_PHY_ACK);
        }

        static uint16_t getWordValue() {
            const uint8_t lsb = softUPDI::getByteWithoutSync();
            const uint8_t msb = softUPDI::getByteWithoutSync();
            return static_cast<uint16_t>(msb << 8u) | lsb;
        }

        static void writeToFlashBuffer() {
            uint16_t startAddress = store.ptr - updi::UPDI_ADDRESS_OFFSET;
            bootloader::clearFlashPage(startAddress);
            for (uint8_t i = 0; i < store.repeats * 2; i+=2) {
                bootloader::fillFlashPage(startAddress + i, getWordValue());
            }
        }

        static void sendStatusInfo() {
            //TODO: Fully add status Info
            //We are already in prog mode... or at least we say so
            constexpr uint8_t status = (1 << updi::UPDI_ASI_SYS_STATUS_NVMPROG);
            softUPDI::sendChar(status);
        }

        static constexpr const uint8_t* lookup_ptr() {
            switch(store.ptr) {
                case 0x1100:
                    return updi::UPDI_DEVICE_ID;
                case 0x0F01:
                    return updi::UPDI_DEVICE_REVISION;
            }
            return nullptr;
        }
    };


    template<typename mcu, auto pinNumber>
    UPDIStore UPDIProtocol<mcu, pinNumber>::store;
}


