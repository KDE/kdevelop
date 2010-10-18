/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qthelpprovider.h"
#include <QtCore/QObject>

#include <KDebug>
#include <QHelpIndexModel>
#include <QTemporaryFile>
#include <QHelpContentModel>

#include <KProcess>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KStandardDirs>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <KIcon>
#include "qthelpdocumentation.h"

QtHelpProvider::QtHelpProvider(QObject *parent, const KComponentData &componentData, const QString &fileName, const QVariantList &args)
    : m_engine(KStandardDirs::locateLocal("appdata", QString( QHelpEngineCore::namespaceName(fileName) + ".qhc" ), true, componentData))
    , m_fileName(fileName)
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    if( !m_engine.setupData() ) {
        kWarning() << "Couldn't setup QtHelp Collection file, searching in Qt docs will fail";
    }
    m_engine.registerDocumentation(fileName);
}

KSharedPtr< KDevelop::IDocumentation > QtHelpProvider::documentationForDeclaration(KDevelop::Declaration* dec) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    if(dec) {
        QStringList idList;
        {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::QualifiedIdentifier qid = dec->qualifiedIdentifier();
        for(int a = 0; a < qid.count(); ++a)
            idList << qid.at(a).identifier().str(); //Copy over the identifier components, without the template-parameters
        }

        QString id = idList.join("::");
        if(!id.isEmpty()) {
            QMap<QString, QUrl> links=m_engine.linksForIdentifier(id);

            kDebug() << "doc_found" << id << links;
            if(!links.isEmpty())
                return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(id, m_engine.linksForIdentifier(id)));
        }
    }

    return KSharedPtr<KDevelop::IDocumentation>();
}

QAbstractListModel* QtHelpProvider::indexModel() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    return m_engine.indexModel();
}

KSharedPtr< KDevelop::IDocumentation > QtHelpProvider::documentationForIndex(const QModelIndex& idx) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    QString name=idx.data(Qt::DisplayRole).toString();
    return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(name, m_engine.indexModel()->linksForKeyword(name)));
}

QIcon QtHelpProvider::icon() const
{
    return KIcon("qtlogo");
}

QString QtHelpProvider::name() const
{
    if(m_engine.customFilters().isEmpty()) {
        return QHelpEngineCore::namespaceName(m_fileName);
    }
   return m_engine.customFilters().at(0);
}

void QtHelpProvider::jumpedTo(const QUrl& newUrl) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    QMap<QString, QUrl> info;
    info.insert(newUrl.toString(), newUrl);
    KSharedPtr<KDevelop::IDocumentation> doc(new QtHelpDocumentation(newUrl.toString(), info));
    emit addHistory(doc);
}

KSharedPtr<KDevelop::IDocumentation> QtHelpProvider::homePage() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProvider*>(this);
    return KSharedPtr<KDevelop::IDocumentation>(new HomeDocumentation);
}
