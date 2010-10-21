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
#include <KComponentData>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

#include "qthelpsettings.h"
#include "qthelpprovider.h"
#include "qthelpqtdoc.h"

K_PLUGIN_FACTORY(QtHelpFactory, registerPlugin<QtHelpPlugin>(); )
K_EXPORT_PLUGIN(QtHelpFactory(KAboutData("kdevqthelp","kdevqthelp", ki18n("QtHelp"), "0.1", ki18n("Check Qt Help documentation"), KAboutData::License_GPL)))

QtHelpPlugin::QtHelpPlugin(QObject* parent, const QVariantList& args)
    : KDevelop::IPlugin(QtHelpFactory::componentData(), parent)
    , documentationProviders()
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProviderProvider )
    Q_UNUSED(args);

    readConfig();
    KSettings::Dispatcher::registerComponent( KComponentData("kdevqthelp_config"),
                                                this, "readConfig" );
    connect(this, SIGNAL(changedProvidersList()), KDevelop::ICore::self()->documentationController(), SLOT(changedDoucmentationProviders()));
}

void QtHelpPlugin::readConfig()
{
    QtHelpSettings::self()->readConfig();
    QStringList qtHelpPathList = QtHelpSettings::qchList();
    foreach(KDevelop::IDocumentationProvider* provider, documentationProviders) {
        documentationProviders.removeAll(provider);
        delete provider;
    }

    foreach(QString fileName,qtHelpPathList){
        documentationProviders.append(new QtHelpProvider(this, QtHelpFactory::componentData(), fileName, QVariantList()));
    }
    documentationProviders.append(new QtHelpQtDoc(this, QtHelpFactory::componentData(), QVariantList()));
    emit changedProvidersList();
}

QList<KDevelop::IDocumentationProvider*> QtHelpPlugin::providers()
{
    return documentationProviders;
}
