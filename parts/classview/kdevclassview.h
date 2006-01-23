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

#ifndef KDEVCLASSVIEW_H
#define KDEVCLASSVIEW_H

#include "kdevtreeview.h"

class KDevClassViewPart;
class KDevCodeModel;
class KDevCodeItem;
class KDevCodeNamespaceItem;
class KDevCodeClassItem;
class KDevCodeFunctionItem;
class KDevCodeVariableItem;
class KURL;

class KDevClassView: public KDevTreeView
{
  Q_OBJECT
public:
  KDevClassView(KDevClassViewPart *part, QWidget *parent);
  virtual ~KDevClassView();

  KDevClassViewPart *part() const;
  KDevCodeModel *codeModel() const;

  KDevCodeItem *currentItem() const;
  KDevCodeNamespaceItem *currentNamespaceItem() const;
  KDevCodeClassItem *currentClassItem() const;
  KDevCodeFunctionItem *currentFunctionItem() const;
  KDevCodeVariableItem *currentVariableItem() const;

signals:
  void activateURL(const KURL &url);

protected slots:
  void slotActivated(const QModelIndex &index);
  void popupContextMenu(const QPoint &pos);

private:
  KDevClassViewPart *m_part;
};

#endif // KDEVCLASSVIEW_H
