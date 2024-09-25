/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpplugin.h"

#include "qthelpprovider.h"
#include "qthelpqtdoc.h"
#include "qthelp_config_shared.h"
#include "debug.h"
#include "qthelpconfig.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <util/owningrawpointercontainer.h>

#include <KPluginFactory>

#include <QDirIterator>
#include <QFileInfo>
#include <QHash>

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

    // Make all .qch file paths read from config absolute as required by loadQtHelpProvider().
    for (auto& path : pathList) {
        path = QFileInfo{path}.absoluteFilePath();
    }
    // Pass an absolute path searchDir to searchHelpDirectory() so that it appends absolute paths to pathList.
    searchDir = QFileInfo{searchDir}.absoluteFilePath();

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
        const auto& fileInfo = dirIt.nextFileInfo();
        pathList.append(fileInfo.filePath());
        nameList.append(fileInfo.baseName());
        iconList.append(logo);
        qCDebug(QTHELP) << "qch found:" << pathList.constLast();
    }
}


void QtHelpPlugin::loadQtHelpProvider(const QStringList& pathList, const QStringList& nameList, const QStringList& iconList)
{
    if (pathList.empty()) {
        qDeleteAll(m_qtHelpProviders);
        m_qtHelpProviders = {}; // shed unneeded capacity
        return;
    }

    // Most or all new providers will likely be the same as old ones. Put the old providers in a hash table
    // and attempt to reuse them instead of destroying and creating the same providers again, which is slow.
    KDevelop::OwningRawPointerContainer<QHash<QString, QtHelpProvider*>> oldProviders;
    oldProviders->reserve(m_qtHelpProviders.size());
    for (auto* const provider : std::as_const(m_qtHelpProviders)) {
        oldProviders->insert(provider->namespaceName(), provider);
    }
    m_qtHelpProviders.clear(); // keep the likely needed capacity

    for(int i=0; i < pathList.length(); i++) {
        QString fileName = pathList.at(i);
        QString name = nameList.at(i);
        QString iconName = iconList.at(i);

        // NOTE: fileName must be an absolute file path, because QHelpEngineCore::documentationFileName()
        // returns an absolute file path, and this return value is compared to fileName.
        Q_ASSERT(QFileInfo{fileName}.isAbsolute());

        const auto namespaceName = QHelpEngineCore::namespaceName(fileName);
        if (namespaceName.isEmpty()) {
            qCWarning(QTHELP) << "skipping documentation file with an empty namespace name:" << fileName;
            continue;
        }

        QtHelpProvider *provider = nullptr;
        if (const auto it = oldProviders->constFind(namespaceName); it != oldProviders->cend()) {
            if ((*it)->fileName() == fileName) {
                provider = *it; // reuse this matching old provider
            } else {
                // The namespace name determines the .qhc file name. Destroy the nonmatching old provider
                // with the same namespace to prevent .qhc file conflict with the new provider created below.
                delete *it;
            }
            oldProviders->erase(it);
        }

        if(!provider){
            provider = new QtHelpProvider(this, fileName, namespaceName, name, iconName);
        }else{
            // The file name and the namespace name already match. Update the name and the icon name.
            provider->setName(name);
            provider->setIconName(iconName);
        }

        bool exist = false;
        for (QtHelpProvider* existingProvider : std::as_const(m_qtHelpProviders)) {
            if (existingProvider->namespaceName() == namespaceName) {
                exist = true;
                break;
            }
        }

        if(!exist){
            m_qtHelpProviders.append(provider);
        }
    }
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
    return m_qtDoc->isQtHelpAvailable();
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
