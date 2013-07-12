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

#ifndef KDEVPLATFORM_ARCHIVETEMPLATELOADER_H
#define KDEVPLATFORM_ARCHIVETEMPLATELOADER_H

#include <grantlee/templateloader.h>

class KArchiveDirectory;
namespace KDevelop
{

class ArchiveTemplateLocation;

class ArchiveTemplateLoader : public Grantlee::AbstractTemplateLoader
{
public:
    static ArchiveTemplateLoader* self();
    virtual ~ArchiveTemplateLoader();
    virtual bool canLoadTemplate(const QString& name) const;
    virtual Grantlee::Template loadByName(const QString& name, const Grantlee::Engine* engine) const;

    virtual QPair<QString, QString> getMediaUri(const QString& fileName) const;

protected:
    friend class ArchiveTemplateLocation;
    void addLocation(ArchiveTemplateLocation* location);
    void removeLocation(ArchiveTemplateLocation* location);

private:
    Q_DISABLE_COPY(ArchiveTemplateLoader);
    ArchiveTemplateLoader();

    class ArchiveTemplateLoaderPrivate* const d;
};

/**
 * RAII class that should be used to add KArchiveDirectory locations to the engine.
 *
 * Adds the archive @p directory to the list of places searched for templates
 * during the lifetime of the created ArchiveTemplateLocation class.
 */
class ArchiveTemplateLocation
{
public:
    ArchiveTemplateLocation(const KArchiveDirectory* directory);
    ~ArchiveTemplateLocation();

    bool hasTemplate(const QString& name) const;
    QString templateContents(const QString& name) const;

private:
    const KArchiveDirectory* m_directory;
};

}

#endif // KDEVPLATFORM_ARCHIVETEMPLATELOADER_H
