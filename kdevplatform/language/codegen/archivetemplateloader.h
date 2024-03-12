/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ARCHIVETEMPLATELOADER_H
#define KDEVPLATFORM_ARCHIVETEMPLATELOADER_H

#include <KTextTemplate/TemplateLoader>

class KArchiveDirectory;
namespace KDevelop {
class ArchiveTemplateLocation;
class ArchiveTemplateLoaderPrivate;

class ArchiveTemplateLoader : public KTextTemplate::AbstractTemplateLoader
{
public:
    static ArchiveTemplateLoader* self();
    ~ArchiveTemplateLoader() override;
    bool canLoadTemplate(const QString& name) const override;
    KTextTemplate::Template loadByName(const QString& name, const KTextTemplate::Engine* engine) const override;

    QPair<QString, QString> getMediaUri(const QString& fileName) const override;

protected:
    friend class ArchiveTemplateLocation;
    void addLocation(ArchiveTemplateLocation* location);
    void removeLocation(ArchiveTemplateLocation* location);

private:
    Q_DISABLE_COPY(ArchiveTemplateLoader)
    ArchiveTemplateLoader();

    const QScopedPointer<class ArchiveTemplateLoaderPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ArchiveTemplateLoader)
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
    explicit ArchiveTemplateLocation(const KArchiveDirectory* directory);
    ~ArchiveTemplateLocation();

    bool hasTemplate(const QString& name) const;
    QString templateContents(const QString& name) const;

private:
    Q_DISABLE_COPY(ArchiveTemplateLocation)

    const KArchiveDirectory* m_directory;
};
}

#endif // KDEVPLATFORM_ARCHIVETEMPLATELOADER_H
