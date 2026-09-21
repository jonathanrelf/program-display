// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "display_metadata.hpp"

#if defined(__APPLE__)
#include "platform/macos/display_metadata.hpp"
#endif

#include <QScreen>
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include <QCryptographicHash>
#include <sstream>

namespace {

std::string identityFingerprint(const program_display::domain::DisplayMetadata &metadata)
{
	std::ostringstream source;
	source << metadata.manufacturer << '\n'
	       << metadata.model << '\n'
	       << metadata.serial << '\n'
	       << metadata.name << '\n'
	       << metadata.physical_width_mm << 'x' << metadata.physical_height_mm;
	return QCryptographicHash::hash(QByteArray::fromStdString(source.str()), QCryptographicHash::Sha256)
		.toHex()
		.toStdString();
}

} // namespace
#endif

namespace program_display::qt {

domain::DisplayMetadata displayMetadataFor(QScreen *screen)
{
	domain::DisplayMetadata metadata;
	if (!screen) {
		return metadata;
	}

	metadata.manufacturer = screen->manufacturer().toStdString();
	metadata.model = screen->model().toStdString();
	metadata.serial = screen->serialNumber().toStdString();
	metadata.name = screen->name().toStdString();
	metadata.physical_width_mm = qRound(screen->physicalSize().width());
	metadata.physical_height_mm = qRound(screen->physicalSize().height());
	const QSize pixels = screen->size() * screen->devicePixelRatio();
	metadata.pixel_width = pixels.width();
	metadata.pixel_height = pixels.height();

#if defined(__APPLE__)
	platform::macos::supplementDisplayMetadata(screen, metadata);
#endif

	return metadata;
}

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
std::string displayDiagnosticFor(QScreen *screen)
{
	const domain::DisplayMetadata metadata = displayMetadataFor(screen);
	std::ostringstream diagnostic;
	diagnostic << "qt-name=" << (metadata.name.empty() ? "unnamed" : metadata.name)
		   << " manufacturer=" << (metadata.manufacturer.empty() ? "absent" : metadata.manufacturer)
		   << " model=" << (metadata.model.empty() ? "absent" : metadata.model)
		   << " serial-present=" << (!metadata.serial.empty() ? "yes" : "no")
		   << " physical-mm=" << metadata.physical_width_mm << 'x' << metadata.physical_height_mm
		   << " pixels=" << metadata.pixel_width << 'x' << metadata.pixel_height
		   << " identity-sha256=" << identityFingerprint(metadata);
#if defined(__APPLE__)
	diagnostic << ' ' << platform::macos::displayDiagnosticFor(screen);
#endif
	return diagnostic.str();
}

std::vector<std::string> displayRegistryDiagnostics()
{
#if defined(__APPLE__)
	return platform::macos::displayRegistryDiagnostics();
#else
	return {};
#endif
}
#endif

} // namespace program_display::qt
