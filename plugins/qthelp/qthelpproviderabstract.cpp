/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpprovider.h"

#include <QHelpIndexModel>
#include <QHelpContentModel>

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
IDocumentation::Ptr documentationPtrFromUrl(QtHelpProviderAbstract* provider, const QUrl& url)
{
    const QList<QHelpLink> info{{url, url.toString()}};
    return IDocumentation::Ptr(new QtHelpDocumentation(provider, url.toString(), info));
}
}

QtHelpProviderAbstract::QtHelpProviderAbstract(QObject* parent, const QString& collectionFilePath)
    : QObject(parent)
    , m_engine(collectionFilePath)
    , m_nam(new HelpNetworkAccessManager(&m_engine, this))
{
    connect(&m_engine, &QHelpEngine::warning, this, [collectionFilePath](const QString& msg) {
        qCWarning(QTHELP) << "engine warning for" << collectionFilePath << msg;
    });

    // we use a writable engine (see initialization above)
    // in Qt6 we must mark the engine as writable, otherwise registering will always fail
    m_engine.setReadOnly(false);

    // we assume that the setup finished synchronously, otherwise our code does not work correctly
    // the below will catch situations when Qt would change its behavior
    bool setupFinished = false;
    auto startedConnection = connect(&m_engine, &QHelpEngine::setupStarted, this, [collectionFilePath]() {
        qCDebug(QTHELP) << "setup started" << collectionFilePath;
    });
    auto finishedConnection =
        connect(&m_engine, &QHelpEngine::setupFinished, this, [&setupFinished, collectionFilePath]() {
            qCDebug(QTHELP) << "setup finished" << collectionFilePath;
            setupFinished = true;
        });

    if( !m_engine.setupData() ) {
        qCWarning(QTHELP) << "Couldn't setup QtHelp Collection file";
    }

    Q_ASSERT(setupFinished);
    disconnect(startedConnection);
    disconnect(finishedConnection);

    Q_ASSERT(!m_engine.isReadOnly());

    m_engine.setUsesFilterEngine(true);
}

QtHelpProviderAbstract::~QtHelpProviderAbstract()
{
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForDeclaration(Declaration* dec) const
{
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
            const QList<QHelpLink> links = m_engine.documentsForIdentifier(id);
            if(!links.isEmpty())
                return IDocumentation::Ptr(
                    new QtHelpDocumentation(const_cast<QtHelpProviderAbstract*>(this), id, links));
        }
    }

    return {};
}

KDevelop::IDocumentation::Ptr QtHelpProviderAbstract::documentation(const QUrl& url) const
{
    //findFile returns a valid url even if we don't have a page for that documentationForURL
    auto data = m_engine.fileData(url);
    return data.isEmpty() ? IDocumentation::Ptr{}
                          : documentationPtrFromUrl(const_cast<QtHelpProviderAbstract*>(this), url);
}

QAbstractItemModel* QtHelpProviderAbstract::indexModel() const
{
    return m_engine.indexModel();
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForIndex(const QModelIndex& idx) const
{
    QString name = idx.data(Qt::DisplayRole).toString();
    return IDocumentation::Ptr(
        new QtHelpDocumentation(const_cast<QtHelpProviderAbstract*>(this), name, m_engine.documentsForKeyword(name)));
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
            doc = documentationPtrFromUrl(this, newUrl);
        }
    }
    controller->showDocumentation(doc);
}

IDocumentation::Ptr QtHelpProviderAbstract::homePage() const
{
    return IDocumentation::Ptr(new HomeDocumentation(const_cast<QtHelpProviderAbstract*>(this)));
}

bool QtHelpProviderAbstract::isValid() const
{
    return !m_engine.registeredDocumentations().isEmpty();
}

HelpNetworkAccessManager * QtHelpProviderAbstract::networkAccess() const
{
    return m_nam;
}

#include "moc_qthelpproviderabstract.cpp"
