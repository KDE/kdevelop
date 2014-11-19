/*  This file is part of KDevelop
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>
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

#include <KPluginFactory>

#include <interfaces/idocumentation.h>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <serialization/indexedstring.h>
#include <language/duchain/parsingenvironment.h>

#include <language/duchain/classdeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <QtCore/QFile>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iprojectcontroller.h>

#include "manpageplugin.h"
#include "manpagedocumentation.h"
#include "manpagemodel.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ManPageFactory, registerPlugin<ManPagePlugin>(); )

ManPagePlugin::ManPagePlugin(QObject* parent, const QVariantList& args)
    : IPlugin("kdevmanpage", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IDocumentationProvider )
    Q_UNUSED(args);
    ManPageDocumentation::s_provider = this;
    m_model = new ManPageModel(this);
}

ManPagePlugin::~ManPagePlugin()
{
    delete m_model;
}

QString ManPagePlugin::name() const
{
    return QString(i18n("Man Page"));
}

QIcon ManPagePlugin::icon() const
{
    static QIcon icon = QIcon::fromTheme("x-office-address-book");
    return icon;
}

ManPageModel* ManPagePlugin::model() const{
    return m_model;
}

IDocumentation::Ptr ManPagePlugin::documentationForDeclaration( Declaration* dec ) const
{
    Q_ASSERT(dec);
    Q_ASSERT(dec->topContext());
    Q_ASSERT(dec->topContext()->parsingEnvironmentFile());
    static const IndexedString cppLanguage("C++");
    if (dec->topContext()->parsingEnvironmentFile()->language() != cppLanguage) {
        return {};
    }
    
    // Don't show man-page documentation for files that are part of our project
    if(core()->projectController()->findProjectForUrl(dec->topContext()->url().toUrl()))
        return {};

    // Don't show man-page documentation for files that are not in /usr/include, because then we
    // most probably will be confusing the global function-name with a local one
    if(!dec->topContext()->url().str().startsWith("/usr/"))
        return {};
    
    ///@todo Do more verification to make sure that we're showing the correct documentation for the declaration

    QString identifier = dec->identifier().toString();
    if(m_model->containsIdentifier(identifier)){
        DUChainReadLocker lock;
        QualifiedIdentifier qid = dec->qualifiedIdentifier();
        if(qid.count() == 1){
            if(m_model->identifierInSection(identifier, "3")){
                return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl("man:(3)/"+identifier)));
            } else if(m_model->identifierInSection(identifier, "2")){
                return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl("man:(2)/"+identifier)));
            } else {
                return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl("man:"+identifier)));
            }
        }
    }
    return  {};
}

QAbstractListModel* ManPagePlugin::indexModel() const
{
    return m_model->indexList();
}

IDocumentation::Ptr ManPagePlugin::documentationForIndex(const QModelIndex& index) const
{
    QString name = index.data().toString();
    return IDocumentation::Ptr(new ManPageDocumentation(name, QUrl("man:"+name)));
}

IDocumentation::Ptr ManPagePlugin::homePage() const
{
    return IDocumentation::Ptr(new ManPageHomeDocumentation);
}

#include "manpageplugin.moc"
