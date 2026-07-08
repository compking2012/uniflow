/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include <QTest>

class MonitorGeometryTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void monitorContaining_pointInside_returnsIndex();
  void monitorContaining_deadZone_returnsMinusOne();
  void findMonitor_deadZone_returnsNearest();
  void hasSameMachineNeighbor_internalEdge_true();
  void hasSameMachineNeighbor_exposedEdge_false();
  void chooseEntryMonitor_enterLeftEdge_picksMonitorAtThatRow();
  void chooseEntryMonitor_enterLeftEdge_picksLeftmostWhenMultiple();
  void chooseEntryMonitor_enterTopEdge_picksColumnMonitor();
  void chooseEntryMonitor_noSpanningMonitor_returnsMinusOne();
};
