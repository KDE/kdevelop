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
    : IPlugin(ManPageFactory::componentData(), parent), m_model(new ManPageModel(this))
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProvider )
    Q_UNUSED(args);
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
    QString fname;

    if ( ClassFunctionDeclaration* fdec = dynamic_cast<ClassFunctionDeclaration*>( dec ) ) {

        if ( dec->context() && dec->context()->type() == DUContext::Class && dec->context()->owner() ) {
            QString className = dec->context()->owner()->identifier().toString();

            if ( !isLocal ) {
                fname = className + '->' + fdec->identifier().toString();
            } else {
                if ( fdec->isConstructor() ) {
                    fname = className;
                } else if ( fdec->isDestructor() ) {
                    fname = "~" + className;
                } else {
                    fname = fdec->identifier().toString();
                }
            }
        }
    } else if ( dynamic_cast<ClassDeclaration*>(dec) ) {
        fname = "class." + dec->identifier().toString();
    } else if ( dynamic_cast<FunctionDeclaration*>(dec) ) {
        fname = "function." + dec->identifier().toString();
    }


    if ( !fname.isEmpty() && isLocal ) {
        fname = fname.toLower();
    }

    return "man:"+fname;
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForDeclaration( Declaration* dec ) const
{
    if ( dec ) {
        DUChainReadLocker lock( DUChain::lock() );

        // filter non global or non-php declarations
        if ( dec->topContext()->url() != m_model->internalFunctionFile() ) {
            return KSharedPtr<IDocumentation>();
        }

        KUrl url = KUrl("man:(3)");

        QString file = getDocumentationFilename( dec, url.isLocalFile() );
        if ( file.isEmpty() ) {
            kDebug() << "no documentation pattern found for" << dec->toString();
            return KSharedPtr<IDocumentation>();
        }

        url.addPath( file );
        if ( url.isLocalFile() && !QFile::exists( url.toLocalFile() ) ) {
            kDebug() << "bad path" << url << "for documentation of" << dec->toString() << " - aborting";
            return KSharedPtr<IDocumentation>();
        }

        kDebug() << "man page documentation located at " << url << "for" << dec->toString();
        return documentationForUrl(url, dec->qualifiedIdentifier().toString(), dec->comment());
    }

    return KSharedPtr<IDocumentation>();
}

QAbstractListModel* ManPagePlugin::indexModel() const
{
    return m_model;
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForIndex(const QModelIndex& index) const
{
    return documentationForDeclaration(static_cast<Declaration*>(
        index.data(ManPageModel::DeclarationRole).value<DeclarationPointer>().data()
    ));
}

void ManPagePlugin::loadUrl(const QUrl& url) const
{
    kDebug() << "loading URL" << url;
    KSharedPtr<IDocumentation> doc = documentationForUrl(url, QString());
    ICore::self()->documentationController()->showDocumentation(doc);
}

KSharedPtr< IDocumentation > ManPagePlugin::documentationForUrl(const KUrl& url, const QString& name, const QByteArray& description) const
{
    return KSharedPtr<IDocumentation>(new ManPageDocumentation( url, name, description, const_cast<ManPagePlugin*>(this)));
}

KSharedPtr< IDocumentation > ManPagePlugin::homePage() const
{
    KUrl url = KUrl("");
    return documentationForUrl(url, i18n("Man Documentation"));
}



