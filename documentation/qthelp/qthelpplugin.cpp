/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
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

#include "qthelpplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KSettings/Dispatcher>
#include <KDebug>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <QDirIterator>
#include "qthelpprovider.h"
#include "qthelpqtdoc.h"
#include "qthelp_config_shared.h"

QtHelpPlugin *QtHelpPlugin::s_plugin = 0;

K_PLUGIN_FACTORY_DEFINITION(QtHelpFactory, registerPlugin<QtHelpPlugin>(); )
K_EXPORT_PLUGIN(QtHelpFactory(KAboutData("kdevqthelp","kdevqthelp", ki18n("QtHelp"), "0.1", ki18n("Check Qt Help documentation"), KAboutData::License_GPL)))

QtHelpPlugin::QtHelpPlugin(QObject* parent, const QVariantList& args)
    : KDevelop::IPlugin(QtHelpFactory::componentData(), parent)
    , m_qtHelpProviders()
    , m_qtDoc(0)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProviderProvider )

    Q_UNUSED(args);
    s_plugin = this;
    KSettings::Dispatcher::registerComponent( KComponentData("kdevqthelp_config"),
                                                    this, "readConfig" );
    connect(this, SIGNAL(changedProvidersList()), KDevelop::ICore::self()->documentationController(), SLOT(changedDocumentationProviders()));
    QMetaObject::invokeMethod(this, "readConfig", Qt::QueuedConnection);
}

QtHelpPlugin::~QtHelpPlugin()
{
}


void QtHelpPlugin::readConfig()
{
    QStringList iconList, nameList, pathList, ghnsList;
    bool loadQtDoc;
    QString searchDir;
    qtHelpReadConfig(iconList, nameList, pathList, ghnsList, searchDir, loadQtDoc);

    searchHelpDirectory(pathList, nameList, iconList, searchDir);
    loadQtHelpProvider(pathList, nameList, iconList);
    loadQtDocumentation(loadQtDoc);

    emit changedProvidersList();
}

void QtHelpPlugin::loadQtDocumentation(bool loadQtDoc)
{
    if(m_qtDoc && !loadQtDoc){
        delete m_qtDoc;
        m_qtDoc = 0;
    } else if(!m_qtDoc && loadQtDoc) {
        m_qtDoc = new QtHelpQtDoc(this, QVariantList());
    }
}

void QtHelpPlugin::searchHelpDirectory(QStringList& pathList, QStringList& nameList, QStringList& iconList, const QString& searchDir)
{
    kDebug() << "Searching qch files in: " << searchDir;
    QDirIterator dirIt(searchDir, QStringList() << "*.qch", QDir::Files, QDirIterator::Subdirectories);
    const QString logo("qtlogo");
    while(dirIt.hasNext() == true)
    {
        dirIt.next();
        kDebug() << "qch found: " << dirIt.filePath();
        pathList.append(dirIt.filePath());
        nameList.append(dirIt.fileInfo().baseName());
        iconList.append(logo);
    }
}


void QtHelpPlugin::loadQtHelpProvider(QStringList pathList, QStringList nameList, QStringList iconList)
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
            QtHelpProvider *provider = 0;
            foreach(QtHelpProvider* oldProvider, oldList){
                if(QHelpEngineCore::namespaceName(oldProvider->fileName()) == nameSpace){
                    provider = oldProvider;
                    oldList.removeAll(provider);
                    break;
                }
            }
            if(!provider){
                provider = new QtHelpProvider(this, fileName, name, iconName, QVariantList());
            }else{
                provider->setName(name);
                provider->setIconName(iconName);
            }

            bool exist = false;
            foreach(QtHelpProvider* existingProvider, m_qtHelpProviders){
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
    foreach(QtHelpProvider* provider, m_qtHelpProviders) {
        list.append(provider);
    }
    if(m_qtDoc){
        list.append(m_qtDoc);
    }
    return list;
}

QList<QtHelpProvider*> QtHelpPlugin::qtHelpProviderLoaded()
{
    return m_qtHelpProviders;
}

bool QtHelpPlugin::qtHelpQtDocLoaded(){
    return m_qtDoc;
}
