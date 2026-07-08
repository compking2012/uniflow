/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include <QTest>

class MonitorLayoutLinksTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void lShape_generatesHorizontalAndVerticalSubIntervalLinks();
  void sideBySide_fullEdgeLink();
  void nonAdjacent_noLinks();
};
