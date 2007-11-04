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
#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>

#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "classmodel.h"
#include "classbrowserpart.h"
#include "duchainbase.h"
#include "duchain.h"
#include "duchainlock.h"
#include "declaration.h"
#include "definition.h"

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

void ClassTree::contextMenuEvent(QContextMenuEvent* e)
{
  QMenu *menu = new QMenu(this);
  QModelIndex index = indexAt(e->pos());
  if (index.isValid()) {
    DUChainReadLocker readLock(DUChain::lock());

    DUChainBasePointer* base = model()->objectForIndex(index);

    if (base && base->data()) {
      QAction* openDec = menu->addAction(i18n("Open &Declaration"), this, SLOT(openDeclaration()));
      openDec->setData(QVariant::fromValue(*base));
      QAction* openDef = menu->addAction(i18n("Open De&finition"), this, SLOT(openDefinition()));
      openDef->setData(QVariant::fromValue(*base));

      if (DUContext* d = dynamic_cast<DUContext*>(base->data())) {
        if (d->owner())
          if (d->owner()->asDeclaration()) {
            openDef->setEnabled(false);
          }

      } else if (dynamic_cast<Declaration*>(base->data())) {
        openDef->setEnabled(false);

      }
    }
  }
  menu->exec(QCursor::pos());
}

void ClassTree::itemActivated(const QModelIndex& index)
{
  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer* base = model()->objectForIndex(index);
  if (base && base->data()) {
    KUrl url = (*base)->url();
    KTextEditor::Range range = (*base)->textRange();

    readLock.unlock();

    IDocument* doc = m_part->core()->documentController()->documentForUrl(url);

    if (!doc)
      doc = m_part->core()->documentController()->openDocument(url, range.start());

    doc->textDocument()->activeView()->setSelection(range);
  }
}

void ClassTree::openDeclaration()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  QAction* a = static_cast<QAction*>(sender());

  Q_ASSERT(a->data().canConvert<DUChainBasePointer>());
  
  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(a->data());
  if (base) {
    Declaration* dec = model()->declarationForObject(base);

    if (dec) {
      KUrl url = dec->url();
      KTextEditor::Range range = dec->textRange();

      readLock.unlock();

      m_part->core()->documentController()->openDocument(url, range.start());

    } else {
      kDebug() << "No declaration for base object" << base;
    }

  } else {
    kDebug() << "Base object has disappeared from the duchain";
  }
}

void ClassTree::openDefinition()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(static_cast<QAction*>(sender())->data());
  if (base) {
    Definition* def = model()->definitionForObject(base);

    if (def) {
      KUrl url = def->url();
      KTextEditor::Range range = def->textRange();

      readLock.unlock();

      m_part->core()->documentController()->openDocument(url, range.start());
    }
  }
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "classtree.moc"
