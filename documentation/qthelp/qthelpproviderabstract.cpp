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

#include <QHelpIndexModel>
#include <QTemporaryFile>
#include <QHelpContentModel>

#include <KDebug>
#include <KStandardDirs>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include "qthelpdocumentation.h"

QtHelpProviderAbstract::QtHelpProviderAbstract(QObject *parent, const QString &collectionFileName, const QVariantList &args)
    : QObject(parent)
    , m_engine(KStandardDirs::locateLocal("appdata", collectionFileName, true))
{
    Q_UNUSED(args);
    if( !m_engine.setupData() ) {
        kWarning() << "Couldn't setup QtHelp Collection file";
    }
}

KSharedPtr< KDevelop::IDocumentation > QtHelpProviderAbstract::documentationForDeclaration(KDevelop::Declaration* dec) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    if(dec) {
        bool isQML = dec->topContext()->parsingEnvironmentFile()->language().str() == "QML/JS";
        QString id;
        if(isQML) {
            KDevelop::DUChainReadLocker lock;
            QString ns;
            bool isClass = dec->abstractType()->whichType() == KDevelop::AbstractType::TypeStructure;
            if(!isClass) {
                ns = dec->context()->owner()->abstractType()->toString();
                ns += "::" + dec->identifier().toString();
            } else {
                ns = dec->abstractType()->toString();
            }

            id = "QML." + ns;
        } else {
            KDevelop::DUChainReadLocker lock;
            KDevelop::QualifiedIdentifier qid = dec->qualifiedIdentifier();
            lock.unlock();
            id = qid.toStringList().join("::");
        }

        if(!id.isEmpty()) {
            QMap<QString, QUrl> links=m_engine.linksForIdentifier(id);

            kDebug() << "doc_found" << id << links;
            if(!links.isEmpty())
                return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(id, m_engine.linksForIdentifier(id)));
        }
    }

    return KSharedPtr<KDevelop::IDocumentation>();
}

QAbstractListModel* QtHelpProviderAbstract::indexModel() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return m_engine.indexModel();
}

KSharedPtr< KDevelop::IDocumentation > QtHelpProviderAbstract::documentationForIndex(const QModelIndex& idx) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    QString name=idx.data(Qt::DisplayRole).toString();
    return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(name, m_engine.indexModel()->linksForKeyword(name)));
}

void QtHelpProviderAbstract::jumpedTo(const QUrl& newUrl) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    QMap<QString, QUrl> info;
    info.insert(newUrl.toString(), newUrl);
    KSharedPtr<KDevelop::IDocumentation> doc(new QtHelpDocumentation(newUrl.toString(), info));
    emit addHistory(doc);
}

KSharedPtr<KDevelop::IDocumentation> QtHelpProviderAbstract::homePage() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return KSharedPtr<KDevelop::IDocumentation>(new HomeDocumentation);
}
