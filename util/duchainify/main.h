/***************************************************************************
 *   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_MAIN_H
#define KDEVPLATFORM_MAIN_H

#include <QObject>
#include <QAtomicInt>
#include <QUrl>

#include <language/duchain/topducontext.h>

namespace KDevelop {
class ParseJob;
}

class KCmdLineArgs;

class Manager : public QObject {
    Q_OBJECT
    public:
        Manager(KCmdLineArgs* args);
        void addToBackgroundParser(QString path, KDevelop::TopDUContext::Features features);
        QSet<QUrl> waiting();
    private:
        QSet<QUrl> m_waiting;
        uint m_total;
        KCmdLineArgs* m_args;
        QAtomicInt m_allFilesAdded;

    public slots:
        // delay init into event loop so the DUChain can always shutdown gracefully
        void init();
        void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext);
        void finish();
};

#endif
