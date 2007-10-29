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

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

ClassModel::ClassModel(ClassBrowserPart* parent)
  : QAbstractItemModel(parent)
{
  connect(this, SIGNAL(scheduleAddition(DUContext*)), this, SLOT(slotScheduleAddition(DUContext*)), Qt::QueuedConnection);
  connect(this, SIGNAL(scheduleRemoval(DUContext*)), this, SLOT(slotScheduleRemoval(DUContext*)), Qt::QueuedConnection);
  
  DUChainWriteLocker writeLock(DUChain::lock());
  DUChain::self()->addObserver(this);
  //new ModelTest(this);
  connect( part()->core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

ClassBrowserPart* ClassModel::part() const {
  return qobject_cast<ClassBrowserPart*>(QObject::parent());
}

ClassModel::~ClassModel()
{
}

int ClassModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);

  return 1;
}

DUChainBase* ClassModel::objectForIndex(const QModelIndex& index) const
{
  return static_cast<DUChainBase*>(index.internalPointer());
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex & parent) const
{
  if (row < 0 || column < 0 || column > 0)
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  if (!parent.isValid()) {
    if (row < m_topObjects.count())
      return createIndex(row, column, m_topObjects.at(row));

    return QModelIndex();
  }

  DUChainBase* base = objectForIndex(parent);
  if (!base)
    return QModelIndex();

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    switch (context->type()) {
      case DUContext::Class: {
        if (row < context->childContexts().count())
          return createIndex(row, column, context->childContexts().at(row));
        int rowOffset = context->childContexts().count();
        if (row - rowOffset < context->localDeclarations().count())
          return createIndex(row, column, context->localDeclarations().at(row - rowOffset));
        rowOffset += context->localDeclarations().count();
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
  QMutexLocker lock(&m_mutex);

  if (!parent.isValid())
    return m_topObjects.count();

  DUChainBase* base = objectForIndex(parent);
  if (!base)
    return 0;

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    switch (context->type()) {
      case DUContext::Class: {
        return context->childContexts().count() + context->localDeclarations().count();
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
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(index);
  if (!base)
    return QModelIndex();

  if (DUContext* context = dynamic_cast<DUContext*>(base))
    if (context->parentContext())
      return createIndex(context->parentContext()->childContexts().indexOf(context), 0, context->parentContext());
    else
      return QModelIndex();

  else if (Declaration* dec = dynamic_cast<Declaration*>(base))
    return createIndex(dec->context()->childContexts().count() + dec->context()->localDeclarations().indexOf(dec), 0, dec->context());

  else if (Definition* def = dynamic_cast<Definition*>(base))
    return createIndex(def->context()->childContexts().count() + def->context()->localDeclarations().count() + def->context()->localDefinitions().indexOf(def), 0, def->context());

  // Shouldn't really hit this
  Q_ASSERT(false);
  return QModelIndex();
}

QVariant ClassModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(index);
  if (!base)
    return QVariant();

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Context: %1", context->localScopeIdentifier().toString());
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Declaration: %1", dec->identifier().toString());
    }

  } else if (Definition* def = dynamic_cast<Definition*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Definition: %1", def->declaration()->identifier().toString());
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

void ClassModel::contextChanged(DUContext * context, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(relationship);

  QMutexLocker lock(&m_mutex);

  if (!context) {
    switch (change) {
      case Deletion:
      case Removal:
        if (m_topObjects.contains(context))
          scheduleRemoval(context);
        break;

      case Change:
        break;

      case Addition:
        scheduleAddition(context);
        break;
    }
  }
}

void ClassModel::declarationChanged(Declaration * declaration, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(declaration);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void ClassModel::definitionChanged(Definition * definition, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(definition);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void ClassModel::useChanged(Use * use, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(use);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void ClassModel::slotScheduleAddition(KDevelop::DUContext* context)
{
  QMutexLocker lock(&m_mutex);
  int index = m_topObjects.count();
  beginInsertRows(QModelIndex(), index, index);
  m_topObjects.append(context);
  endInsertRows();
}

void ClassModel::slotScheduleRemoval(KDevelop::DUContext* context)
{
  QMutexLocker lock(&m_mutex);
  int index = m_topObjects.indexOf(context);
  beginRemoveRows(QModelIndex(), index, index);
  m_topObjects.removeAt(index);
  endRemoveRows();
}

#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
