// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "display_metadata.hpp"

#include <QScreen>
#include <QtGui/qscreen_platform.h>
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include <QCryptographicHash>
#endif

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#import <IOKit/IOKitLib.h>
#endif

#include <cstdint>
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include <cstring>
#endif
#include <iomanip>
#include <sstream>
#include <string>

namespace program_display::platform::macos {
    namespace {

        CGDirectDisplayID directDisplayId(QScreen *screen)
        {
            if (!screen) {
                return kCGNullDirectDisplay;
            }

            auto *native_interface = screen->nativeInterface<QNativeInterface::QCocoaScreen>();
            if (!native_interface) {
                return kCGNullDirectDisplay;
            }

            NSScreen *native_screen = native_interface->nativeScreen();
            if (!native_screen) {
                return kCGNullDirectDisplay;
            }

            if (@available(macOS 26.0, *)) {
                return native_screen.CGDirectDisplayID;
            }

            // Before macOS 26, AppKit exposed the same transient CoreGraphics ID in the
            // screen's device description rather than through a typed property.
            NSNumber *screen_number = native_screen.deviceDescription[@"NSScreenNumber"];
            return screen_number ? screen_number.unsignedIntValue : kCGNullDirectDisplay;
        }

        std::string pnpManufacturerCode(std::uint32_t vendor)
        {
            const char first = static_cast<char>(((vendor >> 10U) & 0x1FU) + '@');
            const char second = static_cast<char>(((vendor >> 5U) & 0x1FU) + '@');
            const char third = static_cast<char>((vendor & 0x1FU) + '@');
            if (first < 'A' || first > 'Z' || second < 'A' || second > 'Z' || third < 'A' || third > 'Z') {
                return {};
            }
            return {first, second, third};
        }

        std::string productCode(std::uint32_t product)
        {
            std::ostringstream value;
            value << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << product;
            return value.str();
        }

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
        std::string booleanText(boolean_t value)
        {
            return value ? "yes" : "no";
        }

        std::string cfString(CFTypeRef value)
        {
            if (!value || CFGetTypeID(value) != CFStringGetTypeID()) {
                return {};
            }

            const auto string = static_cast<CFStringRef>(value);
            const CFIndex maximum =
                CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8) + 1;
            std::string result(static_cast<std::size_t>(maximum), '\0');
            if (!CFStringGetCString(string, result.data(), maximum, kCFStringEncodingUTF8)) {
                return {};
            }
            result.resize(std::strlen(result.c_str()));
            return result;
        }

        CFTypeRef dictionaryValue(CFDictionaryRef dictionary, CFStringRef key)
        {
            return dictionary ? CFDictionaryGetValue(dictionary, key) : nullptr;
        }

        CFDictionaryRef dictionaryProperty(CFDictionaryRef dictionary, CFStringRef key)
        {
            CFTypeRef value = dictionaryValue(dictionary, key);
            return value && CFGetTypeID(value) == CFDictionaryGetTypeID() ? static_cast<CFDictionaryRef>(value)
                                                                          : nullptr;
        }

        std::string sha256(const std::string &value)
        {
            if (value.empty()) {
                return "unavailable";
            }
            return QCryptographicHash::hash(QByteArray::fromStdString(value), QCryptographicHash::Sha256)
                .toHex()
                .toStdString();
        }

        std::string propertyString(CFDictionaryRef dictionary, CFStringRef key)
        {
            return cfString(dictionaryValue(dictionary, key));
        }

        std::string scalarString(CFTypeRef value)
        {
            if (!value) {
                return {};
            }
            if (CFGetTypeID(value) == CFStringGetTypeID()) {
                return cfString(value);
            }
            if (CFGetTypeID(value) == CFNumberGetTypeID()) {
                std::int64_t number = 0;
                if (CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &number)) {
                    return std::to_string(number);
                }
            }
            if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
                return CFBooleanGetValue(static_cast<CFBooleanRef>(value)) ? "yes" : "no";
            }
            return {};
        }

        std::string scalarProperty(CFDictionaryRef dictionary, CFStringRef key)
        {
            return scalarString(dictionaryValue(dictionary, key));
        }

        bool nonZeroNumber(CFTypeRef value)
        {
            if (!value || CFGetTypeID(value) != CFNumberGetTypeID()) {
                return false;
            }
            std::int64_t number = 0;
            return CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &number) && number != 0;
        }
