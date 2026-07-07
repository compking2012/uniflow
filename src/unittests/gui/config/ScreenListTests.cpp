/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ScreenListTests.h"

#include "gui/config/ScreenList.h"

void ScreenListTests::firstScreen_placedAtOrigin()
{
  ScreenList list;
  list.addScreen(Screen("A"));

  QCOMPARE(list.size(), 1);
  QVERIFY(list[0].hasCanvasPos());
  QCOMPARE(list[0].canvasPos(), QPoint(0, 0));
}

void ScreenListTests::secondScreen_placedToTheRightWithoutOverlap()
{
  ScreenList list;
  list.addScreen(Screen("A"));
  list.addScreen(Screen("B"));

  QCOMPARE(list.size(), 2);

  // both screens have no reported monitors yet, so each defaults to a
  // single 1920x1080 placeholder at its canvas position
  const QRect boxA(list[0].canvasPos(), QSize(1920, 1080));
  const QRect boxB(list[1].canvasPos(), QSize(1920, 1080));

  QVERIFY(!boxA.intersects(boxB));
  QVERIFY(list[1].canvasPos().x() > boxA.right());
}

void ScreenListTests::screenWithExplicitPosition_heuristicSkipped()
{
  ScreenList list;
  list.addScreen(Screen("A"));

  Screen b("B");
  b.setCanvasPos(QPoint(5000, 5000));
  list.addScreen(b);

  QCOMPARE(list[1].canvasPos(), QPoint(5000, 5000));
}

QTEST_MAIN(ScreenListTests)
