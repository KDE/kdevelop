/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templateengine.h"
#include "templateengine_p.h"

#include "debug.h"

#include "codedescription.h"
#include "codedescriptionmetatypes.h"
#include "archivetemplateloader.h"

#include <interfaces/icore.h>
#include <QStandardPaths>

using namespace KDevelop;
using namespace KTextTemplate;

TemplateEngine* TemplateEngine::self()
{
    static auto* engine = new TemplateEngine;
    return engine;
}

TemplateEngine::TemplateEngine()
    : d_ptr(new TemplateEnginePrivate)
{
    Q_D(TemplateEngine);

    d->engine.setSmartTrimEnabled(true);

    qCDebug(LANGUAGE) << "Generic data locations:" << QStandardPaths::standardLocations(
        QStandardPaths::GenericDataLocation);

    const auto templateDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                               QStringLiteral(
                                                                   "kdevcodegen/templates"),
                                                               QStandardPaths::LocateDirectory);

    if (!templateDirectories.isEmpty()) {
        qCDebug(LANGUAGE) << "Found template directories:" << templateDirectories;
        addTemplateDirectories(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                         QStringLiteral("kdevcodegen/templates"),
                                                         QStandardPaths::LocateDirectory));
    } else {
        qCWarning(LANGUAGE) << "No template directories found -- templating engine will not work!";
    }

    KTextTemplate::registerMetaType<KDevelop::VariableDescription>();
    KTextTemplate::registerMetaType<KDevelop::FunctionDescription>();
    KTextTemplate::registerMetaType<KDevelop::InheritanceDescription>();
    KTextTemplate::registerMetaType<KDevelop::ClassDescription>();

    d->engine.addTemplateLoader(QSharedPointer<AbstractTemplateLoader>(ArchiveTemplateLoader::self()));
}

TemplateEngine::~TemplateEngine()
{
}

void TemplateEngine::addTemplateDirectories(const QStringList& directories)
{
    Q_D(TemplateEngine);

    auto* loader = new FileSystemTemplateLoader;
    loader->setTemplateDirs(directories);
    d->engine.addTemplateLoader(QSharedPointer<AbstractTemplateLoader>(loader));
}
