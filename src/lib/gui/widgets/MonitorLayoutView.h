/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "gui/config/ScreenList.h"

#include <QGraphicsView>
#include <QMap>
#include <QString>

class QGraphicsScene;
class QGraphicsItemGroup;
class QLabel;
class QMouseEvent;
class QContextMenuEvent;
class QKeyEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QResizeEvent;
class QPainter;

namespace deskflow::gui {

//! Embeddable canvas showing every configured machine's monitors as a
//! rigid, draggable group positioned at the machine's chosen canvas
//! position.  Replaces the fixed NxM computer grid (ScreenSetupModel /
//! ScreenSetupView) with a free arrangement of each machine's real,
//! OS-reported monitor geometry.  Binds directly to a ScreenList so edits
//! (drag, add, remove, per-computer settings) apply straight to the live
//! server configuration -- there is no separate "generate links" step;
//! cross-machine links are always derived from the current arrangement.
class MonitorLayoutView : public QGraphicsView
{
  Q_OBJECT

public:
  explicit MonitorLayoutView(QWidget *parent = nullptr);

  static const QString &mimeType()
  {
    return m_MimeType;
  }

  //! Must be called once, after construction, before the view is used
  //! (mirrors ScreenSetupView::setModel()).  The referenced list must
  //! outlive this view.
  void setScreenList(ScreenList *screens);

  //! The widget a computer's tiles are dropped on to delete it (styled as a
  //! trash can, typically a QLabel placed in a corner of the parent tab).
  //! This view does not take ownership and does not require the widget to
  //! accept drops -- deletion is decided by comparing the drop point against
  //! the widget's on-screen geometry, not by a real cross-widget QDrag.
  void setTrashWidget(QLabel *trash);

  //! Rebuilds the whole canvas from the current ScreenList contents.  Call
  //! after screens are added/removed, or a screen's name changes.
  void rebuild();

  //! Updates just the named machine's monitor tiles (shape only); its
  //! canvas position, and every other machine's group, are left untouched.
  void refreshMonitors(const QString &name, const QList<MonitorTile> &tiles);

  //! Snaps a group's proposed position so its monitors' edges align with
  //! the nearest opposing edges of other machines' monitors.  Called by
  //! group items while being dragged.
  QPointF snapPosition(QGraphicsItemGroup *group, const QPointF &proposedPos) const;

  //! Called by a group item whenever its position changes (including
  //! during a drag); persists the new position onto the matching Screen.
  void commitGroupPosition(const QString &name, const QPointF &pos);

  //! true if \p globalPos (screen coordinates) falls within the trash
  //! widget's on-screen bounds.  false if no trash widget was set.
  bool isOverTrash(const QPoint &globalPos) const;

  //! Highlights (or un-highlights) the trash widget to indicate whether
  //! releasing the mouse right now, at \p globalPos, would delete the
  //! computer currently being dragged.
  void updateTrashHover(const QPoint &globalPos);

  //! Clears any trash-hover highlight left over from a finished drag.
  void clearTrashHover();

  //! Removes the named computer (all its monitors) from the configuration.
  void removeScreen(const QString &name);

Q_SIGNALS:
  void screensChanged();

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
  void addGroupFor(const Screen &screen);
  Screen *findScreen(const QString &name);
  //! finds the machine name owning the top-level group under a scene item,
  //! or an empty string if none
  QString groupNameAt(const QPoint &viewportPos) const;

  ScreenList *m_screens = nullptr;
  QGraphicsScene *m_scene = nullptr;
  QMap<QString, QGraphicsItemGroup *> m_groups;
  QLabel *m_trashWidget = nullptr;
  bool m_trashArmed = false;

  static const QString m_MimeType;
};

} // namespace deskflow::gui
