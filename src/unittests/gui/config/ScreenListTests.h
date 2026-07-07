/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include <QTest>

class ScreenListTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void firstScreen_placedAtOrigin();
  void secondScreen_placedToTheRightWithoutOverlap();
  void screenWithExplicitPosition_heuristicSkipped();
};
