/*
 * KDevelop DUChain viewer
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

#include "duchaintree.h"

#include <QHeaderView>

#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "duchainmodel.h"
#include "duchainviewplugin.h"
#include "duchainbase.h"

using namespace KDevelop;

DUChainTree::DUChainTree(QWidget* parent, DUChainViewPlugin* plugin)
  : QTreeView(parent)
  , mplugin(plugin)
{
  setObjectName("DUChain Viewer Tree");
  setWindowTitle(i18n("Definition-Use Chain"));
  setModel(mplugin->model());
  header()->hide();

  connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(contextActivated(const QModelIndex&)));
}

DUChainTree::~DUChainTree()
{
}

DUChainModel* DUChainTree::model()
{
  return mplugin->model();
}

void DUChainTree::contextActivated(const QModelIndex& index)
{
  DUChainBasePointer* base = model()->objectForIndex(index);
  if (base && base->data()) {
    IDocument* doc = mplugin->core()->documentController()->activeDocument();
    doc->textDocument()->activeView()->setSelection((*base)->textRange());
  }
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "duchaintree.moc"
