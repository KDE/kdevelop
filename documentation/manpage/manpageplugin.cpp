/*  This file is part of KDevelop
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

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

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KMimeType>
#include <KSettings/Dispatcher>
#include <KUrl>
#include <KIcon>

#include <interfaces/idocumentation.h>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/indexedstring.h>

#include <language/duchain/classdeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <QtCore/QFile>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

#include "manpageplugin.h"
#include "manpagedocumentation.h"
#include "manpagemodel.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ManPageFactory, registerPlugin<ManPagePlugin>(); )
K_EXPORT_PLUGIN(ManPageFactory(KAboutData("kdevmanpage","kdevmanpage", ki18n("ManPage"),
                                          "1", ki18n("Check Man Page documentation"),
                               KAboutData::License_GPL)))

ManPagePlugin::ManPagePlugin(QObject* parent, const QVariantList& args)
    : IPlugin(ManPageFactory::componentData(), parent)
    , m_index(new QStringListModel())
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProvider )
    Q_UNUSED(args);
    ManPageDocumentation::s_provider = this;

    kDebug() << "ManPagePlugin constructor";

}


QString ManPagePlugin::name() const
{
    return QString("Man Page");
}

QIcon ManPagePlugin::icon() const
{
    static KIcon icon("x-office-address-book");
    return icon;
}




QString ManPagePlugin::getDocumentationFilename( KDevelop::Declaration* dec, const bool& isLocal ) const
{
    return QString();
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForDeclaration( Declaration* dec ) const
{
    return KSharedPtr<IDocumentation>();
}

QAbstractListModel* ManPagePlugin::indexModel() const
{
    return m_index;
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForIndex(const QModelIndex& index) const
{
    return KSharedPtr<IDocumentation>();
}

void ManPagePlugin::loadUrl(const QUrl& url) const
{
    kDebug() << "loading URL" << url;
    KSharedPtr<IDocumentation> doc = documentationForUrl(url, QString());
    ICore::self()->documentationController()->showDocumentation(doc);
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForUrl(const KUrl& url, const QString& name, const QByteArray& description) const
{
    return KSharedPtr<IDocumentation>(new ManPageDocumentation( url, name, description));
}

KSharedPtr< IDocumentation > ManPagePlugin::homePage() const
{
    return KSharedPtr<KDevelop::IDocumentation>(new ManPageHomeDocumentation);
}

#include "manpageplugin.moc"
