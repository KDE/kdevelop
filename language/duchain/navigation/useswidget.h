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
#ifndef USESWIDGET_H
#define USESWIDGET_H

#include <QWidget>

#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include "../../languageexport.h"

class KComboBox;
class QComboBox;

namespace KDevelop {
    class IndexedDeclaration;
    /**
     * Represents the uses of a declaration within one top-context
     */
    class KDEVPLATFORMLANGUAGE_EXPORT TopContextUsesWidget : public QWidget {
        Q_OBJECT
        public:
            TopContextUsesWidget(IndexedDeclaration declaration, IndexedTopDUContext topContext);
        private:
            IndexedTopDUContext m_topContext;
            IndexedDeclaration m_declaration;
    };
    /**
     * A widget that allows browsing through all the uses of a declaration
     */
    class KDEVPLATFORMLANGUAGE_EXPORT UsesWidget : public QWidget {
        Q_OBJECT
        public:
            UsesWidget(IndexedDeclaration declaration);
        Q_SIGNALS:
            ///Emitted whenever the state is toggled
            void showingUses(bool showing);
        private Q_SLOTS:
            void showUsesForButton();
        private:
            //Duchain needs to be locked
            QList<IndexedTopDUContext> allUsingContexts();
            IndexedDeclaration m_declaration;
            bool m_showingUses;
            QComboBox* m_usingFiles;
    };
}

#endif
