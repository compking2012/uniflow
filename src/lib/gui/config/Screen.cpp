/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2008 Volker Lanz <vl@fidra.de>
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "Screen.h"
#include "config/ScreenConfig.h"
#include <common/Settings.h>

using enum ScreenConfig::Modifier;
using enum ScreenConfig::SwitchCorner;
using enum ScreenConfig::Fix;

Screen::Screen(const QString &name)
{
  setName(name);
}

void Screen::loadSettings(QSettingsProxy &settings)
{
  const auto name = settings.value("name").toString();
  setName(name);

  if (name.isEmpty())
    return;

  setSwitchCornerSize(settings.value("switchCornerSize").toInt());

  readSettings(settings, modifiers(), "modifier", static_cast<int>(DefaultMod), static_cast<int>(NumModifiers));
  readSettings(settings, switchCorners(), "switchCorner", false, static_cast<int>(NumSwitchCorners));
  readSettings(settings, fixes(), "fix", 0, static_cast<int>(NumFixes));

  m_Aliases = Settings::value(Settings::Screen::Aliases.arg(name)).toStringList();

  if (settings.contains("canvasX") && settings.contains("canvasY")) {
    m_CanvasPos = QPoint(settings.value("canvasX").toInt(), settings.value("canvasY").toInt());
    m_HasCanvasPos = true;
  } else {
    m_HasCanvasPos = false;
  }
}

void Screen::saveSettings(QSettingsProxy &settings) const
{

  const auto screenName = name();
  settings.setValue("name", screenName);

  if (screenName.isEmpty())
    return;

  Settings::setValue(Settings::Screen::Aliases.arg(screenName), m_Aliases);

  settings.setValue("switchCornerSize", switchCornerSize());

  writeSettings(settings, modifiers(), "modifier");
  writeSettings(settings, switchCorners(), "switchCorner");
  writeSettings(settings, fixes(), "fix");

  if (m_HasCanvasPos) {
    settings.setValue("canvasX", m_CanvasPos.x());
    settings.setValue("canvasY", m_CanvasPos.y());
  }
}

QString Screen::screensSection() const
{
  const auto lineTemplate = QStringLiteral("\t\t%1 = %2\n");

  QString out = QStringLiteral("\t%1:\n").arg(name());
  for (int i = 0; i < modifiers().size(); i++) {
    if (modifier(i) != i)
      out.append(lineTemplate.arg(modifierName(i), modifierName(modifier(i))));
  }

  for (int i = 0; i < fixes().size(); i++)
    out.append(lineTemplate.arg(fixName(i), fixes().at(i) ? QStringLiteral("true") : QStringLiteral("false")));

  auto corners = QStringLiteral("none");
  for (int i = 0; i < switchCorners().size(); i++) {
    if (switchCorners()[i])
      corners.append(QStringLiteral(" +%1 ").arg(switchCornerName(i)));
  }
  out.append(lineTemplate.arg(QStringLiteral("switchCorners"), corners));

  out.append(lineTemplate.arg(QStringLiteral("switchCornerSize"), QString::number(switchCornerSize())));

  return out;
}

bool Screen::operator==(const Screen &screen) const
{
  return m_Name == screen.m_Name && m_Aliases == screen.m_Aliases && m_Modifiers == screen.m_Modifiers &&
         m_SwitchCorners == screen.m_SwitchCorners && m_SwitchCornerSize == screen.m_SwitchCornerSize &&
         m_Fixes == screen.m_Fixes && m_Swapped == screen.m_Swapped && m_isServer == screen.m_isServer &&
         m_HasCanvasPos == screen.m_HasCanvasPos &&
         (!m_HasCanvasPos || m_CanvasPos == screen.m_CanvasPos);
}

void Screen::setMonitors(const QList<MonitorTile> &monitors)
{
  QRect box;
  for (const auto &tile : monitors) {
    box = box.isNull() ? tile.rect : box.united(tile.rect);
  }
  if (box.isNull()) {
    m_Monitors = monitors;
    return;
  }

  QList<MonitorTile> normalised;
  normalised.reserve(monitors.size());
  for (const auto &tile : monitors) {
    normalised.append(MonitorTile{tile.rect.translated(-box.topLeft()), tile.name});
  }
  m_Monitors = normalised;
}
