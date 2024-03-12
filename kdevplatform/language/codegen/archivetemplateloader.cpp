/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "archivetemplateloader.h"

#include <KArchive>
#include <KTextTemplate/Engine>

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

KTextTemplate::Template ArchiveTemplateLoader::loadByName(const QString& name,
                                                          const KTextTemplate::Engine* engine) const
{
    Q_D(const ArchiveTemplateLoader);

    for (ArchiveTemplateLocation* location : qAsConst(d->locations)) {
        if (location->hasTemplate(name)) {
            return engine->newTemplate(location->templateContents(name), name);
        }
    }

    return KTextTemplate::Template();
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
