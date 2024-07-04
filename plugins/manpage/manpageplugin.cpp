/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "manpageplugin.h"

#include "manpagedocumentation.h"
#include "manpagemodel.h"

#include <interfaces/idocumentation.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <serialization/indexedstring.h>
#include <language/duchain/parsingenvironment.h>

#include <language/duchain/classdeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <KPluginFactory>

#include <QFile>
#include <QStringListModel>


using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ManPageFactory, "kdevmanpage.json", registerPlugin<ManPagePlugin>(); )

ManPagePlugin::ManPagePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : IPlugin(QStringLiteral("kdevmanpage"), parent, metaData)
{
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
    return i18n("Man Page");
}

QIcon ManPagePlugin::icon() const
{
    return QIcon::fromTheme(QStringLiteral("application-x-troff-man"));
}

ManPageModel* ManPagePlugin::model() const{
    return m_model;
}

IDocumentation::Ptr ManPagePlugin::documentationForDeclaration( Declaration* dec ) const
{
    Q_ASSERT(dec);
    Q_ASSERT(dec->topContext());
    Q_ASSERT(dec->topContext()->parsingEnvironmentFile());

    static const IndexedString cppLanguage("C++"); // TODO remove because of new clang parser?
    static const IndexedString clangLanguage("Clang");
    const IndexedString declarationLanguage(dec->topContext()->parsingEnvironmentFile()->language());
    if (declarationLanguage != cppLanguage && declarationLanguage != clangLanguage)
        return {};

    // Don't show man-page documentation for files that are part of our project
    if (core()->projectController()->findProjectForUrl(dec->topContext()->url().toUrl()))
        return {};

    // Don't show man-page documentation for files that are not in /usr/include, because then we
    // most probably will be confusing the global function-name with a local one
    if (!dec->topContext()->url().str().startsWith(QLatin1String("/usr/")))
        return {};

    ///@todo Do more verification to make sure that we're showing the correct documentation for the declaration

    QString identifier;
    IDocumentation::Ptr result;

    // First, try to find help for qualified identifier like 'std::vector'
    {
        DUChainReadLocker lock;
        identifier = dec->qualifiedIdentifier().toString(RemoveTemplateInformation);
    }

    result = documentationForIdentifier(identifier);
    if (result.data())
        return result;

    // Second, try to find help for simple identifier like 'sin'
    {
        DUChainReadLocker lock;
        identifier = dec->identifier().toString(RemoveTemplateInformation);
    }

    result = documentationForIdentifier(identifier);
    if (result.data())
        return result;

    return {};
}

KDevelop::IDocumentation::Ptr ManPagePlugin::documentationForIdentifier(const QString& identifier) const
{
    if (!m_model->containsIdentifier(identifier))
        return KDevelop::IDocumentation::Ptr(nullptr);

    if (m_model->identifierInSection(identifier, QStringLiteral("3")))
        return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl(QLatin1String("man:(3)/") + identifier)));

    if (m_model->identifierInSection(identifier, QStringLiteral("2")))
        return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl(QLatin1String("man:(2)/") + identifier)));

    return IDocumentation::Ptr(new ManPageDocumentation(identifier, QUrl(QLatin1String("man:/") + identifier)));
}

KDevelop::IDocumentation::Ptr ManPagePlugin::documentation(const QUrl& url) const
{
    if (url.toString().startsWith(QLatin1String("man"))) {
        IDocumentation::Ptr newDoc(new ManPageDocumentation(url.path(), url));
        return newDoc;
    }
    return {};
}

QAbstractItemModel* ManPagePlugin::indexModel() const
{
    return m_model->indexList();
}

IDocumentation::Ptr ManPagePlugin::documentationForIndex(const QModelIndex& index) const
{
    QString name = index.data().toString();
    return IDocumentation::Ptr(new ManPageDocumentation(name, QUrl(QLatin1String("man:")+name)));
}

IDocumentation::Ptr ManPagePlugin::homePage() const
{
    return IDocumentation::Ptr(new ManPageHomeDocumentation);
}

#include "manpageplugin.moc"
#include "moc_manpageplugin.cpp"
