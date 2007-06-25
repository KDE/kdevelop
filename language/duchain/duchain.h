/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DUCHAIN_H
#define DUCHAIN_H

#include <QtCore/QObject>

#include <languageexport.h>
#include <duchainobserver.h>

class KUrl;

namespace KDevelop
{

class TopDUContext;
class DUChainLock;

class IdentifiedFile;
class ParsingEnvironmentManager;
class ParsingEnvironment;

/**
 * Holds references to all top level source file contexts.
 *
 * \todo to pull the sorting off properly, will need to know the location of
 *       the defines used to pull in URLs other than the source file URL.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChain : public QObject
{
  Q_OBJECT

public:
  /**
   * Return any chain for the given document
   * */
  TopDUContext* chainForDocument(const KUrl& document);
  
  /**
   * Find the chain based on file-url and identity-number. If the number is zero, any chain for the given url is returned.
   * */
  TopDUContext* chainForDocument(const IdentifiedFile& document);

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * */
  TopDUContext* chainForDocument(const KUrl& document, const ParsingEnvironment* environment);

  void addDocumentChain(const IdentifiedFile& document, TopDUContext* chain);

  void clear();

  static DUChain* self();

  /**
   * Retrieve the read write lock for the entire definition-use chain.
   * To call non-const methods, you must be holding a write lock.
   *
   * Evaluations made prior to holding a lock (including which objects
   * exist) must be verified once the lock is held, as they may have changed
   * or been deleted.
   *
   * \threadsafe
   */
  static DUChainLock* lock();

  const QList<DUChainObserver*>& observers() const;
  void addObserver(DUChainObserver* observer);
  void removeObserver(DUChainObserver* observer);

  // Distribute the notifications
  static void contextChanged(DUContext* context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void declarationChanged(Declaration* declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void definitionChanged(Definition* definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void useChanged(Use* use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);

  /**
   * @see ParsingEnvironmentManager
   * */
  void addParsingEnvironmentManager( ParsingEnvironmentManager* manager );

  ///Remove a manager, for example when a language-part is unloaded
  void removeParsingEnvironmentManager( ParsingEnvironmentManager* manager );
  
public Q_SLOTS:
  void removeDocumentChain(const IdentifiedFile& document);

private:
  void addToEnvironmentManager( TopDUContext * chain );
  void removeFromEnvironmentManager( TopDUContext * chain );
  DUChain();
  ~DUChain();

  friend class DUChainPrivate;
};
}

#endif // DUCHAIN_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
