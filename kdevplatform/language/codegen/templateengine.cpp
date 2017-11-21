/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "templateengine.h"

#include "debug.h"
#include "templateengine_p.h"

#include "codedescription.h"
#include "codedescriptionmetatypes.h"
#include "archivetemplateloader.h"

#include <interfaces/icore.h>
#include <QStandardPaths>

using namespace KDevelop;
using namespace Grantlee;

TemplateEngine* TemplateEngine::self()
{
    static TemplateEngine* engine = new TemplateEngine;
    return engine;
}

TemplateEngine::TemplateEngine()
: d(new TemplateEnginePrivate)
{
    d->engine.setSmartTrimEnabled(true);

    qCDebug(LANGUAGE) << "Generic data locations:" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    const auto templateDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
        QStringLiteral("kdevcodegen/templates"), QStandardPaths::LocateDirectory);

    if (!templateDirectories.isEmpty()) {
        qCDebug(LANGUAGE) << "Found template directories:" << templateDirectories;
        addTemplateDirectories(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kdevcodegen/templates"), QStandardPaths::LocateDirectory));
    } else {
        qCWarning(LANGUAGE) << "No template directories found -- templating engine will not work!";
    }

    Grantlee::registerMetaType<KDevelop::VariableDescription>();
    Grantlee::registerMetaType<KDevelop::FunctionDescription>();
    Grantlee::registerMetaType<KDevelop::InheritanceDescription>();
    Grantlee::registerMetaType<KDevelop::ClassDescription>();

    d->engine.addTemplateLoader(QSharedPointer<AbstractTemplateLoader>(ArchiveTemplateLoader::self()));
}

TemplateEngine::~TemplateEngine()
{
}

void TemplateEngine::addTemplateDirectories(const QStringList& directories)
{
    FileSystemTemplateLoader* loader = new FileSystemTemplateLoader;
    loader->setTemplateDirs(directories);
    d->engine.addTemplateLoader(QSharedPointer<AbstractTemplateLoader>(loader));
}
