/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "MonitorLayoutView.h"

#include "dialogs/ScreenSettingsDialog.h"

#include <QBrush>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QGraphicsItemGroup>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <cmath>

namespace deskflow::gui {

const QString MonitorLayoutView::m_MimeType = "application/x-deskflow-screen";

namespace {

constexpr double kSnapDistance = 80.0; // scene pixels within which edges snap

//! A movable group of one machine's monitors that snaps its edges to the
//! monitors of other machines while being dragged, and reports its screen
//! name back to the owning view.
class MonitorGroupItem : public QGraphicsItemGroup
{
public:
  MonitorGroupItem(MonitorLayoutView *view, QString screenName) : m_view(view), m_screenName(std::move(screenName))
  {
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setHandlesChildEvents(true);
  }

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
  {
    if (change == ItemPositionChange && scene() != nullptr) {
      return m_view->snapPosition(this, value.toPointF());
    }
    if (change == ItemPositionHasChanged && scene() != nullptr) {
      m_view->commitGroupPosition(m_screenName, pos());
    }
    return QGraphicsItemGroup::itemChange(change, value);
  }

private:
  MonitorLayoutView *m_view;
  QString m_screenName;
};

QColor machineColor(const QString &name)
{
  static const QList<QColor> palette = {
      QColor(64, 128, 200, 160), QColor(80, 180, 120, 160), QColor(200, 140, 60, 160),
      QColor(160, 100, 200, 160), QColor(200, 90, 110, 160), QColor(90, 170, 190, 160)
  };
  return palette[static_cast<uint>(qHash(name)) % static_cast<uint>(palette.size())];
}

//! walks up from a scene item to its top-level parent (the machine group)
QGraphicsItem *topLevelItem(QGraphicsItem *item)
{
  while (item != nullptr && item->parentItem() != nullptr) {
    item = item->parentItem();
  }
  return item;
}

} // namespace

MonitorLayoutView::MonitorLayoutView(QWidget *parent) : QGraphicsView(parent)
{
  m_scene = new QGraphicsScene(this);
  setScene(m_scene);
  setRenderHint(QPainter::Antialiasing, true);
  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setDragMode(QGraphicsView::RubberBandDrag);
}

void MonitorLayoutView::setScreenList(ScreenList *screens)
{
  m_screens = screens;
  rebuild();
}

void MonitorLayoutView::addGroupFor(const Screen &screen)
{
  auto *group = new MonitorGroupItem(this, screen.name());
  m_scene->addItem(group);

  QList<QRect> rects = screen.monitors();
  if (rects.isEmpty()) {
    rects = {QRect(0, 0, 1920, 1080)};
  }

  for (int m = 0; m < rects.size(); ++m) {
    auto *rectItem = new QGraphicsRectItem(QRectF(rects[m]));
    rectItem->setBrush(QBrush(machineColor(screen.name())));
    rectItem->setPen(QPen(Qt::black, 2));

    auto *label = new QGraphicsSimpleTextItem(
        rects.size() > 1 ? QStringLiteral("%1 #%2").arg(screen.name()).arg(m + 1) : screen.name()
    );
    label->setParentItem(rectItem);
    label->setPos(rects[m].x() + 8, rects[m].y() + 8);

    group->addToGroup(rectItem);
  }

  group->setPos(screen.canvasPos());
  m_groups.insert(screen.name(), group);
}

void MonitorLayoutView::rebuild()
{
  if (m_scene == nullptr || m_screens == nullptr) {
    return;
  }

  m_scene->clear();
  m_groups.clear();

  for (const Screen &s : *m_screens) {
    if (!s.isNull()) {
      addGroupFor(s);
    }
  }

  const QRectF bounds = m_scene->itemsBoundingRect().adjusted(-200, -200, 200, 200);
  m_scene->setSceneRect(bounds.isEmpty() ? QRectF(-200, -200, 2000, 1400) : bounds);
}

void MonitorLayoutView::refreshMonitors(const QString &name, const QList<QRect> &rects)
{
  if (Screen *s = findScreen(name)) {
    s->setMonitors(rects);
  }
  // a full rebuild re-renders every group at its current (unchanged)
  // canvasPos, so the arrangement in progress is preserved
  rebuild();
}

Screen *MonitorLayoutView::findScreen(const QString &name)
{
  if (m_screens == nullptr) {
    return nullptr;
  }
  for (auto &s : *m_screens) {
    if (s.name() == name) {
      return &s;
    }
  }
  return nullptr;
}

QPointF MonitorLayoutView::snapPosition(QGraphicsItemGroup *group, const QPointF &proposedPos) const
{
  const QPointF delta = proposedPos - group->pos();

  double bestDx = 0.0;
  double bestDy = 0.0;
  double bestAbsDx = kSnapDistance + 1.0;
  double bestAbsDy = kSnapDistance + 1.0;

  const auto childRects = group->childItems();
  for (auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
    QGraphicsItemGroup *other = it.value();
    if (other == group) {
      continue;
    }
    for (const QGraphicsItem *a : childRects) {
      const QRectF ar = a->sceneBoundingRect().translated(delta);
      for (const QGraphicsItem *b : other->childItems()) {
        const QRectF br = b->sceneBoundingRect();

        // x-snap when the rects overlap vertically
        if (ar.bottom() > br.top() && ar.top() < br.bottom()) {
          for (double d : {br.left() - ar.right(), br.right() - ar.left()}) {
            if (std::fabs(d) < bestAbsDx) {
              bestAbsDx = std::fabs(d);
              bestDx = d;
            }
          }
        }
        // y-snap when the rects overlap horizontally
        if (ar.right() > br.left() && ar.left() < br.right()) {
          for (double d : {br.top() - ar.bottom(), br.bottom() - ar.top()}) {
            if (std::fabs(d) < bestAbsDy) {
              bestAbsDy = std::fabs(d);
              bestDy = d;
            }
          }
        }
      }
    }
  }

  QPointF snapped = proposedPos;
  if (bestAbsDx <= kSnapDistance) {
    snapped.rx() += bestDx;
  }
  if (bestAbsDy <= kSnapDistance) {
    snapped.ry() += bestDy;
  }
  return snapped;
}

void MonitorLayoutView::commitGroupPosition(const QString &name, const QPointF &pos)
{
  if (Screen *s = findScreen(name)) {
    s->setCanvasPos(pos.toPoint());
  }
  Q_EMIT screensChanged();
}

QString MonitorLayoutView::groupNameAt(const QPoint &viewportPos) const
{
  QGraphicsItem *item = itemAt(viewportPos);
  if (item == nullptr) {
    return {};
  }
  const QGraphicsItem *top = topLevelItem(item);
  for (auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
    if (it.value() == top) {
      return it.key();
    }
  }
  return {};
}

void MonitorLayoutView::mouseDoubleClickEvent(QMouseEvent *event)
{
  const QString name = groupNameAt(event->pos());
  if (name.isEmpty()) {
    QGraphicsView::mouseDoubleClickEvent(event);
    return;
  }

  if (Screen *s = findScreen(name)) {
    ScreenSettingsDialog dlg(this, s, m_screens);
    if (dlg.exec() == QDialog::Accepted) {
      rebuild();
      Q_EMIT screensChanged();
    }
  }
}

void MonitorLayoutView::contextMenuEvent(QContextMenuEvent *event)
{
  const QString name = groupNameAt(event->pos());
  if (name.isEmpty()) {
    return;
  }

  QMenu menu(this);
  QAction *removeAction = menu.addAction(tr("Remove Computer"));
  if (menu.exec(event->globalPos()) == removeAction) {
    removeScreen(name);
  }
}

void MonitorLayoutView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
    const auto selected = m_scene->selectedItems();
    if (!selected.isEmpty()) {
      const QGraphicsItem *top = topLevelItem(selected.first());
      for (auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
        if (it.value() == top) {
          removeScreen(it.key());
          return;
        }
      }
    }
  }
  QGraphicsView::keyPressEvent(event);
}

