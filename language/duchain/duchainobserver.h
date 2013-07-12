/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVPLATFORM_DUCHAINOBSERVER_H
#define KDEVPLATFORM_DUCHAINOBSERVER_H

#include <QtCore/QObject>

#include <KDE/KUrl>

#include <KDE/KTextEditor/Range>

#include "duchainpointer.h"
#include "../interfaces/iproblem.h"

namespace KDevelop
{

class DUChainBase;
class DUContext;
class Declaration;
class Definition;
class Use;

/**
 * Abstract class for observers of the definition-use chain to receive
 * feedback on changes.
 *
 * \todo change name to DUChainNotifier ?
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainObserver : public QObject
{
  Q_OBJECT
  friend class DUChain;

public:
  /// Destructor.
  virtual ~DUChainObserver();

Q_SIGNALS:
  void branchAdded(KDevelop::DUContextPointer context);
  void branchModified(KDevelop::DUContextPointer context);
  void branchRemoved(KDevelop::DUContextPointer context);
};

}

#endif // KDEVPLATFORM_DUCHAINOBSERVER_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
