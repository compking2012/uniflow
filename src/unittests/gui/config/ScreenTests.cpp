/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2024 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ScreenTests.h"

#include "common/Settings.h"
#include "gui/config/Screen.h"

void ScreenTests::initTestCase()
{
  QDir dir;
  QVERIFY(dir.mkpath(m_settingsPath));

  QFile oldSettings(m_settingsFile);
  if (oldSettings.exists())
    oldSettings.remove();

  Settings::setSettingsFile(m_settingsFile);
  Settings::setStateFile(m_stateFile);
}

void ScreenTests::basicFunctionality()
{
  Screen screen;
  QVERIFY(screen.isNull());

  screen.setName("stub");
  QVERIFY(!screen.isNull());

  screen.saveSettings(Settings::proxy());
  screen.loadSettings(Settings::proxy());
  QCOMPARE("stub", screen.name());
}

void ScreenTests::canvasPosRoundTrip()
{
  // a screen that was never given a canvas position should not report one
  // after a round trip (must run before anything else ever writes a
  // canvasX/canvasY key, since QSettings keys aren't isolated between the
  // saveSettings() calls in this test)
  Screen noPosition("noPosition");
  QVERIFY(!noPosition.hasCanvasPos());
  noPosition.saveSettings(Settings::proxy());

  Screen reloadedNoPosition;
  reloadedNoPosition.loadSettings(Settings::proxy());
  QVERIFY(!reloadedNoPosition.hasCanvasPos());

  // once set, the position persists across a save/load round trip
  Screen withPosition("withPosition");
  withPosition.setCanvasPos(QPoint(1920, -540));
  QVERIFY(withPosition.hasCanvasPos());
  withPosition.saveSettings(Settings::proxy());

  Screen reloaded;
  reloaded.loadSettings(Settings::proxy());
  QVERIFY(reloaded.hasCanvasPos());
  QCOMPARE(reloaded.canvasPos(), QPoint(1920, -540));
}

void ScreenTests::monitorsAreNormalisedAndNotPersisted()
{
  Screen screen("multiMonitor");

  // raw OS-reported rects are typically NOT zero-based
  screen.setMonitors(
      {MonitorTile{QRect(1000, 500, 1920, 1080), "Left"}, MonitorTile{QRect(2920, 500, 1920, 1080), "Right"}}
  );

  const auto monitors = screen.monitors();
  QCOMPARE(monitors.size(), 2);
  // normalised so the bounding box's top-left is at the origin; names are
  // carried through unchanged
  QCOMPARE(monitors[0].rect, QRect(0, 0, 1920, 1080));
  QCOMPARE(monitors[0].name, QStringLiteral("Left"));
  QCOMPARE(monitors[1].rect, QRect(1920, 0, 1920, 1080));
  QCOMPARE(monitors[1].name, QStringLiteral("Right"));

  // monitors are a runtime-only cache: never persisted
  screen.saveSettings(Settings::proxy());
  Screen reloaded;
  reloaded.loadSettings(Settings::proxy());
  QVERIFY(reloaded.monitors().isEmpty());
}

QTEST_MAIN(ScreenTests)
