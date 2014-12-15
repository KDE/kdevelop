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
#include <QString>
#include <QTextDocument>
#include <QStringListModel>
#include <QMimeDatabase>
#include <interfaces/iproject.h>
#include <KGlobalSettings>
#include <documentation/standarddocumentationview.h>
#include <language/duchain/declaration.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <QStandardPaths>
#include "cmakemanager.h"
#include "cmakeparserutils.h"
#include "cmakehelpdocumentation.h"
#include "cmakedoc.h"
#include "debug.h"

K_PLUGIN_FACTORY(CMakeSupportDocFactory, registerPlugin<CMakeDocumentation>(); )

CMakeDocumentation* CMakeDoc::s_provider=0;
KDevelop::IDocumentationProvider* CMakeDoc::provider() const { return s_provider; }

CMakeDocumentation::CMakeDocumentation(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin( "kdevcmakedocumentation", parent )
    , mCMakeCmd(QStandardPaths::findExecutable("cmake"))
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
    QStringList ids=CMake::executeProcess(mCMakeCmd, QStringList(param)).split('\n');
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
        desc = CMake::executeProcess(mCMakeCmd, QStringList(args[t]) << id.simplified()).toHtmlEscaped();
        int firstLine = desc.indexOf('\n');
        firstLine = desc.indexOf('\n', firstLine+1);
        if(firstLine>=0)
            desc = desc.mid(firstLine+1).trimmed(); //we remove the cmake version and the command name
        desc.replace(QLatin1String("\n       "), QLatin1String("\n"));
        desc = QString("<b>%1</b><pre>%2</pre>").arg(id).arg(desc);
    }

    return desc;
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::description(const QString& identifier, const QUrl &file) const
{
    initializeModel(); //make it not queued
    if (!QMimeDatabase().mimeTypeForUrl(file).inherits("text/x-cmake")) {
        return KDevelop::IDocumentation::Ptr();
    }

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
        return KDevelop::IDocumentation::Ptr();
    else
        return KDevelop::IDocumentation::Ptr(new CMakeDoc(identifier, desc));
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::documentationForDeclaration(KDevelop::Declaration* decl) const
{
    return description(decl->identifier().toString(), decl->url().toUrl());
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::documentationForIndex(const QModelIndex& idx) const
{
    return description(idx.data().toString(), QUrl("CMakeLists.txt"));
}

QAbstractListModel* CMakeDocumentation::indexModel() const
{
    initializeModel();
    return m_index;
}

QIcon CMakeDocumentation::icon() const
{
    return QIcon::fromTheme("cmake");
}

QString CMakeDocumentation::name() const
{
    return "CMake";
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::homePage() const
{
    if(!m_typeForName.isEmpty())
        const_cast<CMakeDocumentation*>(this)->delayedInitialization();
//     initializeModel();
    return KDevelop::IDocumentation::Ptr(new CMakeHomeDocumentation);
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

#include "cmakedocumentation.moc"
