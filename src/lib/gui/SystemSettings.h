/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

namespace deskflow::gui {

//! Attempts to open the OS's native display/monitor settings UI (e.g.
//! System Settings > Displays on macOS, Settings > Display on Windows).
//! Tries a platform-appropriate sequence of URLs/commands and returns true
//! if one of them was launched successfully.  Best-effort: on Linux in
//! particular, desktop-environment fragmentation means this may find
//! nothing to launch.
bool openDisplaySettings();

} // namespace deskflow::gui
