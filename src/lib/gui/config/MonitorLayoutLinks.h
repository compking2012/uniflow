/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QList>
#include <QMap>
#include <QRect>
#include <QString>

namespace deskflow::gui {

//! A generated cross-machine link between portions of two machines' edges.
//! Intervals are percentages [0,100] along the source/destination machine's
//! bounding box (height for left/right, width for up/down) -- matching the
//! deskflow config "links" section grammar.
struct MonitorLink
{
  QString srcMachine;
  QString side; ///< "left", "right", "up" or "down"
  double srcStart = 0.0;
  double srcEnd = 100.0;
  QString dstMachine;
  double dstStart = 0.0;
  double dstEnd = 100.0;

  bool operator==(const MonitorLink &o) const
  {
    auto eq = [](double a, double b) { return qFuzzyCompare(a + 1.0, b + 1.0); };
    return srcMachine == o.srcMachine && side == o.side && dstMachine == o.dstMachine && eq(srcStart, o.srcStart) &&
           eq(srcEnd, o.srcEnd) && eq(dstStart, o.dstStart) && eq(dstEnd, o.dstEnd);
  }
};

namespace detail {

//! bounding box (union) of a machine's monitors; null rect if empty
inline QRect monitorsBoundingBox(const QList<QRect> &monitors)
{
  QRect box;
  for (const auto &m : monitors) {
    box = box.isNull() ? m : box.united(m);
  }
  return box;
}

//! length of the overlap of [a0,a1) and [b0,b1); writes the overlap span to
//! (lo,hi).  <=0 means the ranges don't overlap.
inline int overlap1D(int a0, int a1, int b0, int b1, int &lo, int &hi)
{
  lo = (a0 > b0) ? a0 : b0;
  hi = (a1 < b1) ? a1 : b1;
  return hi - lo;
}

//! percentage interval [0,100] of [lo,hi) within [base, base+extent)
inline void toPercent(int lo, int hi, int base, int extent, double &start, double &end)
{
  start = 100.0 * static_cast<double>(lo - base) / static_cast<double>(extent);
  end = 100.0 * static_cast<double>(hi - base) / static_cast<double>(extent);
}

} // namespace detail

//! Given each machine's monitors positioned in a shared canvas coordinate
//! space, compute the cross-machine adjacency links as machine-edge
//! sub-interval links.  Two monitors of different machines are adjacent when
//! one's edge touches (within \p tolerance px) the opposite edge of the other
//! and their perpendicular ranges overlap.  The connecting segment is the
//! overlap, expressed as a fraction of each machine's bounding box (height for
//! left/right, width for up/down).
inline QList<MonitorLink> generateMonitorLinks(const QMap<QString, QList<QRect>> &layout, int tolerance = 2)
{
  using detail::monitorsBoundingBox;
  using detail::overlap1D;
  using detail::toPercent;

  QList<MonitorLink> links;
  const QList<QString> names = layout.keys();

  // precompute bounding boxes
  QMap<QString, QRect> boxes;
  for (const QString &name : names) {
    boxes.insert(name, monitorsBoundingBox(layout.value(name)));
  }

  for (const QString &srcName : names) {
    const QRect &srcBox = boxes.value(srcName);
    if (srcBox.width() <= 0 || srcBox.height() <= 0) {
      continue;
    }
    for (const QString &dstName : names) {
      if (dstName == srcName) {
        continue;
      }
      const QRect &dstBox = boxes.value(dstName);
      if (dstBox.width() <= 0 || dstBox.height() <= 0) {
        continue;
      }

      for (const QRect &ms : layout.value(srcName)) {
        const int msL = ms.x();
        const int msR = ms.x() + ms.width();
        const int msT = ms.y();
        const int msB = ms.y() + ms.height();
        for (const QRect &md : layout.value(dstName)) {
          const int mdL = md.x();
          const int mdR = md.x() + md.width();
          const int mdT = md.y();
          const int mdB = md.y() + md.height();

          int lo;
          int hi;
          MonitorLink link;
          link.srcMachine = srcName;
          link.dstMachine = dstName;

          // src RIGHT edge <-> dst LEFT edge (vertical overlap)
          if (qAbs(msR - mdL) <= tolerance && overlap1D(msT, msB, mdT, mdB, lo, hi) > 0) {
            link.side = QStringLiteral("right");
            toPercent(lo, hi, srcBox.y(), srcBox.height(), link.srcStart, link.srcEnd);
            toPercent(lo, hi, dstBox.y(), dstBox.height(), link.dstStart, link.dstEnd);
            links.append(link);
          }
          // src LEFT edge <-> dst RIGHT edge
          else if (qAbs(msL - mdR) <= tolerance && overlap1D(msT, msB, mdT, mdB, lo, hi) > 0) {
            link.side = QStringLiteral("left");
            toPercent(lo, hi, srcBox.y(), srcBox.height(), link.srcStart, link.srcEnd);
            toPercent(lo, hi, dstBox.y(), dstBox.height(), link.dstStart, link.dstEnd);
            links.append(link);
          }
          // src BOTTOM edge <-> dst TOP edge (horizontal overlap)
          else if (qAbs(msB - mdT) <= tolerance && overlap1D(msL, msR, mdL, mdR, lo, hi) > 0) {
            link.side = QStringLiteral("down");
            toPercent(lo, hi, srcBox.x(), srcBox.width(), link.srcStart, link.srcEnd);
            toPercent(lo, hi, dstBox.x(), dstBox.width(), link.dstStart, link.dstEnd);
            links.append(link);
          }
          // src TOP edge <-> dst BOTTOM edge
          else if (qAbs(msT - mdB) <= tolerance && overlap1D(msL, msR, mdL, mdR, lo, hi) > 0) {
            link.side = QStringLiteral("up");
            toPercent(lo, hi, srcBox.x(), srcBox.width(), link.srcStart, link.srcEnd);
            toPercent(lo, hi, dstBox.x(), dstBox.width(), link.dstStart, link.dstEnd);
            links.append(link);
          }
        }
      }
    }
  }
  return links;
}

} // namespace deskflow::gui
