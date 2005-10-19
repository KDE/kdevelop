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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kdevclassviewdelegate.h"
#include "kdevcodemodel.h"

#include <QtGui/QPainter>
#include <QtCore/qdebug.h>

KDevClassViewDelegate::KDevClassViewDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

KDevClassViewDelegate::~KDevClassViewDelegate()
{
}

void KDevClassViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItem opt = option;

  if (const KDevCodeModel *codeModel = qobject_cast<const KDevCodeModel*>(index.model()))
    {
      KDevCodeItem *item = codeModel->item(index);

      if (item && (item->namespaceItem() || item->classItem()))
        {
          opt.font.setBold(true);
          opt.palette.setColor(QPalette::Text, Qt::blue);
        }
    }

  QItemDelegate::paint(painter, opt, index);
}

#include "kdevclassviewdelegate.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
