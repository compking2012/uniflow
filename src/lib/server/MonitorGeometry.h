/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "base/DirectionTypes.h"
#include "deskflow/IScreen.h"

#include <cstdint>
#include <vector>

namespace deskflow::server {

//! @name Monitor geometry helpers
//! Pure geometric helpers for reasoning about a machine's individual monitors.
//! These let the server route the cursor between physically-adjacent monitors
//! of different machines, including across "internal" edges that border a
//! multi-monitor dead zone (e.g. an L-shaped desktop).
//@{

//! true if monitor rect \p m contains the point (x,y); half-open on the far edge
inline bool monitorContains(const MonitorInfo &m, int32_t x, int32_t y)
{
  return x >= m.x && x < m.x + m.w && y >= m.y && y < m.y + m.h;
}

//! index of the monitor containing (x,y), or -1 if none does
inline int monitorContaining(const std::vector<MonitorInfo> &monitors, int32_t x, int32_t y)
{
  for (size_t i = 0; i < monitors.size(); ++i) {
    if (monitorContains(monitors[i], x, y)) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

//! index of the monitor containing (x,y); if none does, the geometrically
//! nearest one.  \p monitors must be non-empty.
inline int findMonitor(const std::vector<MonitorInfo> &monitors, int32_t x, int32_t y)
{
  if (int i = monitorContaining(monitors, x, y); i >= 0) {
    return i;
  }
  int best = 0;
  long long bestDist = -1;
  for (size_t i = 0; i < monitors.size(); ++i) {
    const MonitorInfo &m = monitors[i];
    const int32_t cx = (x < m.x) ? m.x : (x >= m.x + m.w ? m.x + m.w - 1 : x);
    const int32_t cy = (y < m.y) ? m.y : (y >= m.y + m.h ? m.y + m.h - 1 : y);
    const long long ddx = x - cx;
    const long long ddy = y - cy;
    if (const long long d = ddx * ddx + ddy * ddy; bestDist < 0 || d < bestDist) {
      bestDist = d;
      best = static_cast<int>(i);
    }
  }
  return best;
}

//! Is there another monitor of the same machine immediately adjacent to
//! monitor \p mi on side \p dir, overlapping the perpendicular coordinate
//! \p cross?  If so, that edge is internal and the OS carries the cursor onto
//! the adjacent monitor, so the server must not switch screens there.
inline bool hasSameMachineNeighbor(const std::vector<MonitorInfo> &monitors, int mi, Direction dir, int32_t cross)
{
  const MonitorInfo &m = monitors[static_cast<size_t>(mi)];
  for (size_t i = 0; i < monitors.size(); ++i) {
    if (static_cast<int>(i) == mi) {
      continue;
    }
    const MonitorInfo &n = monitors[i];
    using enum Direction;
    switch (dir) {
    case Left:
      if (n.x + n.w == m.x && cross >= n.y && cross < n.y + n.h) {
        return true;
      }
      break;
    case Right:
      if (n.x == m.x + m.w && cross >= n.y && cross < n.y + n.h) {
        return true;
      }
      break;
    case Top:
      if (n.y + n.h == m.y && cross >= n.x && cross < n.x + n.w) {
        return true;
      }
      break;
    case Bottom:
      if (n.y == m.y + m.h && cross >= n.x && cross < n.x + n.w) {
        return true;
      }
      break;
    case NoDirection:
      break;
    }
  }
  return false;
}

//! Choose the destination monitor to land on when entering a machine while
//! travelling in direction \p dir with perpendicular entry coordinate \p perp
//! (y for Left/Right, x for Top/Bottom).  Returns the index of the monitor
//! whose entering edge is most exposed toward \p dir among those spanning
//! \p perp, or -1 if no monitor spans \p perp (entered opposite a dead zone).
inline int chooseEntryMonitor(const std::vector<MonitorInfo> &monitors, Direction dir, int32_t perp)
{
  using enum Direction;
  const bool horizontal = (dir == Left || dir == Right);
  int best = -1;
  for (size_t i = 0; i < monitors.size(); ++i) {
    const MonitorInfo &m = monitors[i];
    const bool spans = horizontal ? (perp >= m.y && perp < m.y + m.h) : (perp >= m.x && perp < m.x + m.w);
    if (!spans) {
      continue;
    }
    if (best < 0) {
      best = static_cast<int>(i);
      continue;
    }
    const MonitorInfo &b = monitors[static_cast<size_t>(best)];
    bool better = false;
    switch (dir) {
    case Left:
      better = (m.x + m.w) > (b.x + b.w); // enter right edges: rightmost wins
      break;
    case Right:
      better = m.x < b.x; // enter left edges: leftmost wins
      break;
    case Top:
      better = (m.y + m.h) > (b.y + b.h); // enter bottom edges: bottommost wins
      break;
    case Bottom:
      better = m.y < b.y; // enter top edges: topmost wins
      break;
    case NoDirection:
      break;
    }
    if (better) {
      best = static_cast<int>(i);
    }
  }
  return best;
}

//@}

} // namespace deskflow::server
