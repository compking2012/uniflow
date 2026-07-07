/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "gui/SystemSettings.h"

#include "common/PlatformInfo.h"

#include <QDesktopServices>
#include <QList>
#include <QPair>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace deskflow::gui {

namespace {

bool tryOpenUrl(const QString &url)
{
  return QDesktopServices::openUrl(QUrl(url));
}

bool tryStartDetached(const QString &program, const QStringList &arguments = {})
{
  return QProcess::startDetached(program, arguments);
}

} // namespace

bool openDisplaySettings()
{
  if (deskflow::platform::isMac()) {
    // "Displays-Settings.extension" verified working on this machine
    // (macOS 26, System Settings' redesigned Displays pane); the older
    // System Preferences scheme is kept as a fallback for earlier releases.
    return tryOpenUrl(QStringLiteral("x-apple.systempreferences:com.apple.Displays-Settings.extension")) ||
           tryOpenUrl(QStringLiteral("x-apple.systempreferences:com.apple.preference.displays"));
  }

  if (deskflow::platform::isWindows()) {
    return tryOpenUrl(QStringLiteral("ms-settings:display"));
  }

  // Linux has no portable "open display settings" launcher; try each
  // desktop environment's own command in turn and stop at the first one
  // that actually starts.
  static const QList<QPair<QString, QStringList>> candidates = {
      {QStringLiteral("gnome-control-center"), {QStringLiteral("display")}},
      {QStringLiteral("kcmshell6"), {QStringLiteral("kcm_kscreen")}},
      {QStringLiteral("kcmshell5"), {QStringLiteral("kcm_kscreen")}},
      {QStringLiteral("xfce4-display-settings"), {}},
      {QStringLiteral("cinnamon-settings"), {QStringLiteral("display")}},
      {QStringLiteral("mate-display-properties"), {}},
  };
  for (const auto &[program, arguments] : candidates) {
    if (tryStartDetached(program, arguments)) {
      return true;
    }
  }
  return false;
}

} // namespace deskflow::gui
