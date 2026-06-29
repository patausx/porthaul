// QR connect widget for ftpd fork (descry-ftp)
//
// Renders a QR code (from a URL/text) directly via Dear ImGui DrawList,
// drawing each QR module as a filled rectangle. No textures, no platform
// dependency — works on 3DS (citro3d), Switch (deko3d) and Linux (GL).
//
// Public domain wrapper around Nayuki qrcodegen (MIT).

#pragma once

#ifndef CLASSIC

#include <string>

namespace qr
{
/// \brief Draw a QR code for the given text using the current ImGui window.
/// \param text_ Payload to encode (e.g. "ftp://192.168.1.x:5000/")
/// \param pixelSize_ Target size in pixels for the whole QR (auto if <= 0)
/// \return true if the QR was generated and drawn, false on encode failure
bool drawQR (std::string const &text_, float pixelSize_ = 0.0f);
}

#endif // CLASSIC
