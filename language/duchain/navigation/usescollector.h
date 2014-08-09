/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_USESCOLLECTOR_H
#define KDEVPLATFORM_USESCOLLECTOR_H

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <language/duchain/topducontext.h>
#include <serialization/indexedstring.h>

namespace KDevelop {
    class IndexedDeclaration;
    class IndexedString;
    ///A helper base-class for collecting the top-contexts that contain all uses of a declaration
    ///The most important part is that this also updates the duchain if it's not up-to-date or doesn't contain
    ///the required features. The virtual function processUses(..) is called with each up-to-date top-context found
    ///that contains uses of the declaration.
    class KDEVPLATFORMLANGUAGE_EXPORT UsesCollector : public QObject {
        Q_OBJECT
        public:
            UsesCollector(IndexedDeclaration declaration);
            virtual ~UsesCollector();
            
            ///@warning For most tasks, you should use declarations() instead, and respect all of them!
            IndexedDeclaration declaration() const;
            
            ///This must be called to start the actual collecting!
            void startCollecting();
            
            ///Override this to decide whether a file should be respect while computation.
            ///If true is returned, the uses will be computed for this file, and for all files
            ///in the import-chain between that file, and the file where the declaration occurs.
            ///The default-implementation returns true if the file is part of an open project, or
            ///if no project is opened and the file is open in an editor.
            virtual bool shouldRespectFile(IndexedString url);
            
            bool isReady() const;
            
            ///If this is true, the complete overload-chain is computed, and the uses of all overloaded functions together
            ///are computed.
            ///They are also returned in declarations():
            ///The default is "true"
            void setCollectOverloads(bool collect);
            
            ///If this is true, all definitions are loaded too, and part of the processed declarations.
            ///This also means that the collector will first jump from any definition to its declaration, and start
            ///collecting from there.
            ///They are also returned in declarations():
            ///The default is "true"
            void setCollectDefinitions(bool collectDefinition);

            ///Use this to set whether processUses should also be called on contexts that only contain
            ///a declaration that was used for searching the uses
            ///The default is "true".
            void setProcessDeclarations(bool process);

            ///If the searched declaration is a class, this can be used to decide whether constructors
            ///searched as well. The constructors and destructors will also be part of the declarations() list.
            ///The default is "true". This only works with constructors that have the same name as the class.
            ///If this is set to true, also destructors are searched and eventually renamed.
            void setCollectConstructors(bool process);
            
            ///The declarations that were used as base for the search
            ///For classes this contains forward-declarations etc.
            ///@warning When doing refactoring, you have to respect all of these as possible used declarations,
            ///         even within the same context. Multiple different of them may have been used in the same top-context,
            ///         with different local use-indices.
            QList<IndexedDeclaration> declarations();
        Q_SIGNALS:
            ///@see maximumProgress()
            void maximumProgressSignal(uint);
            ///@see progress()
            void progressSignal(uint, uint);
            ///@see processUses()
            void processUsesSignal(KDevelop::ReferencedTopDUContext);
        private slots:
            void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext);
        private:
            ///Called with every top-context that can contain uses of the declaration, or if setProcessDeclarations(false)
            ///has not been called also with all contexts that contain declarations used as base for the search.
            ///Override this to do your custom processing. You do not need to recurse into imports, that's done for you.
            ///The duchain is not locked when this is called.
            virtual void processUses(KDevelop::ReferencedTopDUContext topContext) = 0;
            
            ///Indicates the maximum progress this collector can make. If zero is given, this collector is not going to 
            ///do any progress, and progress(..) is never called.
            virtual void maximumProgress(uint max);
            
            ///Called whenever progress has been made. max equals the value given initially by maximumProgress
            virtual void progress(uint processed, uint max);
            
            IndexedDeclaration m_declaration;
            QSet<IndexedString> m_waitForUpdate;
            QSet<IndexedString> m_updateReady;
            
            //All files that already have been feed to processUses
            QSet<IndexedString> m_processed;
            
            //To prevent endless recursion in updateReady()
            QSet<IndexedTopDUContext> m_checked;
            
            ///Set of all files where the features were manipulated statically through ParseJob
            QSet<IndexedString> m_staticFeaturesManipulated;
            
            QList<IndexedDeclaration> m_declarations;
            QSet<IndexedTopDUContext> m_declarationTopContexts;
            
            bool m_collectOverloads;
            bool m_collectDefinitions;
            bool m_collectConstructors;
            bool m_processDeclarations;
    };
}

#endif
