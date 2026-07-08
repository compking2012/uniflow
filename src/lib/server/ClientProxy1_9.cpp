/*
 * Uniflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "server/ClientProxy1_9.h"

#include "base/IEventQueue.h"
#include "base/Log.h"
#include "uniflow/MonitorNames.h"
#include "uniflow/ProtocolTypes.h"
#include "uniflow/ProtocolUtil.h"

#include <cstring>

ClientProxy1_9::ClientProxy1_9(
    const std::string &name, deskflow::IStream *adoptedStream, Server *server, IEventQueue *events
)
    : ClientProxy1_8(name, adoptedStream, server, events),
      m_events(events)
{
  // do nothing
}

void ClientProxy1_9::getMonitors(std::vector<MonitorInfo> &monitors) const
{
  if (m_monitors.empty()) {
    // no per-monitor info received (yet); fall back to the single
    // union bounding box reported via kMsgDInfo
    IScreen::getMonitors(monitors);
    return;
  }
  monitors = m_monitors;
}

bool ClientProxy1_9::parseMessage(const uint8_t *code)
{
  if (memcmp(code, kMsgDInfoMonitors, 4) == 0) {
    if (recvInfoMonitors()) {
      // the initial "clientMonitors" IPC push (sent right after connect, see
      // Server::adoptClient) races this message and typically goes out
      // before it arrives, carrying only the single-rect fallback.  Raise a
      // shape-changed event so Server::handleShapeChanged() re-sends it now
      // that the real per-monitor layout is known.
      m_events->addEvent(Event(EventTypes::ScreenShapeChanged, getEventTarget()));
      return true;
    }
    return false;
  }
  if (memcmp(code, kMsgDInfoMonitorNames, 4) == 0) {
    if (recvInfoMonitorNames()) {
      m_events->addEvent(Event(EventTypes::ScreenShapeChanged, getEventTarget()));
      return true;
    }
    return false;
  }
  return ClientProxy1_8::parseMessage(code);
}

bool ClientProxy1_9::recvInfoMonitors()
{
  // the payload is a flat list of 32-bit ints, four per monitor:
  // [x0, y0, w0, h0, x1, y1, w1, h1, ...] (signed values as two's complement)
  std::vector<uint32_t> data;
  if (!ProtocolUtil::readf(getStream(), kMsgDInfoMonitors + 4, &data)) {
    return false;
  }

  std::vector<MonitorInfo> monitors;
  monitors.reserve(data.size() / 4);
  for (size_t i = 0; i + 3 < data.size(); i += 4) {
    monitors.push_back(MonitorInfo{
        static_cast<int32_t>(data[i]), static_cast<int32_t>(data[i + 1]), static_cast<int32_t>(data[i + 2]),
        static_cast<int32_t>(data[i + 3])
    });
  }
  m_monitors = std::move(monitors);

  LOG_DEBUG("received client \"%s\" monitor layout: %d monitor(s)", getName().c_str(), (int)m_monitors.size());
  return true;
}

bool ClientProxy1_9::recvInfoMonitorNames()
{
  std::string joined;
  if (!ProtocolUtil::readf(getStream(), kMsgDInfoMonitorNames + 4, &joined)) {
    return false;
  }

  const std::vector<std::string> names = deskflow::splitMonitorNames(joined, m_monitors.size());
  for (size_t i = 0; i < names.size(); ++i) {
    m_monitors[i].name = names[i];
  }

  LOG_DEBUG("received client \"%s\" monitor names", getName().c_str());
  return true;
}
