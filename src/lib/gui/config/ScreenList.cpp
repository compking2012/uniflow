/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2012 - 2021 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2008 Volker Lanz <vl@fidra.de>
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ScreenList.h"

#include "gui/config/MonitorLayoutLinks.h"

namespace {

constexpr int kCanvasGap = 200;

QRect canvasBoundingBox(const Screen &screen)
{
  QList<QRect> rects;
  for (const auto &tile : screen.monitors()) {
    rects.append(tile.rect);
  }
  if (rects.isEmpty()) {
    rects = {QRect(0, 0, 1920, 1080)};
  }
  return deskflow::gui::detail::monitorsBoundingBox(rects).translated(screen.canvasPos());
}

} // namespace

void ScreenList::addScreen(Screen newScreen)
{
  if (!newScreen.hasCanvasPos()) {
    if (isEmpty()) {
      // first screen in the layout: place it at the origin
      newScreen.setCanvasPos(QPoint(0, 0));
    } else {
      // place the new screen just to the right of the current rightmost
      // screen, which by construction can never overlap any existing one
      int rightEdge = 0;
      int topEdge = 0;
      for (const auto &screen : *this) {
        const QRect box = canvasBoundingBox(screen);
        if (box.right() > rightEdge) {
          rightEdge = box.right();
          topEdge = box.top();
        }
      }
      newScreen.setCanvasPos(QPoint(rightEdge + kCanvasGap, topEdge));
    }
  }

  append(newScreen);
}

bool ScreenList::operator==(const ScreenList &sc) const
{
  return QList::operator==(sc);
}
