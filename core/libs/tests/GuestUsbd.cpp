#include "prx/libc/include/general/VabiMacros.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {

struct UsbdTimeval {
    std::int64_t seconds;
    std::int64_t microseconds;
};

struct SceUsbdDevice;
struct SceUsbdDeviceHandle;
struct SceUsbdTransfer;
struct SceUsbdConfigDescriptor;

struct SceUsbdDeviceDescriptor {
    std::uint8_t Length;
    std::uint8_t DescriptorType;
    std::uint16_t BcdUsb;
    std::uint8_t DeviceClass;
    std::uint8_t DeviceSubClass;
    std::uint8_t DeviceProtocol;
    std::uint8_t MaxPacketSize0;
    std::uint16_t IdVendor;
    std::uint16_t IdProduct;
    std::uint16_t BcdDevice;
    std::uint8_t Manufacturer;
    std::uint8_t Product;
    std::uint8_t SerialNumber;
    std::uint8_t ConfigurationCount;
};

}

extern "C" {
std::int32_t APS5_VABI sceUsbdInit();
void APS5_VABI sceUsbdExit();
std::int64_t APS5_VABI sceUsbdGetDeviceList(void*** list);
void APS5_VABI sceUsbdFreeDeviceList(void** list, std::int32_t unrefDevices);
std::int32_t APS5_VABI sceUsbdHandleEventsTimeout(const UsbdTimeval* timeout);
std::int32_t APS5_VABI sceUsbdEventHandlingOk();
SceUsbdTransfer* APS5_VABI sceUsbdAllocTransfer(std::int32_t isoPackets);
void APS5_VABI sceUsbdFreeTransfer(SceUsbdTransfer* transfer);
std::int32_t APS5_VABI sceUsbdSubmitTransfer(SceUsbdTransfer* transfer);
std::int32_t APS5_VABI sceUsbdCancelTransfer(SceUsbdTransfer* transfer);
void APS5_VABI sceUsbdFillInterruptTransfer(SceUsbdTransfer* transfer, SceUsbdDeviceHandle* handle, std::uint8_t endpoint, std::uint8_t* buffer, std::int32_t length, void (*callback)(SceUsbdTransfer*), void* userData, std::uint32_t timeout);
std::int32_t APS5_VABI sceUsbdOpen(SceUsbdDevice* device, SceUsbdDeviceHandle** handle);
void APS5_VABI sceUsbdClose(SceUsbdDeviceHandle* handle);
SceUsbdDevice* APS5_VABI sceUsbdRefDevice(SceUsbdDevice* device);
void APS5_VABI sceUsbdUnrefDevice(SceUsbdDevice* device);
std::uint8_t APS5_VABI sceUsbdGetBusNumber(SceUsbdDevice* device);
std::uint8_t APS5_VABI sceUsbdGetDeviceAddress(SceUsbdDevice* device);
std::int32_t APS5_VABI sceUsbdCheckConnected(SceUsbdDeviceHandle* handle);
std::int32_t APS5_VABI sceUsbdClaimInterface(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber);
std::int32_t APS5_VABI sceUsbdReleaseInterface(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber);
std::int32_t APS5_VABI sceUsbdKernelDriverActive(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber);
std::int32_t APS5_VABI sceUsbdAttachKernelDriver(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber);
std::int32_t APS5_VABI sceUsbdSetConfiguration(SceUsbdDeviceHandle* handle, std::int32_t configuration);
std::int32_t APS5_VABI sceUsbdResetDevice(SceUsbdDeviceHandle* handle);
std::int32_t APS5_VABI sceUsbdGetDeviceDescriptor(SceUsbdDevice* device, SceUsbdDeviceDescriptor* descriptor);
std::int32_t APS5_VABI sceUsbdGetStringDescriptor(SceUsbdDeviceHandle* handle, std::uint8_t index, std::uint16_t langid, std::uint8_t* data, std::int32_t length);
std::int32_t APS5_VABI sceUsbdGetConfigDescriptor(SceUsbdDevice* device, std::uint8_t index, SceUsbdConfigDescriptor** descriptor);
std::int32_t APS5_VABI sceUsbdGetActiveConfigDescriptor(SceUsbdDevice* device, SceUsbdConfigDescriptor** descriptor);
void APS5_VABI sceUsbdFreeConfigDescriptor(SceUsbdConfigDescriptor* descriptor);
std::int32_t APS5_VABI sceUsbdControlTransfer(SceUsbdDeviceHandle* handle, std::uint8_t requestType, std::uint8_t request, std::uint16_t value, std::uint16_t index, std::uint8_t* data, std::int32_t length, std::uint32_t timeout);
}

