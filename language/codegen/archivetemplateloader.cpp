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
#include <KZip>
#include <KComponentData>
#include <KStandardDirs>
#include <KTar>
#include <KDebug>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::ArchiveTemplateLoaderPrivate
{
public:
    const KArchiveDirectory* directory;
};

ArchiveTemplateLoader::ArchiveTemplateLoader (const KArchiveDirectory* directory) : d(new ArchiveTemplateLoaderPrivate)
{
    d->directory = directory;
}

ArchiveTemplateLoader::~ArchiveTemplateLoader()
{

}

bool ArchiveTemplateLoader::canLoadTemplate (const QString& name) const
{
    bool can = d->directory->entry(name) && d->directory->entry(name)->isFile();
    kDebug() << "Can load" << name << "?" << can;
    return can;
}

Grantlee::Template ArchiveTemplateLoader::loadByName (const QString& name, const Grantlee::Engine* engine) const
{
    const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(d->directory->entry(name));
    Q_ASSERT(file);
    
    kDebug() << "Loading file" << name;
    return engine->newTemplate(file->data(), file->name());
}

QPair< QString, QString > ArchiveTemplateLoader::getMediaUri (const QString& fileName) const
{
    Q_UNUSED(fileName);
    return QPair<QString, QString>();
}
