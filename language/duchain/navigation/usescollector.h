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

#include <QObject>
#include <QSet>
#include <language/duchain/topducontext.h>
#include <language/duchain/indexedstring.h>

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
            ~UsesCollector();
            IndexedDeclaration declaration() const;
            
            ///Override this to decide whether a file should be respect while computation.
            ///If true is returned, the uses will be computed for this file, and for all files
            ///in the import-chain between that file, and the file where the declaration occurs.
            ///The default-implementation returns true if the file is part of an open project, or
            ///if no project is opened and the file is open in an editor.
            virtual bool shouldRespectFile(IndexedString url);
        private slots:
            void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext);
        protected:
            //This must be called to start the actual collecting
            void startCollecting();
        private:
            ///Called with every top-context that can contain uses of the declaration.
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
            
            //All files that already have been feeded to processUses
            QSet<IndexedString> m_processed;
            
            //To prevent endless recursion in updateReady()
            QSet<IndexedTopDUContext> m_checked;
            
            ///Set of all files where the features were manipulated statically through ParseJob
            QSet<IndexedString> m_staticFeaturesManipulated;
    };
}