/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <string>
#include <vector>

namespace deskflow {

//! ASCII Unit Separator: joins/splits the per-monitor name list carried by
//! kMsgDInfoMonitorNames.  Display names never contain this control
//! character, so no escaping is needed.
constexpr char kMonitorNameSeparator = '\x1f';

//! Joins \p names with kMonitorNameSeparator, in order, for transmission as
//! the single string payload of kMsgDInfoMonitorNames.
inline std::string joinMonitorNames(const std::vector<std::string> &names)
{
  std::string joined;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0) {
      joined += kMonitorNameSeparator;
    }
    joined += names[i];
  }
  return joined;
}

//! Splits a kMonitorNameSeparator-joined string back into up to \p count
//! names, in order.  If \p joined has fewer segments than \p count, the
//! missing trailing names are left empty; if it has more, the extras are
//! discarded (both are defensive handling of a count mismatch against the
//! preceding kMsgDInfoMonitors message, which should not normally happen).
inline std::vector<std::string> splitMonitorNames(const std::string &joined, size_t count)
{
  std::vector<std::string> names;
  names.reserve(count);

  size_t start = 0;
  while (names.size() < count) {
    const size_t sep = joined.find(kMonitorNameSeparator, start);
    const size_t end = (sep == std::string::npos) ? joined.size() : sep;
    names.push_back(joined.substr(start, end - start));
    if (sep == std::string::npos) {
      break;
    }
    start = sep + 1;
  }
  names.resize(count);
  return names;
}

} // namespace deskflow