void MonitorLayoutView::removeScreen(const QString &name)
{
  if (m_screens == nullptr) {
    return;
  }
  for (int i = 0; i < m_screens->size(); ++i) {
    if ((*m_screens)[i].name() == name) {
      m_screens->removeAt(i);
      break;
    }
  }
  rebuild();
  Q_EMIT screensChanged();
}

void MonitorLayoutView::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat(mimeType())) {
    event->acceptProposedAction();
  } else {
    event->ignore();
  }
}

void MonitorLayoutView::dragMoveEvent(QDragMoveEvent *event)
{
  if (event->mimeData()->hasFormat(mimeType())) {
    event->acceptProposedAction();
  } else {
    event->ignore();
  }
}

void MonitorLayoutView::dropEvent(QDropEvent *event)
{
  if (m_screens == nullptr || !event->mimeData()->hasFormat(mimeType())) {
    event->ignore();
    return;
  }

  QByteArray encoded = event->mimeData()->data(mimeType());
  QDataStream stream(&encoded, QIODevice::ReadOnly);
  int sourceColumn = -1;
  int sourceRow = -1;
  Screen droppedScreen;
  stream >> sourceColumn >> sourceRow >> droppedScreen;

  if (droppedScreen.isNull()) {
    event->ignore();
    return;
  }

  droppedScreen.setCanvasPos(mapToScene(event->position().toPoint()).toPoint());
  m_screens->addScreen(droppedScreen);

  rebuild();
  Q_EMIT screensChanged();
  event->acceptProposedAction();
}

void MonitorLayoutView::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);
  if (m_scene != nullptr && !m_scene->sceneRect().isEmpty()) {
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
  }
}

} // namespace deskflow::gui