namespace {

constexpr std::int32_t invalidArgument = static_cast<std::int32_t>(0x80240002);
constexpr std::int32_t notSupported = static_cast<std::int32_t>(0x8024000C);

void Require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "USBD: %s\n", message);
        std::abort();
    }
}

}

int main() {
    Require(sceUsbdInit() == 0, "initialization failed");
    void** list = nullptr;
    Require(sceUsbdGetDeviceList(&list) >= 0, "the device list reported a failure");
    Require(list != nullptr, "the device list is null");
    std::size_t devices = 0;
    while (devices < 1024 && list[devices] != nullptr) ++devices;
    Require(devices < 1024, "the device list is not null-terminated");
    sceUsbdFreeDeviceList(list, 1);
    Require(sceUsbdGetDeviceList(nullptr) == invalidArgument, "null list accepted");
    Require(sceUsbdHandleEventsTimeout(nullptr) == invalidArgument, "null timeout accepted");
    const UsbdTimeval invalid{0, 1000000};
    Require(sceUsbdHandleEventsTimeout(&invalid) == invalidArgument, "out-of-range microseconds accepted");
    const UsbdTimeval timeout{0, 50000};
    const auto start = std::chrono::steady_clock::now();
    Require(sceUsbdHandleEventsTimeout(&timeout) == 0, "event handling failed");
    Require(std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(45), "event handling returned before the timeout");
    Require(sceUsbdEventHandlingOk() == 1, "event handling is not reported as ok");
    auto* transfer = sceUsbdAllocTransfer(0);
    if (transfer != nullptr) {
        sceUsbdFreeTransfer(transfer);
        SceUsbdDeviceHandle* handle = reinterpret_cast<SceUsbdDeviceHandle*>(1);
        Require(sceUsbdSubmitTransfer(nullptr) == invalidArgument, "a null transfer was submitted");
        Require(sceUsbdCancelTransfer(nullptr) == invalidArgument, "a null transfer was cancelled");
        Require(sceUsbdClaimInterface(nullptr, 0) == invalidArgument, "a null device handle claimed an interface");
        Require(sceUsbdResetDevice(nullptr) == invalidArgument, "a null device handle was reset");
        Require(sceUsbdGetStringDescriptor(nullptr, 1, 0x409, nullptr, 8) == invalidArgument, "a null string buffer was accepted");
        Require(sceUsbdOpen(nullptr, &handle) == invalidArgument, "a null device was opened");
        Require(sceUsbdCheckConnected(nullptr) == invalidArgument, "a null device handle reports a connection");
        Require(sceUsbdGetBusNumber(nullptr) == 0, "a null device has a bus number");
        sceUsbdFreeTransfer(nullptr);
        sceUsbdClose(nullptr);
        sceUsbdUnrefDevice(nullptr);
        sceUsbdFreeConfigDescriptor(nullptr);
    } else {
        Require(sceUsbdClaimInterface(reinterpret_cast<SceUsbdDeviceHandle*>(1), 0) == notSupported, "a device call without a host USB library did not report it as unsupported");
        Require(sceUsbdSubmitTransfer(reinterpret_cast<SceUsbdTransfer*>(1)) == notSupported, "a transfer without a host USB library did not report it as unsupported");
    }
    SceUsbdDeviceDescriptor descriptor{};
    Require(sceUsbdGetDeviceDescriptor(nullptr, &descriptor) == invalidArgument, "a null device returned a descriptor");
    Require(sceUsbdGetDeviceDescriptor(reinterpret_cast<SceUsbdDevice*>(1), nullptr) == invalidArgument, "a null descriptor pointer was accepted");
    Require(sceUsbdControlTransfer(nullptr, 0, 0, 0, 0, nullptr, 0, 0) == invalidArgument, "a null device handle ran a control transfer");
    Require(sceUsbdControlTransfer(reinterpret_cast<SceUsbdDeviceHandle*>(1), 0, 0, 0, 0, nullptr, -1, 0) == invalidArgument, "a negative control transfer length was accepted");
    sceUsbdExit();
    std::puts("USBD tests passed");
    return 0;
}
