/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 - 2026 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2008 Volker Lanz <vl@fidra.de>
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ServerConfig.h"

#include "Hotkey.h"
#include "common/Settings.h"

#include <QAbstractButton>
#include <QPushButton>

using enum ScreenConfig::Modifier;
using enum ScreenConfig::SwitchCorner;
using enum ScreenConfig::Fix;

ServerConfig::ServerConfig()
{
  recall();
}

bool ServerConfig::save(const QString &fileName) const
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  save(file);
  file.close();

  return true;
}

bool ServerConfig::operator==(const ServerConfig &sc) const
{
  return m_Screens == sc.m_Screens && //
         m_Hotkeys == sc.m_Hotkeys;   //
}

void ServerConfig::save(QFile &file) const
{
  QTextStream outStream(&file);
  outStream << *this;
}

void ServerConfig::setupScreens()
{
  screens().clear();
  hotkeys().clear();
}

void ServerConfig::commit()
{
  qDebug("committing server config");

  settings().beginGroup("internalConfig");
  settings().remove("");

  settings().beginWriteArray("screens");
  for (int i = 0; i < screens().size(); i++) {
    settings().setArrayIndex(i);
    const auto &screen = screens()[i];
    screen.saveSettings(settings());
    auto screenName = Settings::value(Settings::Core::ComputerName).toString();
    if (screen.isServer() && screenName != screen.name()) {
      Settings::setValue(Settings::Core::ComputerName, screen.name());
    }
  }
  settings().endArray();

  settings().beginWriteArray("hotkeys");
  for (int i = 0; i < hotkeys().size(); i++) {
    settings().setArrayIndex(i);
    hotkeys()[i].saveSettings(settings().get());
  }
  settings().endArray();

  settings().endGroup();
}

void ServerConfig::recall()
{
  qDebug("recalling server config");

  settings().beginGroup("internalConfig");

  // only used to migrate a screen that predates per-monitor canvas
  // positions: an old grid-based config placed screens at array index
  // i = row * oldWidth + column.
  const int oldWidth = Settings::value(Settings::Server::GridWidth).toInt();

  setupScreens();

  int numEntries = settings().beginReadArray("screens");
  for (int i = 0; i < numEntries; i++) {
    settings().setArrayIndex(i);
    Screen s;
    s.loadSettings(settings());
    if (s.isNull()) {
      // empty grid cell from an older, grid-based config; the flat list has
      // no equivalent placeholder
      continue;
    }
    if (getServerName() == s.name()) {
      s.markAsServer();
    }
    if (!s.hasCanvasPos()) {
      const int col = oldWidth > 0 ? i % oldWidth : i;
      const int row = oldWidth > 0 ? i / oldWidth : 0;
      s.setCanvasPos(QPoint(col * 2000, row * 1200));
    }
    screens().append(s);
  }
  settings().endArray();

  int numHotkeys = settings().beginReadArray("hotkeys");
  for (int i = 0; i < numHotkeys; i++) {
    settings().setArrayIndex(i);
    Hotkey h;
    h.loadSettings(settings().get());
    hotkeys().append(h);
  }
  settings().endArray();

  settings().endGroup();
}

QTextStream &operator<<(QTextStream &outStream, const ServerConfig &config)
{
  outStream << "section: screens" << Qt::endl;

  for (const Screen &s : config.screens()) {
    if (!s.isNull())
      outStream << s.screensSection();
  }

  outStream << "end" << Qt::endl << Qt::endl;

  outStream << "section: links" << Qt::endl;

  // links are always derived from the canvas arrangement: every screen's
  // monitors, translated to its chosen canvas position.
  QMap<QString, QList<QRect>> layout;
  for (const Screen &s : config.screens()) {
    if (s.isNull())
      continue;
    QList<QRect> translated;
    for (const QRect &r : s.monitors())
      translated.append(r.translated(s.canvasPos()));
    if (translated.isEmpty())
      translated.append(QRect(s.canvasPos(), QSize(1920, 1080)));
    layout.insert(s.name(), translated);
  }

  QString currentScreen;
  for (const auto &link : deskflow::gui::generateMonitorLinks(layout)) {
    if (link.srcMachine != currentScreen) {
      currentScreen = link.srcMachine;
      outStream << "\t" << currentScreen << ":\n";
    }
    const auto interval = [](double a, double b) {
      return QStringLiteral("(%1,%2)").arg(qRound(a)).arg(qRound(b));
    };
    outStream << "\t\t" << link.side << interval(link.srcStart, link.srcEnd) << " = " << link.dstMachine
              << interval(link.dstStart, link.dstEnd) << Qt::endl;
  }

  outStream << "end" << Qt::endl << Qt::endl;

  outStream << "section: options" << Qt::endl;

  for (const Hotkey &hotkey : config.hotkeys())
    outStream << hotkey;

  outStream << "end" << Qt::endl << Qt::endl;

  return outStream;
}

int ServerConfig::numScreens() const
{
  int rval = 0;

  for (const Screen &s : screens()) {
    if (!s.isNull())
      rval++;
  }

  return rval;
}

QString ServerConfig::getServerName() const
{
  return Settings::value(Settings::Core::ComputerName).toString();
}

void ServerConfig::updateServerName()
{
  for (auto &screen : screens()) {
    if (screen.isServer()) {
      screen.setName(Settings::value(Settings::Core::ComputerName).toString());
      break;
    }
  }
}

QString ServerConfig::configFile() const
{
  return Settings::value(Settings::Server::ExternalConfigFile).toString();
}

bool ServerConfig::useExternalConfig() const
{
  return Settings::value(Settings::Server::ExternalConfig).toBool();
}

bool ServerConfig::screenExists(const QString &screenName) const
{
  bool isExists = false;

  for (const auto &screen : screens()) {
    if (!screen.isNull() && screen.name() == screenName) {
      isExists = true;
      break;
    }
  }

  return isExists;
}

void ServerConfig::addClient(const QString &clientName)
{
  int serverIndex = -1;
  const auto screenName = Settings::value(Settings::Core::ComputerName).toString();

  if (findScreenName(screenName, serverIndex)) {
    m_Screens[serverIndex].markAsServer();
  } else {
    Screen serverScreen(screenName);
    serverScreen.markAsServer();
    m_Screens.addScreen(serverScreen);
  }

  m_Screens.addScreen(Screen(clientName));
}

void ServerConfig::setConfigFile(const QString &configFile) const
{
  Settings::setValue(Settings::Server::ExternalConfigFile, configFile);
}

void ServerConfig::setUseExternalConfig(bool useExternalConfig) const
{
  Settings::setValue(Settings::Server::ExternalConfig, useExternalConfig);
}

bool ServerConfig::findScreenName(const QString &name, int &index)
{
  bool found = false;
  for (int i = 0; i < screens().size(); i++) {
    if (!screens()[i].isNull() && screens()[i].name().compare(name) == 0) {
      index = i;
      found = true;
      break;
    }
  }
  return found;
}

QSettingsProxy &ServerConfig::settings()
{
  return Settings::proxy();
}
