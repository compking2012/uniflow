/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2008 Volker Lanz <vl@fidra.de>
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "ScreenConfig.h"

#include "common/QSettingsProxy.h"

#include <QIcon>
#include <QList>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QStringList>

class QSettings;
class QTextStream;
class ScreenSettingsDialog;

//! One physical monitor belonging to a machine, in that machine's own local
//! coordinates (see Screen::monitors()).
struct MonitorTile
{
  QRect rect;
  //! Human-readable name as shown in the OS's own display settings (e.g.
  //! "Built-in Retina Display", "DELL U2720Q", "HDMI-1"); empty if unknown.
  QString name;

  bool operator==(const MonitorTile &o) const
  {
    return rect == o.rect && name == o.name;
  }
};

class Screen : public ScreenConfig
{
  friend class ScreenSettingsDialog;

  friend QDataStream &operator<<(QDataStream &outStream, const Screen &screen)
  {
    return outStream << screen.name() << screen.switchCornerSize() << screen.aliases() << screen.modifiers()
                     << screen.switchCorners() << screen.fixes() << screen.isServer();
  }

  friend QDataStream &operator>>(QDataStream &inStream, Screen &screen)
  {
    return inStream >> screen.m_Name >> screen.m_SwitchCornerSize >> screen.m_Aliases >> screen.m_Modifiers >>
           screen.m_SwitchCorners >> screen.m_Fixes >> screen.m_isServer;
  }

public:
  explicit Screen(const QString &name = QString());

  [[nodiscard]] const QPixmap &pixmap() const
  {
    return m_Pixmap;
  }
  [[nodiscard]] const QString &name() const
  {
    return m_Name;
  }
  [[nodiscard]] const QStringList &aliases() const
  {
    return m_Aliases;
  }

  [[nodiscard]] bool isNull() const
  {
    return m_Name.isEmpty();
  }
  [[nodiscard]] int modifier(int m) const
  {
    return m_Modifiers[m] == static_cast<int>(ScreenConfig::Modifier::DefaultMod) ? m : m_Modifiers[m];
  }
  [[nodiscard]] const QList<int> &modifiers() const
  {
    return m_Modifiers;
  }
  [[nodiscard]] bool switchCorner(int c) const
  {
    return m_SwitchCorners[c];
  }
  [[nodiscard]] const QList<bool> &switchCorners() const
  {
    return m_SwitchCorners;
  }
  [[nodiscard]] int switchCornerSize() const
  {
    return m_SwitchCornerSize;
  }
  [[nodiscard]] bool fix(const Fix f) const
  {
    return m_Fixes[static_cast<int8_t>(f)];
  }
  [[nodiscard]] const QList<bool> &fixes() const
  {
    return m_Fixes;
  }

  void loadSettings(QSettingsProxy &settings);
  void saveSettings(QSettingsProxy &settings) const;
  [[nodiscard]] QString screensSection() const;
  [[nodiscard]] QString aliasesSection() const;

  [[nodiscard]] bool swapped() const
  {
    return m_Swapped;
  }

  void setName(const QString &name)
  {
    m_Name = name;
  }
  [[nodiscard]] bool isServer() const
  {
    return m_isServer;
  }
  void markAsServer()
  {
    m_isServer = true;
  }

  //! Position of this machine's monitor group in the shared canvas
  //! coordinate space, as arranged by the server administrator.  Persisted.
  [[nodiscard]] QPoint canvasPos() const
  {
    return m_CanvasPos;
  }
  void setCanvasPos(const QPoint &pos)
  {
    m_CanvasPos = pos;
    m_HasCanvasPos = true;
  }
  //! false until a canvas position has been assigned (manually, or by
  //! migrating an older grid-based config).  Lets callers distinguish
  //! "never placed" from "placed at the origin".
  [[nodiscard]] bool hasCanvasPos() const
  {
    return m_HasCanvasPos;
  }

  //! This machine's individual monitors, in the machine's own local
  //! coordinates (top-left of the bounding box normalised to 0,0).  This is
  //! always a live snapshot of the machine's real, OS-reported display
  //! layout (or a single placeholder monitor before any data is known) --
  //! it is never user-edited and is not persisted.
  [[nodiscard]] const QList<MonitorTile> &monitors() const
  {
    return m_Monitors;
  }
  //! Sets this machine's monitors from raw OS/IPC-reported tiles, whose
  //! rects are typically in that machine's own virtual-desktop coordinates
  //! (not zero-based).  Normalises them so the bounding box's top-left is
  //! at the origin, matching monitors()'s documented invariant.  Names are
  //! carried through unchanged.
  void setMonitors(const QList<MonitorTile> &monitors);

  bool operator==(const Screen &screen) const;

protected:
  QStringList &aliases()
  {
    return m_Aliases;
  }
  void setModifier(const Modifier m, const int n)
  {
    m_Modifiers[static_cast<int8_t>(m)] = n;
  }
  QList<int> &modifiers()
  {
    return m_Modifiers;
  }
  void addAlias(const QString &alias)
  {
    m_Aliases.append(alias);
  }
  void setSwitchCorner(const SwitchCorner c, const bool on)
  {
    m_SwitchCorners[static_cast<int8_t>(c)] = on;
  }
  QList<bool> &switchCorners()
  {
    return m_SwitchCorners;
  }
  void setSwitchCornerSize(const int val)
  {
    m_SwitchCornerSize = val;
  }
  void setFix(const Fix f, const bool on)
  {
    m_Fixes[static_cast<int8_t>(f)] = on;
  }
  QList<bool> &fixes()
  {
    return m_Fixes;
  }
  void setSwapped(const bool on)
  {
    m_Swapped = on;
  }

private:
  QPixmap m_Pixmap = QIcon::fromTheme("video-display").pixmap(QSize(96, 96));
  QString m_Name = {};
  QStringList m_Aliases = {};
  QList<int> m_Modifiers = {0, 1, 2, 3, 4, 5, 6};
  QList<bool> m_SwitchCorners = {false, false, false, false};
  int m_SwitchCornerSize = 0;
  QList<bool> m_Fixes{false, false, false, false};
  bool m_Swapped = false;
  bool m_isServer = false;
  QPoint m_CanvasPos;
  bool m_HasCanvasPos = false;
  QList<MonitorTile> m_Monitors;
};
