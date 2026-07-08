/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "MonitorLayoutLinksTests.h"

#include "gui/config/MonitorLayoutLinks.h"

using namespace deskflow::gui;

namespace {

bool hasLink(
    const QList<MonitorLink> &links, const QString &src, const QString &side, double s0, double s1, const QString &dst,
    double d0, double d1
)
{
  return links.contains(MonitorLink{src, side, s0, s1, dst, d0, d1});
}

} // namespace

void MonitorLayoutLinksTests::lShape_generatesHorizontalAndVerticalSubIntervalLinks()
{
  // server: A|B on top, C directly below B (L-shape); client D below A / left of C
  QMap<QString, QList<QRect>> layout;
  layout.insert(
      "server", {QRect(0, 0, 1920, 1080), QRect(1920, 0, 1920, 1080), QRect(1920, 1080, 1920, 1080)}
  );
  layout.insert("D", {QRect(0, 1080, 1920, 1080)});

  const auto links = generateMonitorLinks(layout);

  QCOMPARE(links.size(), 4);
  // C's left edge (lower half of server) <-> D's right edge
  QVERIFY(hasLink(links, "server", "left", 50, 100, "D", 0, 100));
  QVERIFY(hasLink(links, "D", "right", 0, 100, "server", 50, 100));
  // A's bottom edge (left half of server) <-> D's top edge
  QVERIFY(hasLink(links, "server", "down", 0, 50, "D", 0, 100));
  QVERIFY(hasLink(links, "D", "up", 0, 100, "server", 0, 50));
}

void MonitorLayoutLinksTests::sideBySide_fullEdgeLink()
{
  // two single-monitor machines, client to the right of server
  QMap<QString, QList<QRect>> layout;
  layout.insert("server", {QRect(0, 0, 1920, 1080)});
  layout.insert("client", {QRect(1920, 0, 1920, 1080)});

  const auto links = generateMonitorLinks(layout);

  QCOMPARE(links.size(), 2);
  QVERIFY(hasLink(links, "server", "right", 0, 100, "client", 0, 100));
  QVERIFY(hasLink(links, "client", "left", 0, 100, "server", 0, 100));
}

void MonitorLayoutLinksTests::nonAdjacent_noLinks()
{
  QMap<QString, QList<QRect>> layout;
  layout.insert("server", {QRect(0, 0, 1920, 1080)});
  layout.insert("client", {QRect(5000, 0, 1920, 1080)});

  QCOMPARE(generateMonitorLinks(layout).size(), 0);
}

QTEST_MAIN(MonitorLayoutLinksTests)
