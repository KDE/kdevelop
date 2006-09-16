/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdevprojectmanagerdelegate.h"
#include "kdevprojectmodel.h"

#include <QtGui/QPainter>

#include <kdebug.h>

KDevProjectManagerDelegate::KDevProjectManagerDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

KDevProjectManagerDelegate::~KDevProjectManagerDelegate()
{
}

void KDevProjectManagerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItem opt = option;

  const KDevProjectModel *projectModel = qobject_cast<const KDevProjectModel*>(index.model());
  if ( projectModel )
  {
    KDevProjectItem *item = projectModel->item(index);
    if ( item )
    {
      if ( item->folder() )
      {
        opt.font.setBold(true);
        opt.palette.setColor(QPalette::Text, Qt::blue);
      }
      else if ( item->target() )
      {
        opt.font.setBold(true);
        opt.palette.setColor(QPalette::Text, Qt::red);
      }
    }
  }

  QItemDelegate::paint(painter, opt, index);
}

#include "kdevprojectmanagerdelegate.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
