#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>
#include <SDL.h>

#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

constexpr std::int32_t SCE_USBD_ERROR_INVALID_ARG = static_cast<std::int32_t>(0x80240002);
constexpr std::int32_t SCE_USBD_ERROR_OTHER = static_cast<std::int32_t>(0x802400FF);
constexpr std::int32_t SCE_USBD_ERROR_NOT_SUPPORTED = static_cast<std::int32_t>(0x8024000C);

constexpr int LIBUSB_ERROR_OTHER = -99;

struct UsbdTimeval {
    std::int64_t seconds;
    std::int64_t microseconds;
};

struct SceUsbdDevice;
struct SceUsbdDeviceHandle;
struct SceUsbdTransfer;
struct SceUsbdConfigDescriptor;
struct SceUsbdControlSetup;
using SceUsbdTransferCallback = void (*)(SceUsbdTransfer*);

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

struct UsbdApi {
    void* library = nullptr;
    void* context = nullptr;
    bool available = false;

    int (*init)(void** context) = nullptr;
    void (*exit)(void* context) = nullptr;
    std::ptrdiff_t (*getDeviceList)(void* context, void*** list) = nullptr;
    void (*freeDeviceList)(void** list, int unrefDevices) = nullptr;
    int (*handleEventsTimeout)(void* context, void* timeval) = nullptr;
    int (*eventHandlingOk)(void* context) = nullptr;
    SceUsbdTransfer* (*allocTransfer)(int isoPackets) = nullptr;
    void (*freeTransfer)(SceUsbdTransfer* transfer) = nullptr;
    int (*submitTransfer)(SceUsbdTransfer* transfer) = nullptr;
    int (*cancelTransfer)(SceUsbdTransfer* transfer) = nullptr;
    void (*fillInterruptTransfer)(SceUsbdTransfer* transfer, SceUsbdDeviceHandle* handle, std::uint8_t endpoint, std::uint8_t* buffer, int length, SceUsbdTransferCallback callback, void* userData, unsigned int timeout) = nullptr;
    int (*open)(SceUsbdDevice* device, SceUsbdDeviceHandle** handle) = nullptr;
    void (*close)(SceUsbdDeviceHandle* handle) = nullptr;
    SceUsbdDevice* (*refDevice)(SceUsbdDevice* device) = nullptr;
    void (*unrefDevice)(SceUsbdDevice* device) = nullptr;
    SceUsbdDevice* (*getDevice)(SceUsbdDeviceHandle* handle) = nullptr;
    std::uint8_t (*getBusNumber)(SceUsbdDevice* device) = nullptr;
    std::uint8_t (*getDeviceAddress)(SceUsbdDevice* device) = nullptr;
    int (*claimInterface)(SceUsbdDeviceHandle* handle, int interfaceNumber) = nullptr;
    int (*releaseInterface)(SceUsbdDeviceHandle* handle, int interfaceNumber) = nullptr;
    int (*kernelDriverActive)(SceUsbdDeviceHandle* handle, int interfaceNumber) = nullptr;
    int (*attachKernelDriver)(SceUsbdDeviceHandle* handle, int interfaceNumber) = nullptr;
    int (*setConfiguration)(SceUsbdDeviceHandle* handle, int configuration) = nullptr;
    int (*resetDevice)(SceUsbdDeviceHandle* handle) = nullptr;
    int (*getDeviceDescriptor)(SceUsbdDevice* device, SceUsbdDeviceDescriptor* descriptor) = nullptr;
    int (*getStringDescriptor)(SceUsbdDeviceHandle* handle, std::uint8_t index, std::uint16_t langid, std::uint8_t* data, int length) = nullptr;
    int (*getConfigDescriptor)(SceUsbdDevice* device, std::uint8_t index, SceUsbdConfigDescriptor** descriptor) = nullptr;
    int (*getActiveConfigDescriptor)(SceUsbdDevice* device, SceUsbdConfigDescriptor** descriptor) = nullptr;
    void (*freeConfigDescriptor)(SceUsbdConfigDescriptor* descriptor) = nullptr;
    int (*controlTransfer)(SceUsbdDeviceHandle* handle, std::uint8_t requestType, std::uint8_t request, std::uint16_t value, std::uint16_t index, std::uint8_t* data, std::uint16_t length, unsigned int timeout) = nullptr;
};

template <typename Function>
void Resolve(void* library, Function& function, const char* name) {
    function = reinterpret_cast<Function>(SDL_LoadFunction(library, name));
}

