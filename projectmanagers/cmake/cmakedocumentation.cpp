/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakedocumentation.h"
#include "cmakeutils.h"
#include <KDebug>
#include <QString>
#include <QTextDocument>
#include <QStringListModel>
#include <interfaces/iproject.h>
#include <KStandardDirs>
#include <KIcon>
#include <KGlobalSettings>
#include <KMimeType>
#include <documentation/standarddocumentationview.h>
#include <language/duchain/declaration.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include "cmakemanager.h"
#include "cmakeparserutils.h"
#include "cmakehelpdocumentation.h"
#include "cmakedoc.h"

K_PLUGIN_FACTORY(CMakeSupportDocFactory, registerPlugin<CMakeDocumentation>(); )
K_EXPORT_PLUGIN(CMakeSupportDocFactory(KAboutData("kdevcmakedocumentation","kdevcmakedocumentation", ki18n("CMake Documentation"), "1.0", ki18n("Support for CMake documentation"), KAboutData::License_GPL)))

CMakeDocumentation* CMakeDoc::s_provider=0;
KDevelop::IDocumentationProvider* CMakeDoc::provider() const { return s_provider; }

CMakeDocumentation::CMakeDocumentation(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin( CMakeSupportDocFactory::componentData(), parent )
    , mCMakeCmd(KStandardDirs::findExe("cmake"))
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProvider )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeDocumentation )

    if (mCMakeCmd.isEmpty()) {
        return;
    }

    CMakeDoc::s_provider=this;
    m_index= new QStringListModel(this);
    initializeModel();
}

bool CMakeDocumentation::hasError() const
{
    return mCMakeCmd.isEmpty();
}

QString CMakeDocumentation::errorDescription() const
{
    return mCMakeCmd.isEmpty() ? i18n("cmake is not installed") : QString();
}

static const char* args[] = { "--help-command", "--help-variable", "--help-module", "--help-property", 0, 0 };

void CMakeDocumentation::delayedInitialization()
{
    for(int i=0; i<=Property; i++) {
        collectIds(QString(args[i])+"-list", (Type) i);
    }
    
    m_index->setStringList(m_typeForName.keys());
}

void CMakeDocumentation::collectIds(const QString& param, Type type)
{
    QStringList ids=CMakeParserUtils::executeProcess(mCMakeCmd, QStringList(param)).split('\n');
    ids.takeFirst();
    foreach(const QString& name, ids)
    {
        m_typeForName[name]=type;
    }
}

QStringList CMakeDocumentation::names(CMakeDocumentation::Type t) const
{
    return m_typeForName.keys(t);
}

QString CMakeDocumentation::descriptionForIdentifier(const QString& id, Type t) const
{
    QString desc;
    if(args[t]) {
        desc = Qt::escape(CMakeParserUtils::executeProcess(mCMakeCmd, QStringList(args[t]) << id.simplified()));
        int firstLine = desc.indexOf('\n');
        firstLine = desc.indexOf('\n', firstLine+1);
        if(firstLine>=0)
            desc = desc.mid(firstLine+1).trimmed(); //we remove the cmake version and the command name
        desc.replace(QLatin1String("\n       "), QLatin1String("\n"));
        desc = QString("<b>%1</b><pre>%2</pre>").arg(id).arg(desc);
    }

    return desc;
}

KSharedPtr<KDevelop::IDocumentation> CMakeDocumentation::description(const QString& identifier, const KUrl& file) const
{
    initializeModel(); //make it not queued
    if(!KMimeType::findByUrl(file)->is("text/x-cmake"))
        return KSharedPtr<KDevelop::IDocumentation>();
    
    kDebug() << "seeking documentation for " << identifier;
    QString desc;

    if(m_typeForName.contains(identifier)) {
        desc=descriptionForIdentifier(identifier, m_typeForName[identifier]);
    } else if(m_typeForName.contains(identifier.toLower())) {
        desc=descriptionForIdentifier(identifier, m_typeForName[identifier.toLower()]);
    } else if(m_typeForName.contains(identifier.toUpper())) {
        desc=descriptionForIdentifier(identifier, m_typeForName[identifier.toUpper()]);
    }
    
    KDevelop::IProject* p=KDevelop::ICore::self()->projectController()->findProjectForUrl(file);
    ICMakeManager* m=0;
    if(p)
        m=p->managerPlugin()->extension<ICMakeManager>();
    if(m)
    {
        QPair<QString, QString> entry = m->cacheValue(p, identifier);
        if(!entry.first.isEmpty())
            desc += i18n("<br /><em>Cache Value:</em> %1\n", entry.first);
        
        if(!entry.second.isEmpty())
            desc += i18n("<br /><em>Cache Documentation:</em> %1\n", entry.second);
    }
    
    if(desc.isEmpty())
        return KSharedPtr<KDevelop::IDocumentation>();
    else
        return KSharedPtr<KDevelop::IDocumentation>(new CMakeDoc(identifier, desc));
}

KSharedPtr<KDevelop::IDocumentation> CMakeDocumentation::documentationForDeclaration(KDevelop::Declaration* decl) const
{
    return description(decl->identifier().toString(), decl->url().toUrl());
}

KSharedPtr<KDevelop::IDocumentation > CMakeDocumentation::documentationForIndex(const QModelIndex& idx) const
{
    return description(idx.data().toString(), KUrl("CMakeLists.txt"));
}

QAbstractListModel* CMakeDocumentation::indexModel() const
{
    initializeModel();
    return m_index;
}

QIcon CMakeDocumentation::icon() const
{
    return KIcon("cmake");
}

QString CMakeDocumentation::name() const
{
    return "CMake";
}

KSharedPtr<KDevelop::IDocumentation> CMakeDocumentation::homePage() const
{
    if(!m_typeForName.isEmpty())
        const_cast<CMakeDocumentation*>(this)->delayedInitialization();
//     initializeModel();
    return KSharedPtr<KDevelop::IDocumentation>(new CMakeHomeDocumentation);
}

void CMakeDocumentation::initializeModel() const
{
    if(!m_typeForName.isEmpty())
        return;
    
    QMetaObject::invokeMethod(const_cast<CMakeDocumentation*>(this), "delayedInitialization", Qt::QueuedConnection);
}

//////////CMakeDoc

QWidget* CMakeDoc::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent)
{
    KDevelop::StandardDocumentationView* view = new KDevelop::StandardDocumentationView(findWidget, parent);
    view->setFont(KGlobalSettings::fixedFont());
    view->setHtml("<html><body style='background:#fff'><code>"+description()+"</code></body></html>");
    return view;
}
