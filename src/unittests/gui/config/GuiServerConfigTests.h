/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include <QTest>

class GuiServerConfigTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  // Tests are run in order top to bottom
  void initTestCase();
  void linksAreDerivedFromCanvasArrangement();
  void screenWithoutMonitors_fallsBackToFullBoxLink();

private:
  inline static const QString m_settingsPath = QStringLiteral("tmp/test-guiserverconfig");
  inline static const QString m_settingsFile = QStringLiteral("%1/Uniflow.conf").arg(m_settingsPath);
  inline static const QString m_stateFile = QStringLiteral("%1/Uniflow.state").arg(m_settingsPath);
};
