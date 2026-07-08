/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2003 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/ClipboardTypes.h"

#include <cstdint>
#include <string>
#include <vector>

class IClipboard;

//! Individual monitor geometry
/*!
Position of the upper-left corner and size of a single physical monitor
in the virtual desktop coordinate system.  A screen (one machine) may be
made up of several of these.
*/
struct MonitorInfo
{
  int32_t x = 0;
  int32_t y = 0;
  int32_t w = 0;
  int32_t h = 0;

  //! The monitor's human-readable name, as shown in the OS's own display
  //! settings (e.g. "Built-in Retina Display", "DELL U2720Q", "HDMI-1").
  //! Empty when the platform couldn't determine one, or when reported by a
  //! peer that predates this field.
  std::string name;
};

//! Screen interface
/*!
This interface defines the methods common to all screens.
*/
class IScreen
{
public:
  virtual ~IScreen() = default;
  struct ClipboardInfo
  {
  public:
    ClipboardID m_id;
    uint32_t m_sequenceNumber;
  };

  //! @name accessors
  //@{

  //! Get event target
  /*!
  Returns the target used for events created by this object.
  */
  virtual void *getEventTarget() const = 0;

  //! Get clipboard
  /*!
  Save the contents of the clipboard indicated by \c id and return
  true iff successful.
  */
  virtual bool getClipboard(ClipboardID id, IClipboard *) const = 0;

  //! Get screen shape
  /*!
  Return the position of the upper-left corner of the screen in \c x and
  \c y and the size of the screen in \c width and \c height.
  */
  virtual void getShape(int32_t &x, int32_t &y, int32_t &width, int32_t &height) const = 0;

  //! Get individual monitor geometry
  /*!
  Return the list of individual physical monitors that make up this screen,
  each in virtual desktop coordinates.  The default implementation returns a
  single entry equal to getShape() (the union bounding box), which preserves
  the legacy single-rectangle behavior for callers and peers that don't
  provide per-monitor information.
  */
  virtual void getMonitors(std::vector<MonitorInfo> &monitors) const
  {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    getShape(x, y, w, h);
    monitors.clear();
    monitors.push_back(MonitorInfo{x, y, w, h});
  }

  //! Get cursor position
  /*!
  Return the current position of the cursor in \c x and \c y.
  */
  virtual void getCursorPos(int32_t &x, int32_t &y) const = 0;

  //@}
};
