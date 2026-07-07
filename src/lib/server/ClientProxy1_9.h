/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "server/ClientProxy1_8.h"

#include <vector>

class IEventQueue;

//! Proxy for client implementing protocol version 1.9
/*!
Adds the per-monitor screen layout (kMsgDInfoMonitors) on top of protocol
version 1.8.  This lets the server route the cursor between individual
physically-adjacent monitors of different machines rather than only between
whole-machine bounding boxes.
*/
class ClientProxy1_9 : public ClientProxy1_8
{
public:
  ClientProxy1_9(const std::string &name, deskflow::IStream *adoptedStream, Server *server, IEventQueue *events);
  ~ClientProxy1_9() override = default;

  // IScreen overrides
  void getMonitors(std::vector<MonitorInfo> &monitors) const override;

protected:
  bool parseMessage(const uint8_t *code) override;

private:
  bool recvInfoMonitors();

  // per-monitor geometry received from the client, in the client's virtual
  // desktop coordinate system.  empty until a kMsgDInfoMonitors is received.
  std::vector<MonitorInfo> m_monitors;
  IEventQueue *m_events;
};
