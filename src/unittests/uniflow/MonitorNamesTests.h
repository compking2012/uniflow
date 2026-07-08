/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include <QTest>

class MonitorNamesTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void roundTrip_multipleNames();
  void roundTrip_emptyNames();
  void split_fewerSegmentsThanCount_padsWithEmpty();
  void split_moreSegmentsThanCount_discardsExtras();
  void join_emptyList_producesEmptyString();
};
