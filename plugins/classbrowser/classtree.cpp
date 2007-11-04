/*
 * KDevelop Class viewer
 *
 * Copyright (c) 2007 Hamish Rodda <rodda@kde.org>
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

#include "classtree.h"

#include <QHeaderView>

#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "classmodel.h"
#include "classbrowserpart.h"
#include "duchainbase.h"

using namespace KDevelop;

ClassTree::ClassTree(QWidget* parent, ClassBrowserPart* part)
  : QTreeView(parent)
  , m_part(part)
{
  setObjectName("Class Browser Tree");
  setWindowTitle(i18n("Class Browser"));
  setModel(m_part->model());
  header()->hide();

  connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
}

ClassTree::~ClassTree()
{
}

ClassModel* ClassTree::model()
{
  return m_part->model();
}

void ClassTree::itemActivated(const QModelIndex& index)
{
  DUChainBasePointer* base = model()->objectForIndex(index);
  if (base && base->data()) {
    IDocument* doc = m_part->core()->documentController()->documentForUrl((*base)->url());

    if (!doc)
      doc = m_part->core()->documentController()->openDocument((*base)->url(), (*base)->textRange().start());

    doc->textDocument()->activeView()->setSelection((*base)->textRange());
  }
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "classtree.moc"
