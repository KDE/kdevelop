/*
 * KDevelop Class Browser
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "classmodel.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kprocess.h>

#include "idocument.h"
#include "icore.h"
#include "ilanguagecontroller.h"
#include "backgroundparser/backgroundparser.h"
#include "backgroundparser/parsejob.h"

#include "classbrowserpart.h"
#include "topducontext.h"
#include "declaration.h"
#include "definition.h"
#include "parsingenvironment.h"
#include "use.h"
#include "duchain.h"
#include "duchainlock.h"
#include "duchainpointer.h"
#include "duchainutils.h"

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

ClassModel::ClassModel(ClassBrowserPart* parent)
  : QAbstractItemModel(parent)
{
  //new ModelTest(this);

  bool success = connect(DUChain::self()->notifier(), SIGNAL(contextChanged(KDevelop::DUContextPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), SLOT(contextChanged(KDevelop::DUContextPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), Qt::QueuedConnection);
  success &= connect(DUChain::self()->notifier(), SIGNAL(declarationChanged(KDevelop::DeclarationPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), SLOT(declarationChanged(KDevelop::DeclarationPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), Qt::QueuedConnection);
  success &= connect(DUChain::self()->notifier(), SIGNAL(definitionChanged(KDevelop::DefinitionPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), SLOT(definitionChanged(KDevelop::DefinitionPointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), Qt::QueuedConnection);
  success &= connect(DUChain::self()->notifier(), SIGNAL(useChanged(KDevelop::UsePointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), SLOT(useChanged(KDevelop::UsePointer, KDevelop::DUChainObserver::Modification, KDevelop::DUChainObserver::Relationship, KDevelop::DUChainBasePointer)), Qt::QueuedConnection);
  Q_ASSERT(success);
}

ClassBrowserPart* ClassModel::part() const {
  return qobject_cast<ClassBrowserPart*>(QObject::parent());
}

ClassModel::~ClassModel()
{
  qDeleteAll(m_topObjects);
}

int ClassModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);

  return 1;
}

DUChainBasePointer* ClassModel::objectForIndex(const QModelIndex& index) const
{
  return static_cast<DUChainBasePointer*>(index.internalPointer());
}

QModelIndex ClassModel::createIndex(int row, int column, KDevelop::DUChainBase* object) const
{
  if (!m_knownObjects.contains(object))
    m_knownObjects.insert(object, new DUChainBasePointer(object->weakPointer()));

  return QAbstractItemModel::createIndex(row, column, m_knownObjects[object]);
}

QModelIndex ClassModel::createIndex(int row, int column, KDevelop::DUChainBasePointer* pointer) const
{
  return QAbstractItemModel::createIndex(row, column, pointer);
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex & parent) const
{
  if (row < 0 || column < 0 || column > 0)
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  if (!parent.isValid()) {
    if (row < m_topObjects.count())
      return createIndex(row, column, m_topObjects.at(row));

    return QModelIndex();
  }

  DUChainBasePointer* base = objectForIndex(parent);
  if (!base)
    return QModelIndex();

  if (DUContext* context = dynamic_cast<DUContext*>(base->data())) {
    switch (context->type()) {
      case DUContext::Class: {
        if (row < context->localDeclarations().count())
          return createIndex(row, column, context->localDeclarations().at(row));

        int rowOffset = context->localDeclarations().count();

        if (row - rowOffset < context->localDefinitions().count())
          return createIndex(row, column, context->localDefinitions().at(row - rowOffset));

        return QModelIndex();
      }
    }
  }

  return QModelIndex();
}

int ClassModel::rowCount(const QModelIndex & parent) const
{
  DUChainReadLocker readLock(DUChain::lock());

  if (!parent.isValid())
    return m_topObjects.count();

  DUChainBasePointer* base = objectForIndex(parent);
  if (!base)
    return 0;

  if (DUContext* context = dynamic_cast<DUContext*>(base->data())) {
    switch (context->type()) {
      case DUContext::Class: {
        return context->localDeclarations().count() + context->localDefinitions().count();
      }
    }
  }

  return 0;
}

QModelIndex ClassModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer* base = objectForIndex(index);
  if (!base || !base->data())
    return QModelIndex();

  if (DUContext* context = dynamic_cast<DUContext*>(base->data()))
    return QModelIndex();

  else if (Declaration* dec = dynamic_cast<Declaration*>(base->data()))
    return contextIndex(dec->context());

  else if (Definition* def = dynamic_cast<Definition*>(base->data()))
    return contextIndex(def->context());

  // Shouldn't really hit this
  Q_ASSERT(false);
  return QModelIndex();
}

QModelIndex ClassModel::contextIndex(DUContext* context) const
{
  Q_ASSERT(context);
  if (DUChainBasePointer* pointer = m_knownObjects[context])
    return createIndex(m_topObjects.indexOf(pointer), 0, context);

  return QModelIndex();
}

QVariant ClassModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer* basep = objectForIndex(index);
  if (!basep)
    return QVariant();

  DUChainBase* base = basep->data();
  if (!base)
    return QVariant();

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    if (context->owner()) {
      if (Definition* definition = context->owner()->asDefinition()) {
        switch (role) {
          case Qt::DisplayRole:
            return definition->declaration()->identifier().toString();
          case Qt::DecorationRole:
            return DUChainUtils::iconForDeclaration(definition->declaration());
        }

      } else if (Declaration* declaration = context->owner()->asDeclaration()) {
        switch (role) {
          case Qt::DisplayRole:
            return declaration->identifier().toString();
          case Qt::DecorationRole:
            return DUChainUtils::iconForDeclaration(declaration);
        }
      }
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return dec->identifier().toString();
      case Qt::DecorationRole:
        return DUChainUtils::iconForDeclaration(dec);
    }

  } else if (Definition* def = dynamic_cast<Definition*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return def->declaration()->identifier().toString();
      case Qt::DecorationRole:
        return DUChainUtils::iconForDeclaration(def->declaration());
    }

  } else if (Use* use = dynamic_cast<Use*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Use: %1", use->declaration() ? use->declaration()->identifier().toString() : i18n("[No definition found]"));
    }

  } else {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Unknown object!");
    }
  }

  return QVariant();
}

void ClassModel::contextChanged(DUContextPointer context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBasePointer relatedObject)
{
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);

  DUChainReadLocker readLock(DUChain::lock());

  switch (change) {
    case DUChainObserver::Deletion:
    case DUChainObserver::Removal: {
      int i = 0;
      foreach (DUChainBasePointer* base, m_topObjects) {
        if (*base == relatedObject) {
          beginRemoveRows(QModelIndex(), i, i);
          delete m_topObjects.takeAt(i);
          endRemoveRows();
          break;
        }
        ++i;
      }
      break;
    }

    case DUChainObserver::Change:
      break;

    case DUChainObserver::Addition:
      if (DUContext* newContext = dynamic_cast<DUContext*>(relatedObject.data())) {
        if (newContext->type() == DUContext::Class && newContext->owner()) {
          beginInsertRows(QModelIndex(), m_topObjects.count(), m_topObjects.count());
          DUChainBasePointer* bp = new DUChainBasePointer(relatedObject);
          m_knownObjects.insert(relatedObject.data(), bp);
          m_topObjects.append(bp);
          endInsertRows();
        }
      }
      break;
  }
}

void ClassModel::declarationChanged(DeclarationPointer declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBasePointer relatedObject)
{
  Q_UNUSED(declaration);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void ClassModel::definitionChanged(DefinitionPointer definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBasePointer relatedObject)
{
  Q_UNUSED(definition);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void ClassModel::useChanged(UsePointer use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBasePointer relatedObject)
{
  Q_UNUSED(use);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
