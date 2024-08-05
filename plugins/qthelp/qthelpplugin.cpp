/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpplugin.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include "qthelpprovider.h"
#include "qthelpqtdoc.h"
#include "qthelp_config_shared.h"
#include "debug.h"
#include "qthelpconfig.h"

#include <KPluginFactory>

#include <QDirIterator>

K_PLUGIN_FACTORY_WITH_JSON(QtHelpPluginFactory, "kdevqthelp.json", registerPlugin<QtHelpPlugin>(); )

QtHelpPlugin::QtHelpPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevqthelp"), parent, metaData)
    , m_qtHelpProviders()
    , m_qtDoc(new QtHelpQtDoc(this, QtHelpQtDoc::qmakeCandidates().value(0), QStringLiteral("qthelpcollection.qhc")))
    , m_loadSystemQtDoc(false)
{
    Q_UNUSED(args);
    connect(this, &QtHelpPlugin::changedProvidersList, KDevelop::ICore::self()->documentationController(), &KDevelop::IDocumentationController::changedDocumentationProviders);
    QMetaObject::invokeMethod(this, "readConfig", Qt::QueuedConnection);
}

QtHelpPlugin::~QtHelpPlugin()
{
}


void QtHelpPlugin::readConfig()
{
    QStringList iconList, nameList, pathList, ghnsList;
    QString searchDir;
    qtHelpReadConfig(iconList, nameList, pathList, ghnsList, searchDir, m_loadSystemQtDoc);

    searchHelpDirectory(pathList, nameList, iconList, searchDir);
    loadQtHelpProvider(pathList, nameList, iconList);
    loadQtDocumentation(m_loadSystemQtDoc);

    emit changedProvidersList();
}

void QtHelpPlugin::loadQtDocumentation(bool loadQtDoc)
{
    if (!m_qtDoc->isInitialized()) {
        // defer loading until initialization has finished
        connect(m_qtDoc, &QtHelpQtDoc::isInitializedChanged, this, [this, loadQtDoc]() {
            loadQtDocumentation(loadQtDoc);
        });
        return;
    }

    if(!loadQtDoc){
        m_qtDoc->unloadDocumentation();
    } else if(loadQtDoc) {
        m_qtDoc->loadDocumentation();
    }
}

void QtHelpPlugin::searchHelpDirectory(QStringList& pathList, QStringList& nameList, QStringList& iconList, const QString& searchDir)
{
    if (searchDir.isEmpty()) {
        return;
    }

    qCDebug(QTHELP) << "Searching qch files in: " << searchDir;
    QDirIterator dirIt(searchDir, QStringList() << QStringLiteral("*.qch"), QDir::Files, QDirIterator::Subdirectories);
    const QString logo(QStringLiteral("qtlogo"));
    while(dirIt.hasNext() == true)
    {
        dirIt.next();
        qCDebug(QTHELP) << "qch found: " << dirIt.filePath();
        pathList.append(dirIt.filePath());
        nameList.append(dirIt.fileInfo().baseName());
        iconList.append(logo);
    }
}


void QtHelpPlugin::loadQtHelpProvider(const QStringList& pathList, const QStringList& nameList, const QStringList& iconList)
{
    QList<QtHelpProvider*> oldList(m_qtHelpProviders);
    m_qtHelpProviders.clear();
    for(int i=0; i < pathList.length(); i++) {
        // check if provider already exist
        QString fileName = pathList.at(i);
        QString name = nameList.at(i);
        QString iconName = iconList.at(i);
        QString nameSpace = QHelpEngineCore::namespaceName(fileName);
        if(!nameSpace.isEmpty()){
            QtHelpProvider *provider = nullptr;
            for (QtHelpProvider* oldProvider : std::as_const(oldList)) {
                if(QHelpEngineCore::namespaceName(oldProvider->fileName()) == nameSpace){
                    provider = oldProvider;
                    oldList.removeAll(provider);
                    break;
                }
            }
            if(!provider){
                provider = new QtHelpProvider(this, fileName, name, iconName);
            }else{
                provider->setName(name);
                provider->setIconName(iconName);
            }

            bool exist = false;
            for (QtHelpProvider* existingProvider : std::as_const(m_qtHelpProviders)) {
                if(QHelpEngineCore::namespaceName(existingProvider->fileName()) ==  nameSpace){
                    exist = true;
                    break;
                }
            }

            if(!exist){
                m_qtHelpProviders.append(provider);
            }
        }
    }

    // delete unused providers
    qDeleteAll(oldList);
}

QList<KDevelop::IDocumentationProvider*> QtHelpPlugin::providers()
{
    QList<KDevelop::IDocumentationProvider*> list;
    list.reserve(m_qtHelpProviders.size() + (m_loadSystemQtDoc?1:0));
    for (QtHelpProvider* provider : std::as_const(m_qtHelpProviders)) {
        list.append(provider);
    }
    if(m_loadSystemQtDoc){
        list.append(m_qtDoc);
    }
    return list;
}

QList<QtHelpProvider*> QtHelpPlugin::qtHelpProviderLoaded()
{
    return m_qtHelpProviders;
}

bool QtHelpPlugin::isQtHelpQtDocLoaded() const
{
    return m_loadSystemQtDoc && m_qtDoc->isInitialized();
}

bool QtHelpPlugin::isQtHelpAvailable() const
{
    return !m_qtDoc->qchFiles().isEmpty();
}

bool QtHelpPlugin::isInitialized() const
{
    return m_qtDoc->isInitialized();
}

KDevelop::ConfigPage* QtHelpPlugin::configPage(int number, QWidget* parent)
{
    if (number == 0) {
        return new QtHelpConfig(this, parent);
    }
    return nullptr;
}

int QtHelpPlugin::configPages() const
{
    return 1;
}

#include "qthelpplugin.moc"
#include "moc_qthelpplugin.cpp"
