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
#include <QHelpContentModel>

#include <QStandardPaths>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include "qthelpnetwork.h"
#include "qthelpdocumentation.h"
#include "debug.h"

using namespace KDevelop;

namespace {
IDocumentation::Ptr documentationPtrFromUrl(const QUrl& url)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QList<QHelpLink> info {{url, url.toString()}};
#else
    QMap<QString, QUrl> info;
    info.insert(url.toString(), url);
#endif
    return IDocumentation::Ptr(new QtHelpDocumentation(url.toString(), info));
}
}

QtHelpProviderAbstract::QtHelpProviderAbstract(QObject *parent, const QString &collectionFileName, const QVariantList &args)
    : QObject(parent)
    , m_engine(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+QLatin1Char('/')+collectionFileName)
    , m_nam(new HelpNetworkAccessManager(&m_engine, this))
{
    Q_UNUSED(args);
    if( !m_engine.setupData() ) {
        qCWarning(QTHELP) << "Couldn't setup QtHelp Collection file";
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_engine.setUsesFilterEngine(true);
#endif
}


QtHelpProviderAbstract::~QtHelpProviderAbstract()
{
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForDeclaration(Declaration* dec) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    if (dec) {
        static const IndexedString qmlJs("QML/JS");
        QString id;

        {
            DUChainReadLocker lock;
            id = dec->qualifiedIdentifier().toString(RemoveTemplateInformation);
            if (dec->topContext()->parsingEnvironmentFile()->language() == qmlJs && !id.isEmpty())
                id = QLatin1String("QML.") + id;
        }

        if (!id.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            const QList<QHelpLink> links = m_engine.documentsForIdentifier(id);
#else
            QMap<QString, QUrl> links = m_engine.linksForIdentifier(id);
#endif

            if(!links.isEmpty())
                return IDocumentation::Ptr(new QtHelpDocumentation(id, links));
        }
    }

    return {};
}

KDevelop::IDocumentation::Ptr QtHelpProviderAbstract::documentation(const QUrl& url) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    //findFile returns a valid url even if we don't have a page for that documentationForURL
    auto data = m_engine.fileData(url);
    return data.isEmpty() ? IDocumentation::Ptr{} : documentationPtrFromUrl(url);
}

QAbstractItemModel* QtHelpProviderAbstract::indexModel() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return m_engine.indexModel();
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForIndex(const QModelIndex& idx) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    QString name=idx.data(Qt::DisplayRole).toString();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return IDocumentation::Ptr(new QtHelpDocumentation(name, m_engine.documentsForKeyword(name)));
#else
    return IDocumentation::Ptr(new QtHelpDocumentation(name, m_engine.indexModel()->linksForKeyword(name)));
#endif
}

void QtHelpProviderAbstract::jumpedTo(const QUrl& newUrl)
{
    auto doc = documentation(newUrl);
    IDocumentationController* controller = ICore::self()->documentationController();
    if (!doc) {
        doc = controller->documentation(newUrl);
        if (!doc) {
            // Follow the unsupported link and run the risk of displaying a blank page
            // if this is a broken local link. If this is an external link, we can follow it
            // and show the webpage. Our support for website navigation is pretty good.
            doc = documentationPtrFromUrl(newUrl);
        }
    }
    controller->showDocumentation(doc);
}

IDocumentation::Ptr QtHelpProviderAbstract::homePage() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return IDocumentation::Ptr(new HomeDocumentation);
}

bool QtHelpProviderAbstract::isValid() const
{
    return !m_engine.registeredDocumentations().isEmpty();
}

HelpNetworkAccessManager * QtHelpProviderAbstract::networkAccess() const
{
    return m_nam;
}
