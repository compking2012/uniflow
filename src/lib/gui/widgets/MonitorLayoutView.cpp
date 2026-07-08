/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "MonitorLayoutView.h"

#include "dialogs/ScreenSettingsDialog.h"
#include "gui/SystemSettings.h"

#include <QBrush>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFontMetricsF>
#include <QDropEvent>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>
#include <cmath>

namespace deskflow::gui {

const QString MonitorLayoutView::m_MimeType = "application/x-deskflow-screen";

namespace {

constexpr double kSnapDistance = 80.0; // scene pixels within which edges snap
constexpr int kTileCornerRadius = 10;
constexpr int kGridStep = 40;
constexpr double kTilePaddingRatio = 0.06;

//! A movable group of one machine's monitors that snaps its edges to the
//! monitors of other machines while being dragged, and reports its screen
//! name back to the owning view.  Dropping the group onto the owning view's
//! trash widget deletes the machine.
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

  //! The default QGraphicsItemGroup::shape() is just the rectangular
  //! boundingRect(), which for an L-shaped arrangement of tiles would
  //! include the empty corner.  Hit-testing (context menu, double-click,
  //! and Qt's own itemAt()) must only consider the tiles actually drawn, so
  //! another machine's tiles can be moved into that empty corner instead of
  //! being blocked by this group's bounding box.
  QPainterPath shape() const override
  {
    QPainterPath path;
    for (const QGraphicsItem *child : childItems()) {
      path.addPath(mapFromItem(child, child->shape()));
    }
    return path;
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

  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
  {
    QGraphicsItemGroup::mouseMoveEvent(event);
    m_view->updateTrashHover(event->screenPos());
  }

  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
  {
    QGraphicsItemGroup::mouseReleaseEvent(event);
    const bool droppedOnTrash = m_view->isOverTrash(event->screenPos());
    m_view->clearTrashHover();
    if (droppedOnTrash) {
      m_view->removeScreen(m_screenName);
    }
  }

private:
  MonitorLayoutView *m_view;
  QString m_screenName;
};

QColor machineColor(const QString &name)
{
  static const QList<QColor> palette = {
      QColor(84, 138, 201, 220), QColor(90, 176, 137, 220), QColor(214, 154, 68, 220),
      QColor(158, 116, 200, 220), QColor(206, 105, 120, 220), QColor(94, 168, 186, 220)
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

void setTextFont(QGraphicsSimpleTextItem *item, int pixelSize, bool bold)
{
  QFont font = item->font();
  font.setPixelSize(qMax(1, pixelSize));
  font.setBold(bold);
  item->setFont(font);
}

QRectF fitTextToBounds(QGraphicsSimpleTextItem *item, int desiredPixelSize, const QSizeF &maxSize, bool bold)
{
  const QString fullText = item->text();
  for (int pixelSize = qMax(1, desiredPixelSize); pixelSize > 1; --pixelSize) {
    item->setText(fullText);
    setTextFont(item, pixelSize, bold);
    const QRectF bounds = item->boundingRect();
    if (bounds.width() <= maxSize.width() && bounds.height() <= maxSize.height()) {
      return bounds;
    }
  }

  setTextFont(item, 1, bold);
  QFontMetricsF metrics(item->font());
  item->setText(metrics.elidedText(fullText, Qt::ElideRight, maxSize.width()));
  return item->boundingRect();
}

QRectF paddedRect(const QRect &rect)
{
  const double minSide = qMin(rect.width(), rect.height());
  const double padding = qMax(2.0, minSide * kTilePaddingRatio);
  QRectF content = QRectF(rect).adjusted(padding, padding, -padding, -padding);
  if (content.width() <= 1.0 || content.height() <= 1.0) {
    return QRectF(rect);
  }
  return content;
}

QRect topLeftTile(const QList<MonitorTile> &tiles)
{
  QRect topLeft;
  for (const auto &tile : tiles) {
    if (topLeft.isNull() || tile.rect.top() < topLeft.top() ||
        (tile.rect.top() == topLeft.top() && tile.rect.left() < topLeft.left())) {
      topLeft = tile.rect;
    }
  }
  return topLeft;
}

//! Adds a rounded-corner tag naming the machine inside the top-left monitor
//! tile so it remains attached to the visible screen rectangle at every zoom.
void addComputerTag(QGraphicsItemGroup *group, const QString &name, const QList<MonitorTile> &tiles, const QColor &color)
{
  const QRect anchorTile = topLeftTile(tiles);
  if (anchorTile.isNull()) {
    return;
  }

  const QRectF content = paddedRect(anchorTile);
  const double minSide = qMin(anchorTile.width(), anchorTile.height());

  auto *label = new QGraphicsSimpleTextItem(name);
  label->setBrush(Qt::white);

  const int desiredFontSize = qMin(120, qMax(1, static_cast<int>(minSide * 0.09)));
  const QRectF textBounds = fitTextToBounds(
      label, desiredFontSize, QSizeF(qMax(1.0, content.width() * 0.72), qMax(1.0, content.height() * 0.18)),
      true
  );
  const qreal pillPadX = textBounds.height() * 0.7;
  const qreal pillPadY = textBounds.height() * 0.35;
  const qreal pillW = textBounds.width() + pillPadX * 2;
  const qreal pillH = textBounds.height() + pillPadY * 2;

  auto *pill = new QGraphicsPathItem;
  QPainterPath path;
  path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(pillW, pillH)), pillH / 2.0, pillH / 2.0);
  pill->setPath(path);
  pill->setBrush(QBrush(color.darker(115)));
  pill->setPen(Qt::NoPen);
  pill->setPos(content.topLeft());
  pill->setZValue(1);

  label->setParentItem(pill);
  label->setPos(pillPadX - textBounds.left(), (pillH - textBounds.height()) / 2.0 - textBounds.top());

  group->addToGroup(pill);
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

void MonitorLayoutView::setTrashWidget(QLabel *trash)
{
  m_trashWidget = trash;
}

void MonitorLayoutView::addGroupFor(const Screen &screen)
{
  auto *group = new MonitorGroupItem(this, screen.name());
  m_scene->addItem(group);

  QList<MonitorTile> tiles = screen.monitors();
  if (tiles.isEmpty()) {
    tiles = {MonitorTile{QRect(0, 0, 1920, 1080), {}}};
  }

  const QColor color = machineColor(screen.name());

  for (int m = 0; m < tiles.size(); ++m) {
    const QRect &r = tiles[m].rect;
    const QRectF content = paddedRect(r);
    const double minSide = qMin(r.width(), r.height());

    auto *tileItem = new QGraphicsPathItem;
    QPainterPath path;
    path.addRoundedRect(QRectF(r), kTileCornerRadius, kTileCornerRadius);
    tileItem->setPath(path);
    tileItem->setBrush(QBrush(color));
    tileItem->setPen(QPen(color.darker(140), 2));

    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 90));
    tileItem->setGraphicsEffect(shadow);

    const QString monitorName = tiles[m].name.isEmpty() ? tr("Monitor %1").arg(m + 1) : tiles[m].name;
    auto *label = new QGraphicsSimpleTextItem(monitorName, tileItem);
    label->setBrush(Qt::white);
    const int desiredLabelSize = qMin(130, qMax(1, static_cast<int>(minSide * 0.11)));
    const QRectF labelBounds = fitTextToBounds(
        label, desiredLabelSize, QSizeF(qMax(1.0, content.width()), qMax(1.0, content.height() * 0.24)), true
    );

    // Tiles are in real monitor-pixel scene units, then the whole scene is
    // scaled by fitInView(). Size the icon/caption as a bounded fraction of
    // the tile, and compute the icon from the remaining vertical space so the
    // combined block never leaves the rectangle.
    const double gap = qMax(1.0, minSide * 0.025);
    const double maxIconHeight = qMax(1.0, content.height() - labelBounds.height() - gap);
    const int iconSize = qMax(
        1,
        static_cast<int>(qMin(qMin(content.width(), maxIconHeight), minSide * 0.38))
    );

    // QIcon::fromTheme() caps out at the largest raster asset the icon theme
    // actually ships (often 64px) and silently refuses to upscale beyond
    // that, so requesting a bigger QSize from pixmap() alone has no visible
    // effect. Force an explicit smooth upscale to the size we actually want.
    QPixmap iconPixmap = QIcon::fromTheme("video-display").pixmap(QSize(iconSize, iconSize));
    if (!iconPixmap.isNull() && (iconPixmap.width() < iconSize || iconPixmap.height() < iconSize)) {
      iconPixmap = iconPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    // QIcon::pixmap() on HiDPI screens tags the pixmap with a devicePixelRatio
    // > 1, which makes its *logical* (device-independent) size — the size
    // QGraphicsPixmapItem actually lays out with — smaller than its raw
    // width()/height(). Reset it to 1.0 so the item's on-scene size matches
    // iconSize exactly, or all the sizing above would render ~1/DPR too small.
    iconPixmap.setDevicePixelRatio(1.0);

    const double iconWidth = iconPixmap.width();
    const double iconHeight = iconPixmap.height();
    const double combinedHeight = iconHeight > 0 ? iconHeight + gap + labelBounds.height() : labelBounds.height();
    const double blockTop = content.top() + (content.height() - combinedHeight) / 2.0;

    if (!iconPixmap.isNull()) {
      auto *icon = new QGraphicsPixmapItem(iconPixmap, tileItem);
      icon->setPos(content.left() + (content.width() - iconWidth) / 2.0, blockTop);
    }
    label->setPos(
        content.left() + (content.width() - labelBounds.width()) / 2.0 - labelBounds.left(),
        blockTop + (iconHeight > 0 ? iconHeight + gap : 0) - labelBounds.top()
    );

    group->addToGroup(tileItem);
  }

  addComputerTag(group, screen.name(), tiles, color);

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

void MonitorLayoutView::refreshMonitors(const QString &name, const QList<MonitorTile> &tiles)
{
  if (Screen *s = findScreen(name)) {
    s->setMonitors(tiles);
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

bool MonitorLayoutView::isOverTrash(const QPoint &globalPos) const
{
  if (m_trashWidget == nullptr) {
    return false;
  }
  const QRect globalRect(m_trashWidget->mapToGlobal(QPoint(0, 0)), m_trashWidget->size());
  return globalRect.contains(globalPos);
}

void MonitorLayoutView::updateTrashHover(const QPoint &globalPos)
{
  if (m_trashWidget == nullptr) {
    return;
  }
  const bool over = isOverTrash(globalPos);
  if (over == m_trashArmed) {
    return;
  }
  m_trashArmed = over;
  m_trashWidget->setStyleSheet(
      over ? QStringLiteral(
                 "QLabel { border: 2px solid #e74c3c; border-radius: 8px; background-color: rgba(231,76,60,40); }"
             )
           : QString()
  );
}

void MonitorLayoutView::clearTrashHover()
{
  if (m_trashWidget != nullptr) {
    m_trashArmed = false;
    m_trashWidget->setStyleSheet(QString());
  }
}

void MonitorLayoutView::removeScreen(const QString &name)
{
  if (m_screens == nullptr) {
    return;
  }
  for (int i = 0; i < m_screens->size(); ++i) {
    if ((*m_screens)[i].name() == name) {
      if ((*m_screens)[i].isServer()) {
        // removing the server's own computer would leave the config
        // without a server at all
        QMessageBox::information(
            this, tr("Remove Computer"), tr("The server's own computer can't be removed.")
        );
        return;
      }
      m_screens->removeAt(i);
      break;
    }
  }
  rebuild();
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

  Screen *screen = findScreen(name);

  QMenu menu(this);
  QAction *openSettingsAction = nullptr;
  if (screen != nullptr && screen->isServer()) {
    openSettingsAction = menu.addAction(tr("Open Display Settings..."));
    menu.addSeparator();
  }
  QAction *removeAction = menu.addAction(tr("Remove Computer"));

  QAction *chosen = menu.exec(event->globalPos());
  if (chosen == removeAction) {
    removeScreen(name);
  } else if (chosen == openSettingsAction) {
    if (!deskflow::gui::openDisplaySettings()) {
      QMessageBox::information(
          this, tr("Open Display Settings"),
          tr("Couldn't find a way to open display settings on this system. Please open it manually.")
      );
    }
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

void MonitorLayoutView::drawBackground(QPainter *painter, const QRectF &rect)
{
  painter->fillRect(rect, QColor(248, 249, 251));

  painter->setPen(QPen(QColor(222, 224, 230), 1));
  const int left = static_cast<int>(std::floor(rect.left() / kGridStep)) * kGridStep;
  const int top = static_cast<int>(std::floor(rect.top() / kGridStep)) * kGridStep;
  for (int x = left; x < rect.right(); x += kGridStep) {
    for (int y = top; y < rect.bottom(); y += kGridStep) {
      painter->drawPoint(x, y);
    }
  }
}

} // namespace deskflow::gui