UsbdApi Load() {
    UsbdApi api;
    const char* names[] = {
#ifdef _WIN32
        "libusb-1.0.dll",
#elif defined(__APPLE__)
        "libusb-1.0.0.dylib",
        "libusb-1.0.dylib",
#else
        "libusb-1.0.so.0",
        "libusb-1.0.so",
#endif
    };
    for (const char* name : names) {
        api.library = SDL_LoadObject(name);
        if (api.library != nullptr) break;
    }
    if (api.library == nullptr) return api;
    Resolve(api.library, api.init, "libusb_init");
    Resolve(api.library, api.exit, "libusb_exit");
    Resolve(api.library, api.getDeviceList, "libusb_get_device_list");
    Resolve(api.library, api.freeDeviceList, "libusb_free_device_list");
    Resolve(api.library, api.handleEventsTimeout, "libusb_handle_events_timeout");
    Resolve(api.library, api.eventHandlingOk, "libusb_event_handling_ok");
    Resolve(api.library, api.allocTransfer, "libusb_alloc_transfer");
    Resolve(api.library, api.freeTransfer, "libusb_free_transfer");
    Resolve(api.library, api.submitTransfer, "libusb_submit_transfer");
    Resolve(api.library, api.cancelTransfer, "libusb_cancel_transfer");
    Resolve(api.library, api.fillInterruptTransfer, "libusb_fill_interrupt_transfer");
    Resolve(api.library, api.open, "libusb_open");
    Resolve(api.library, api.close, "libusb_close");
    Resolve(api.library, api.refDevice, "libusb_ref_device");
    Resolve(api.library, api.unrefDevice, "libusb_unref_device");
    Resolve(api.library, api.getDevice, "libusb_get_device");
    Resolve(api.library, api.getBusNumber, "libusb_get_bus_number");
    Resolve(api.library, api.getDeviceAddress, "libusb_get_device_address");
    Resolve(api.library, api.claimInterface, "libusb_claim_interface");
    Resolve(api.library, api.releaseInterface, "libusb_release_interface");
    Resolve(api.library, api.kernelDriverActive, "libusb_kernel_driver_active");
    Resolve(api.library, api.attachKernelDriver, "libusb_attach_kernel_driver");
    Resolve(api.library, api.setConfiguration, "libusb_set_configuration");
    Resolve(api.library, api.resetDevice, "libusb_reset_device");
    Resolve(api.library, api.getDeviceDescriptor, "libusb_get_device_descriptor");
    Resolve(api.library, api.getStringDescriptor, "libusb_get_string_descriptor");
    Resolve(api.library, api.getConfigDescriptor, "libusb_get_config_descriptor");
    Resolve(api.library, api.getActiveConfigDescriptor, "libusb_get_active_config_descriptor");
    Resolve(api.library, api.freeConfigDescriptor, "libusb_free_config_descriptor");
    Resolve(api.library, api.controlTransfer, "libusb_control_transfer");
    api.available = api.init != nullptr && api.exit != nullptr && api.getDeviceList != nullptr &&
        api.freeDeviceList != nullptr && api.allocTransfer != nullptr && api.freeTransfer != nullptr;
    return api;
}

UsbdApi& Api() {
    static UsbdApi api = Load();
    return api;
}

std::int32_t Error(const int result) {
    if (result == LIBUSB_ERROR_OTHER) return SCE_USBD_ERROR_OTHER;
    if (result < 0) return static_cast<std::int32_t>(0x80240000u - static_cast<std::uint32_t>(-static_cast<std::int64_t>(result)));
    return static_cast<std::int32_t>(result);
}

std::int32_t Unsupported() {
    return Api().available ? SCE_USBD_ERROR_OTHER : SCE_USBD_ERROR_NOT_SUPPORTED;
}

void* g_emptyDeviceList[1] = {nullptr};

}

