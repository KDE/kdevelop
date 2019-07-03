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

#include <QProcess>
#include <QString>
#include <QStringListModel>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QIcon>
#include <KLocalizedString>
#include <KDescendantsProxyModel>

#include <interfaces/iproject.h>
#include <KPluginFactory>
#include <documentation/standarddocumentationview.h>
#include <language/duchain/declaration.h>
#include <serialization/indexedstring.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include "cmakemanager.h"
#include "cmakehelpdocumentation.h"
#include "cmakebuilderconfig.h"
#include "cmakedoc.h"
#include "cmakecommandscontents.h"
#include "debug.h"

K_PLUGIN_FACTORY_WITH_JSON(CMakeSupportDocFactory, "kdevcmakedocumentation.json", registerPlugin<CMakeDocumentation>(); )

CMakeDocumentation* CMakeDoc::s_provider=nullptr;
KDevelop::IDocumentationProvider* CMakeDoc::provider() const { return s_provider; }

CMakeDocumentation::CMakeDocumentation(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin( QStringLiteral("kdevcmakedocumentation"), parent )
    , m_index(new CMakeCommandsContents(this))
    , m_flatIndex(new KDescendantsProxyModel(m_index))
{
    m_flatIndex->setSourceModel(m_index);
    if (CMakeBuilderSettings::self()->cmakeExecutable().isEmpty()) {
        setErrorDescription(i18n("Unable to find a CMake executable. Is one installed on the system?"));
        return;
    }

    CMakeDoc::s_provider=this;
}

QVector<QString> CMakeDocumentation::names(CMakeDocumentation::Type t) const
{
    return m_index->names(t);
}

QString CMakeDocumentation::descriptionForIdentifier(const QString& id, Type t) const
{
    return m_index->descriptionForIdentifier(id, t);
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::description(const QString& identifier, const QUrl &file) const
{
    if (!file.isEmpty() && !QMimeDatabase().mimeTypeForUrl(file).inherits(QStringLiteral("text/x-cmake"))) {
        return KDevelop::IDocumentation::Ptr();
    }


    Type t = m_index->typeFor(identifier);
    QString desc=descriptionForIdentifier(identifier, t);

    KDevelop::IProject* p=KDevelop::ICore::self()->projectController()->findProjectForUrl(file);
    ICMakeManager* m=nullptr;
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
    return description(idx.data().toString(), QUrl());
}

QAbstractItemModel* CMakeDocumentation::indexModel() const
{
    return m_flatIndex;
}

QIcon CMakeDocumentation::icon() const
{
    return QIcon::fromTheme(QStringLiteral("cmake"));
}

QString CMakeDocumentation::name() const
{
    return QStringLiteral("CMake");
}

KDevelop::IDocumentation::Ptr CMakeDocumentation::homePage() const
{
    return KDevelop::IDocumentation::Ptr(new CMakeHomeDocumentation);
}

//////////CMakeDoc

QWidget* CMakeDoc::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent)
{
    auto* view = new KDevelop::StandardDocumentationView(findWidget, parent);
    view->initZoom(provider()->name());
    view->setHtml(mDesc);
    return view;
}

#include "cmakedocumentation.moc"
#include "moc_cmakedoc.cpp"