#endif

    }  // namespace

    void supplementDisplayMetadata(QScreen *screen, domain::DisplayMetadata &metadata)
    {
        const CGDirectDisplayID display_id = directDisplayId(screen);
        if (display_id == kCGNullDirectDisplay) {
            return;
        }

        const std::uint32_t vendor = CGDisplayVendorNumber(display_id);
        const std::uint32_t product = CGDisplayModelNumber(display_id);
        const std::uint32_t serial = CGDisplaySerialNumber(display_id);

        if (domain::normalize_identity_text(metadata.manufacturer).empty() && vendor != 0) {
            metadata.manufacturer = pnpManufacturerCode(vendor);
        }
        if (domain::normalize_identity_text(metadata.model).empty() && product != 0) {
            metadata.model = productCode(product);
        }
        if (domain::normalize_identity_text(metadata.serial).empty() && serial != 0) {
            metadata.serial = std::to_string(serial);
        }
    }

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
    std::string displayDiagnosticFor(QScreen *screen)
    {
        const CGDirectDisplayID display_id = directDisplayId(screen);
        if (display_id == kCGNullDirectDisplay) {
            return "cg-display-id=unavailable edid-sha256=unavailable io-location=unavailable";
        }

        std::ostringstream diagnostic;
        diagnostic << "cg-display-id=" << display_id << " cg-vendor=" << CGDisplayVendorNumber(display_id)
                   << " cg-product=" << CGDisplayModelNumber(display_id)
                   << " cg-serial-present=" << booleanText(CGDisplaySerialNumber(display_id) != 0)
                   << " cg-built-in=" << booleanText(CGDisplayIsBuiltin(display_id));

        // CGDisplayIOServicePort is the old route from a CG display to IOKit display
        // properties, but Apple marks it "No longer supported". Do not use it to
        // obtain EDID or IODisplayLocation: the resulting QScreen association would
        // not be a supported basis even for diagnostics.
        diagnostic << " edid-sha256=unavailable(supported-iokit-mapping-unavailable)"
                   << " io-location=unavailable(supported-iokit-mapping-unavailable)"
                   << " cg-display-uuid=unavailable(current-sdk)";
        return diagnostic.str();
    }

    std::vector<std::string> displayRegistryDiagnostics()
    {
        std::vector<std::string> diagnostics;
        CFMutableDictionaryRef matching = IOServiceMatching("IOMobileFramebuffer");
        if (!matching) {
            return diagnostics;
        }

        io_iterator_t iterator = IO_OBJECT_NULL;
        if (IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iterator) != KERN_SUCCESS) {
            return diagnostics;
        }

        while (const io_service_t service = IOIteratorNext(iterator)) {
            CFMutableDictionaryRef properties = nullptr;
            if (IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS &&
                properties) {
                const CFDictionaryRef attributes = dictionaryProperty(properties, CFSTR("DisplayAttributes"));
                const CFDictionaryRef product = dictionaryProperty(attributes, CFSTR("ProductAttributes"));
                const std::string edid_uuid = propertyString(properties, CFSTR("EDID UUID"));
                const CFDictionaryRef transport = dictionaryProperty(properties, CFSTR("Transport"));
                const std::string upstream = scalarProperty(transport, CFSTR("Upstream"));
                const std::string downstream = scalarProperty(transport, CFSTR("Downstream"));
                const std::string manufacturer = scalarProperty(product, CFSTR("ManufacturerID"));
                const std::string product_id = scalarProperty(product, CFSTR("ProductID"));
                const std::string display_name = scalarProperty(product, CFSTR("ProductName"));
                const std::string week = scalarProperty(product, CFSTR("WeekOfManufacture"));
                const std::string year = scalarProperty(product, CFSTR("YearOfManufacture"));
                const std::string dcp_index = scalarProperty(properties, CFSTR("DCPIndex"));
                const std::string external = scalarProperty(properties, CFSTR("external"));
                const std::string native_width = scalarProperty(attributes, CFSTR("NativeFormatHorizontalPixels"));
                const std::string native_height = scalarProperty(attributes, CFSTR("NativeFormatVerticalPixels"));
                const std::string physical_width = scalarProperty(attributes, CFSTR("MaxHorizontalImageSize"));
                const std::string physical_height = scalarProperty(attributes, CFSTR("MaxVerticalImageSize"));
                const bool serial_present = nonZeroNumber(dictionaryValue(product, CFSTR("SerialNumber")));
                std::uint64_t registry_entry_id = 0;
                const kern_return_t entry_id_result = IORegistryEntryGetRegistryEntryID(service, &registry_entry_id);

                // These records are not associated with a QScreen. They are logged as
                // separate evidence only, never used by matching or persistence.
                std::ostringstream diagnostic;
                diagnostic << "registry-class=IOMobileFramebuffer"
                           << " edid-uuid-sha256=" << sha256(edid_uuid) << " registry-entry-id="
                           << (entry_id_result == KERN_SUCCESS ? std::to_string(registry_entry_id) : "unavailable")
                           << " dcp-index=" << (dcp_index.empty() ? "unavailable" : dcp_index)
                           << " external=" << (external.empty() ? "unavailable" : external)
                           << " transport=" << (upstream.empty() ? "unavailable" : upstream) << "-to-"
                           << (downstream.empty() ? "unavailable" : downstream)
                           << " manufacturer=" << (manufacturer.empty() ? "unavailable" : manufacturer)
                           << " product=" << (product_id.empty() ? "unavailable" : product_id)
                           << " name=" << (display_name.empty() ? "unavailable" : display_name)
                           << " manufacture-week=" << (week.empty() ? "unavailable" : week)
                           << " manufacture-year=" << (year.empty() ? "unavailable" : year)
                           << " native-pixels=" << (native_width.empty() ? "unavailable" : native_width) << 'x'
                           << (native_height.empty() ? "unavailable" : native_height)
                           << " physical-cm=" << (physical_width.empty() ? "unavailable" : physical_width) << 'x'
                           << (physical_height.empty() ? "unavailable" : physical_height)
                           << " serial-present=" << (serial_present ? "yes" : "no");
                diagnostics.push_back(diagnostic.str());
                CFRelease(properties);
            }
            IOObjectRelease(service);
        }
        IOObjectRelease(iterator);
        return diagnostics;
    }
#endif

}  // namespace program_display::platform::macos
