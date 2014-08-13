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

#include "templateengine_p.h"

#include "codedescription.h"
#include "codedescriptionmetatypes.h"
#include "archivetemplateloader.h"

#include <interfaces/icore.h>
#include <QStandardPaths>
#include <QCoreApplication>

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
#if WITH_SMART_TRIM
    d->engine.setSmartTrimEnabled(true);
#endif

    addTemplateDirectories(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kdevcodegen/templates", QStandardPaths::LocateDirectory));

    Grantlee::registerMetaType<KDevelop::VariableDescription>();
    Grantlee::registerMetaType<KDevelop::FunctionDescription>();
    Grantlee::registerMetaType<KDevelop::InheritanceDescription>();
    Grantlee::registerMetaType<KDevelop::ClassDescription>();

    d->engine.addTemplateLoader(AbstractTemplateLoader::Ptr(ArchiveTemplateLoader::self()));
}

TemplateEngine::~TemplateEngine()
{
}

void TemplateEngine::addTemplateDirectories(const QStringList& directories)
{
    FileSystemTemplateLoader* loader = new FileSystemTemplateLoader;
    loader->setTemplateDirs(directories);
    d->engine.addTemplateLoader(AbstractTemplateLoader::Ptr(loader));
}
