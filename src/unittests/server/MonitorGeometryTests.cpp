/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "MonitorGeometryTests.h"

#include "server/MonitorGeometry.h"

#include <vector>

using namespace deskflow::server;

namespace {

// The bug scenario's server desktop: A|B on the top row, C directly below B,
// leaving the bottom-left quadrant a dead zone (an L-shape).  All monitors
// 1920x1080.
//   A = index 0, B = index 1, C = index 2
std::vector<MonitorInfo> lShapeServer()
{
  return {
      MonitorInfo{0, 0, 1920, 1080},    // A: top-left
      MonitorInfo{1920, 0, 1920, 1080}, // B: top-right
      MonitorInfo{1920, 1080, 1920, 1080}, // C: below B
  };
}

} // namespace

void MonitorGeometryTests::monitorContaining_pointInside_returnsIndex()
{
  const auto monitors = lShapeServer();
  QCOMPARE(monitorContaining(monitors, 1000, 500), 0);  // in A
  QCOMPARE(monitorContaining(monitors, 2500, 500), 1);  // in B
  QCOMPARE(monitorContaining(monitors, 2500, 1500), 2); // in C
}

void MonitorGeometryTests::monitorContaining_deadZone_returnsMinusOne()
{
  const auto monitors = lShapeServer();
  // bottom-left quadrant is not covered by any monitor
  QCOMPARE(monitorContaining(monitors, 500, 1500), -1);
}

void MonitorGeometryTests::findMonitor_deadZone_returnsNearest()
{
  const auto monitors = lShapeServer();
  // (500,1500) is in the dead zone; A's nearest edge is 420px away (below A),
  // C's nearest edge is 1420px away (left of C) -> A is nearest
  QCOMPARE(findMonitor(monitors, 500, 1500), 0);
}

void MonitorGeometryTests::hasSameMachineNeighbor_internalEdge_true()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // B's left edge borders A (same machine) at y=500 -> internal, no jump
  QVERIFY(hasSameMachineNeighbor(monitors, 1, Left, 500));
  // C's top edge borders B (same machine) at x=2500 -> internal, no jump
  QVERIFY(hasSameMachineNeighbor(monitors, 2, Top, 2500));
}

void MonitorGeometryTests::hasSameMachineNeighbor_exposedEdge_false()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // C's left edge (x=1920, y in 1080..2160) borders the dead zone -> exposed
  QVERIFY(!hasSameMachineNeighbor(monitors, 2, Left, 1512));
  // A's bottom edge borders the dead zone -> exposed
  QVERIFY(!hasSameMachineNeighbor(monitors, 0, Bottom, 500));
}

void MonitorGeometryTests::chooseEntryMonitor_enterLeftEdge_picksMonitorAtThatRow()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // entering the machine's left edge (travelling Right) at y=1512 -> only C
  // spans that row, so the cursor must land on C (index 2), not the dead zone
  QCOMPARE(chooseEntryMonitor(monitors, Right, 1512), 2);
}

void MonitorGeometryTests::chooseEntryMonitor_enterLeftEdge_picksLeftmostWhenMultiple()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // at y=500 both A and B span the row; entering the left edge picks the
  // leftmost (A, index 0)
  QCOMPARE(chooseEntryMonitor(monitors, Right, 500), 0);
}

void MonitorGeometryTests::chooseEntryMonitor_enterTopEdge_picksColumnMonitor()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // entering the top edge (travelling Bottom) at x=2500: B and C span that
  // column; topmost (B, index 1) wins
  QCOMPARE(chooseEntryMonitor(monitors, Bottom, 2500), 1);
  // at x=500 only A spans that column
  QCOMPARE(chooseEntryMonitor(monitors, Bottom, 500), 0);
}

void MonitorGeometryTests::chooseEntryMonitor_noSpanningMonitor_returnsMinusOne()
{
  const auto monitors = lShapeServer();
  using enum Direction;
  // y=3000 is below every monitor -> nothing spans it
  QCOMPARE(chooseEntryMonitor(monitors, Right, 3000), -1);
}

QTEST_MAIN(MonitorGeometryTests)
