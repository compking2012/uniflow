/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "MonitorNamesTests.h"

#include "uniflow/MonitorNames.h"

using deskflow::joinMonitorNames;
using deskflow::splitMonitorNames;

void MonitorNamesTests::roundTrip_multipleNames()
{
  const std::vector<std::string> names = {"Built-in Retina Display", "DELL U2720Q", "HDMI-1"};
  const std::string joined = joinMonitorNames(names);
  QCOMPARE(splitMonitorNames(joined, names.size()), names);
}

void MonitorNamesTests::roundTrip_emptyNames()
{
  // some monitors may have no resolvable name at all
  const std::vector<std::string> names = {"", "Second Monitor", ""};
  const std::string joined = joinMonitorNames(names);
  QCOMPARE(splitMonitorNames(joined, names.size()), names);
}

void MonitorNamesTests::split_fewerSegmentsThanCount_padsWithEmpty()
{
  // e.g. an older peer that reported fewer names than monitors
  const std::string joined = joinMonitorNames({"A"});
  const std::vector<std::string> expected = {"A", "", ""};
  QCOMPARE(splitMonitorNames(joined, 3), expected);
}

void MonitorNamesTests::split_moreSegmentsThanCount_discardsExtras()
{
  const std::string joined = joinMonitorNames({"A", "B", "C"});
  const std::vector<std::string> expected = {"A", "B"};
  QCOMPARE(splitMonitorNames(joined, 2), expected);
}

void MonitorNamesTests::join_emptyList_producesEmptyString()
{
  QCOMPARE(joinMonitorNames({}), std::string());
  QVERIFY(splitMonitorNames("", 0).empty());
}

QTEST_MAIN(MonitorNamesTests)
