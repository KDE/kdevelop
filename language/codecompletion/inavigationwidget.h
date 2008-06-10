/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef INAVIGATIONWIDGET_H
#define INAVIGATIONWIDGET_H

#include "../languageexport.h"

namespace KDevelop {

  /**
   * This class deleted itself when its part is deleted, so always use a QPointer when referencing it.
   * The duchain must be read-locked for most operations
   * */
  class KDEVPLATFORMLANGUAGE_EXPORT INavigationWidget {
    public:
      virtual ~INavigationWidget();

      ///Keyboard-action "next"
      virtual void next() = 0;
      ///Keyboard-action "previous"
      virtual void previous() = 0;
      ///Keyboard-action "accept"
      virtual void accept() = 0;
      virtual void up() = 0;
      virtual void down() = 0;
  };
}

#endif
