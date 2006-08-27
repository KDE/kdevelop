/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef CPPNAMESPACE_H
#define CPPNAMESPACE_H

#include <QFlags>

namespace Cpp {
  enum AccessPolicy
  {
    Public,
    Protected,
    Private
  };

  enum CVSpec {
    CVNone = 0,
    Const = 0x1,
    Volatile = 0x2
  };
  Q_DECLARE_FLAGS(CVSpecs, CVSpec);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cpp::CVSpecs);

#endif // CPPNAMESPACE_H

// kate: space-indent on; indent-width 2; replace-tabs on;