extern "C" {

std::int32_t APS5_VABI sceUsbdInit() {
    UsbdApi& api = Api();
    if (!api.available || api.context != nullptr) return 0;
    void* context = nullptr;
    const auto result = api.init(&context);
    if (result != 0) return Error(result);
    api.context = context;
    return 0;
}

void APS5_VABI sceUsbdExit() {
    UsbdApi& api = Api();
    if (!api.available || api.context == nullptr) return;
    api.exit(api.context);
    api.context = nullptr;
}

std::int64_t APS5_VABI sceUsbdGetDeviceList(void*** list) {
    if (list == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available) {
        *list = g_emptyDeviceList;
        return 0;
    }
    void** devices = nullptr;
    const auto count = api.getDeviceList(api.context, &devices);
    if (count < 0) return Error(static_cast<int>(count));
    *list = devices;
    return static_cast<std::int64_t>(count);
}

void APS5_VABI sceUsbdFreeDeviceList(void** list, std::int32_t unrefDevices) {
    if (list == nullptr) return;
    UsbdApi& api = Api();
    if (!api.available || list == g_emptyDeviceList) {
        if (list != g_emptyDeviceList) throw std::runtime_error(std::string(__func__) + ": list was not returned by sceUsbdGetDeviceList");
        return;
    }
    api.freeDeviceList(list, unrefDevices != 0 ? 1 : 0);
}

std::int32_t APS5_VABI sceUsbdHandleEventsTimeout(const UsbdTimeval* timeout) {
    if (timeout == nullptr || timeout->seconds < 0 || timeout->microseconds < 0 || timeout->microseconds >= 1000000) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.context == nullptr || api.handleEventsTimeout == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(timeout->seconds) + std::chrono::microseconds(timeout->microseconds));
        return 0;
    }
    struct HostTimeval {
        long seconds;
        long microseconds;
    } host{static_cast<long>(timeout->seconds), static_cast<long>(timeout->microseconds)};
    return Error(api.handleEventsTimeout(api.context, &host));
}

std::int32_t APS5_VABI sceUsbdEventHandlingOk() {
    UsbdApi& api = Api();
    if (!api.available || api.context == nullptr || api.eventHandlingOk == nullptr) return 1;
    return api.eventHandlingOk(api.context);
}

SceUsbdTransfer* APS5_VABI sceUsbdAllocTransfer(std::int32_t isoPackets) {
    UsbdApi& api = Api();
    if (!api.available) return nullptr;
    return api.allocTransfer(isoPackets);
}

void APS5_VABI sceUsbdFreeTransfer(SceUsbdTransfer* transfer) {
    UsbdApi& api = Api();
    if (!api.available || transfer == nullptr) return;
    api.freeTransfer(transfer);
}

std::int32_t APS5_VABI sceUsbdSubmitTransfer(SceUsbdTransfer* transfer) {
    if (transfer == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.submitTransfer == nullptr) return Unsupported();
    return Error(api.submitTransfer(transfer));
}

std::int32_t APS5_VABI sceUsbdCancelTransfer(SceUsbdTransfer* transfer) {
    if (transfer == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.cancelTransfer == nullptr) return Unsupported();
    return Error(api.cancelTransfer(transfer));
}

void APS5_VABI sceUsbdFillInterruptTransfer(SceUsbdTransfer* transfer, SceUsbdDeviceHandle* handle, std::uint8_t endpoint, std::uint8_t* buffer, std::int32_t length, SceUsbdTransferCallback callback, void* userData, std::uint32_t timeout) {
    UsbdApi& api = Api();
    if (!api.available || api.fillInterruptTransfer == nullptr || transfer == nullptr || handle == nullptr) return;
    api.fillInterruptTransfer(transfer, handle, endpoint, buffer, length, callback, userData, timeout);
}

std::int32_t APS5_VABI sceUsbdOpen(SceUsbdDevice* device, SceUsbdDeviceHandle** handle) {
    if (device == nullptr || handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.open == nullptr) return Unsupported();
    *handle = nullptr;
    return Error(api.open(device, handle));
}

void APS5_VABI sceUsbdClose(SceUsbdDeviceHandle* handle) {
    UsbdApi& api = Api();
    if (!api.available || api.close == nullptr || handle == nullptr) return;
    api.close(handle);
}

SceUsbdDevice* APS5_VABI sceUsbdRefDevice(SceUsbdDevice* device) {
    UsbdApi& api = Api();
    if (!api.available || api.refDevice == nullptr || device == nullptr) return device;
    return api.refDevice(device);
}

void APS5_VABI sceUsbdUnrefDevice(SceUsbdDevice* device) {
    UsbdApi& api = Api();
    if (!api.available || api.unrefDevice == nullptr || device == nullptr) return;
    api.unrefDevice(device);
}

std::uint8_t APS5_VABI sceUsbdGetBusNumber(SceUsbdDevice* device) {
    UsbdApi& api = Api();
    if (!api.available || api.getBusNumber == nullptr || device == nullptr) return 0;
    return api.getBusNumber(device);
}

