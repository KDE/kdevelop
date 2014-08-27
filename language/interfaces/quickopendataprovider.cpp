/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "quickopendataprovider.h"
#include <QVariant>

#include <QIcon>

namespace KDevelop {
  QuickOpenFileSetInterface::~QuickOpenFileSetInterface() {
  }

  QuickOpenEmbeddedWidgetInterface::~QuickOpenEmbeddedWidgetInterface() {
  }

  QuickOpenDataBase::~QuickOpenDataBase() {
  }
    QIcon QuickOpenDataBase::icon() const {
      return QIcon();
    }
  
  bool QuickOpenDataBase::isExpandable() const {
    return false;
  }

  QWidget* QuickOpenDataBase::expandingWidget() const {
    return 0;
  }
  
  QList<QVariant> QuickOpenDataBase::highlighting() const {
    return QList<QVariant>();
  }

  QuickOpenDataProviderBase::~QuickOpenDataProviderBase() {
  }

  void QuickOpenDataProviderBase::enableData( const QStringList& , const QStringList& ) {
  }

  bool extractLineNumber(const QString& from, QString& path, uint& lineNumber) {
    int colonIndex = from.indexOf(':');
    if (colonIndex != -1) {
      if (colonIndex == from.count() - 1) {
        path = from.mid(0, colonIndex);
        lineNumber = 0;
      } else {
        bool ok;
        uint number = from.mid(colonIndex + 1).toUInt(&ok);
        if (ok) {
          path = from.mid(0, colonIndex);
          lineNumber = number;
        } else {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  }
}

