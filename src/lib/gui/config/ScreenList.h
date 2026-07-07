/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2021 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "Screen.h"

class ScreenList : public QList<Screen>
{
public:
  ScreenList() = default;

  /**
   * @brief addScreen appends a new screen to the list, choosing an initial
   * canvas position that places it just to the right of the server's
   * screen (or the rightmost known screen if there is no server yet),
   * offset far enough to avoid overlapping any existing screen's monitor
   * bounding box.  Falls back to the origin if this is the first screen.
   * @param newScreen
   */
  void addScreen(Screen newScreen);

  /**
   * @brief Returns true if screens are equal
   * @param sc
   */
  bool operator==(const ScreenList &sc) const;
};