std::uint8_t APS5_VABI sceUsbdGetDeviceAddress(SceUsbdDevice* device) {
    UsbdApi& api = Api();
    if (!api.available || api.getDeviceAddress == nullptr || device == nullptr) return 0;
    return api.getDeviceAddress(device);
}

std::int32_t APS5_VABI sceUsbdCheckConnected(SceUsbdDeviceHandle* handle) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.getDevice == nullptr || api.getDeviceDescriptor == nullptr) return Unsupported();
    const auto device = api.getDevice(handle);
    if (device == nullptr) return SCE_USBD_ERROR_OTHER;
    SceUsbdDeviceDescriptor descriptor{};
    return Error(api.getDeviceDescriptor(device, &descriptor));
}

std::int32_t APS5_VABI sceUsbdClaimInterface(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.claimInterface == nullptr) return Unsupported();
    return Error(api.claimInterface(handle, interfaceNumber));
}

std::int32_t APS5_VABI sceUsbdReleaseInterface(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.releaseInterface == nullptr) return Unsupported();
    return Error(api.releaseInterface(handle, interfaceNumber));
}

std::int32_t APS5_VABI sceUsbdKernelDriverActive(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.kernelDriverActive == nullptr) return Unsupported();
    return Error(api.kernelDriverActive(handle, interfaceNumber));
}

std::int32_t APS5_VABI sceUsbdAttachKernelDriver(SceUsbdDeviceHandle* handle, std::int32_t interfaceNumber) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.attachKernelDriver == nullptr) return Unsupported();
    return Error(api.attachKernelDriver(handle, interfaceNumber));
}

std::int32_t APS5_VABI sceUsbdSetConfiguration(SceUsbdDeviceHandle* handle, std::int32_t configuration) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.setConfiguration == nullptr) return Unsupported();
    return Error(api.setConfiguration(handle, configuration));
}

std::int32_t APS5_VABI sceUsbdResetDevice(SceUsbdDeviceHandle* handle) {
    if (handle == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.resetDevice == nullptr) return Unsupported();
    return Error(api.resetDevice(handle));
}

std::int32_t APS5_VABI sceUsbdGetDeviceDescriptor(SceUsbdDevice* device, SceUsbdDeviceDescriptor* descriptor) {
    if (device == nullptr || descriptor == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.getDeviceDescriptor == nullptr) return Unsupported();
    return Error(api.getDeviceDescriptor(device, descriptor));
}

std::int32_t APS5_VABI sceUsbdGetStringDescriptor(SceUsbdDeviceHandle* handle, std::uint8_t index, std::uint16_t langid, std::uint8_t* data, std::int32_t length) {
    if (handle == nullptr || data == nullptr || length <= 0) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.getStringDescriptor == nullptr) return Unsupported();
    return Error(api.getStringDescriptor(handle, index, langid, data, length));
}

std::int32_t APS5_VABI sceUsbdGetConfigDescriptor(SceUsbdDevice* device, std::uint8_t index, SceUsbdConfigDescriptor** descriptor) {
    if (device == nullptr || descriptor == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.getConfigDescriptor == nullptr) return Unsupported();
    *descriptor = nullptr;
    return Error(api.getConfigDescriptor(device, index, descriptor));
}

std::int32_t APS5_VABI sceUsbdGetActiveConfigDescriptor(SceUsbdDevice* device, SceUsbdConfigDescriptor** descriptor) {
    if (device == nullptr || descriptor == nullptr) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.getActiveConfigDescriptor == nullptr) return Unsupported();
    *descriptor = nullptr;
    return Error(api.getActiveConfigDescriptor(device, descriptor));
}

void APS5_VABI sceUsbdFreeConfigDescriptor(SceUsbdConfigDescriptor* descriptor) {
    UsbdApi& api = Api();
    if (!api.available || api.freeConfigDescriptor == nullptr || descriptor == nullptr) return;
    api.freeConfigDescriptor(descriptor);
}

std::int32_t APS5_VABI sceUsbdControlTransfer(SceUsbdDeviceHandle* handle, std::uint8_t requestType, std::uint8_t request, std::uint16_t value, std::uint16_t index, std::uint8_t* data, std::int32_t length, std::uint32_t timeout) {
    if (handle == nullptr || length < 0 || length > 65535 || (length > 0 && data == nullptr)) return SCE_USBD_ERROR_INVALID_ARG;
    UsbdApi& api = Api();
    if (!api.available || api.controlTransfer == nullptr) return Unsupported();
    return Error(api.controlTransfer(handle, requestType, request, value, index, data, static_cast<std::uint16_t>(length), timeout));
}

}
