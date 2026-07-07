/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "GuiServerConfigTests.h"

#include "common/Settings.h"
#include "gui/config/ServerConfig.h"

void GuiServerConfigTests::initTestCase()
{
  QDir dir;
  QVERIFY(dir.mkpath(m_settingsPath));

  QFile oldSettings(m_settingsFile);
  if (oldSettings.exists())
    oldSettings.remove();

  Settings::setSettingsFile(m_settingsFile);
  Settings::setStateFile(m_stateFile);
}

void GuiServerConfigTests::linksAreDerivedFromCanvasArrangement()
{
  ServerConfig config;

  // server: A|B on the top row, C directly below B (L-shape, all 1920x1080)
  Screen server("server");
  server.setMonitors({QRect(0, 0, 1920, 1080), QRect(1920, 0, 1920, 1080), QRect(1920, 1080, 1920, 1080)});
  server.setCanvasPos(QPoint(0, 0));
  config.addScreen(server);

  // D: a single 1920x1080 monitor, positioned below A / left of C -- the
  // same L-shape scenario proven correct by MonitorLayoutLinksTests
  Screen d("D");
  d.setMonitors({QRect(0, 0, 1920, 1080)});
  d.setCanvasPos(QPoint(0, 1080));
  config.addScreen(d);

  QString output;
  QTextStream stream(&output);
  stream << config;

  // C's left edge <-> D's right edge
  QVERIFY(output.contains("left(50,100) = D(0,100)"));
  QVERIFY(output.contains("right(0,100) = server(50,100)"));
  // A's bottom edge <-> D's top edge
  QVERIFY(output.contains("down(0,50) = D(0,100)"));
  QVERIFY(output.contains("up(0,100) = server(0,50)"));
}

void GuiServerConfigTests::screenWithoutMonitors_fallsBackToFullBoxLink()
{
  ServerConfig config;

  Screen server("server2");
  server.setMonitors({QRect(0, 0, 1920, 1080)});
  server.setCanvasPos(QPoint(0, 0));
  config.addScreen(server);

  // a newly-added, not-yet-connected client: no monitors reported yet, so
  // operator<< must fall back to a full 1920x1080 box at its canvas position
  Screen notConnected("notConnected");
  notConnected.setCanvasPos(QPoint(1920, 0));
  config.addScreen(notConnected);

  QString output;
  QTextStream stream(&output);
  stream << config;

  QVERIFY(output.contains("right(0,100) = notConnected(0,100)"));
  QVERIFY(output.contains("left(0,100) = server2(0,100)"));
}

QTEST_MAIN(GuiServerConfigTests)
