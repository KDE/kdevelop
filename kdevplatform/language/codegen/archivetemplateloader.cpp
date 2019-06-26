/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include "archivetemplateloader.h"

#include <grantlee/engine.h>

#include <KArchive>

using namespace KDevelop;

class KDevelop::ArchiveTemplateLoaderPrivate
{
public:
    QList<ArchiveTemplateLocation*> locations;
};

ArchiveTemplateLoader* ArchiveTemplateLoader::self()
{
    static auto* loader = new ArchiveTemplateLoader;
    return loader;
}

ArchiveTemplateLoader::ArchiveTemplateLoader()
    : d_ptr(new ArchiveTemplateLoaderPrivate)
{
}

ArchiveTemplateLoader::~ArchiveTemplateLoader() = default;

void ArchiveTemplateLoader::addLocation(ArchiveTemplateLocation* location)
{
    Q_D(ArchiveTemplateLoader);

    d->locations.append(location);
}

void ArchiveTemplateLoader::removeLocation(ArchiveTemplateLocation* location)
{
    Q_D(ArchiveTemplateLoader);

    d->locations.removeOne(location);
}

bool ArchiveTemplateLoader::canLoadTemplate(const QString& name) const
{
    Q_D(const ArchiveTemplateLoader);

    return std::any_of(d->locations.constBegin(), d->locations.constEnd(), [&](ArchiveTemplateLocation* location) {
        return (location->hasTemplate(name));
    });
}

Grantlee::Template ArchiveTemplateLoader::loadByName(const QString& name, const Grantlee::Engine* engine) const
{
    Q_D(const ArchiveTemplateLoader);

    for (ArchiveTemplateLocation* location : qAsConst(d->locations)) {
        if (location->hasTemplate(name)) {
            return engine->newTemplate(location->templateContents(name), name);
        }
    }

    return Grantlee::Template();
}

QPair<QString, QString> ArchiveTemplateLoader::getMediaUri(const QString& fileName) const
{
    Q_UNUSED(fileName);
    return QPair<QString, QString>();
}

ArchiveTemplateLocation::ArchiveTemplateLocation(const KArchiveDirectory* directory)
    : m_directory(directory)
{
    ArchiveTemplateLoader::self()->addLocation(this);
}

ArchiveTemplateLocation::~ArchiveTemplateLocation()
{
    ArchiveTemplateLoader::self()->removeLocation(this);
}

bool ArchiveTemplateLocation::hasTemplate(const QString& name) const
{
    return m_directory->entry(name) && m_directory->entry(name)->isFile();
}

QString ArchiveTemplateLocation::templateContents(const QString& name) const
{
    const auto* file = dynamic_cast<const KArchiveFile*>(m_directory->entry(name));
    Q_ASSERT(file);
    return QString::fromUtf8(file->data());
}
