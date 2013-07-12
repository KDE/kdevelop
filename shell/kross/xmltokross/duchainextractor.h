/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
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

#ifndef KDEVPLATFORM_DUCHAINEXTRACTOR_H
#define KDEVPLATFORM_DUCHAINEXTRACTOR_H

#include <QObject>
#include <QStringList>
#include <KUrl>

namespace KDevelop {
    class ILanguageSupport;
    class ParseJob;
}

class DummyBSM;

class DUChainExtractor : public QObject
{
    Q_OBJECT
    public:
        DUChainExtractor(QObject* parent=0);
        void start(const KUrl& _input, const KUrl& builddir, const KUrl::List& includes, 
                   const QString& filename, const QString& directory, const QStringList& toinclude, const QString& output);
        bool isDone() const { return m_done; }
        void setWriteImpl(bool w) { m_writeImpl=w; }
    public slots:
        void parsingFinished(KDevelop::ParseJob* job);
        void progressUpdated(int minimum, int maximum, int value);
        
    private:
        KUrl input;
        QString m_output;
        QString m_filename;
        QString m_directory;
        QStringList m_toinclude;
        bool m_done;
        DummyBSM* m_manager;
        bool m_writeImpl;
};

#endif
