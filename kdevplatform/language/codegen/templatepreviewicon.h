/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEPREVIEWICON_H
#define KDEVPLATFORM_TEMPLATEPREVIEWICON_H

#include <QSharedDataPointer>
#include <QMetaType>

#include <language/languageexport.h>

class QPixmap;

namespace KDevelop {
class TemplatePreviewIconData;

/**
 * @brief A class for loading preview images/icons as specified in a template description on demand
 *
 * It stores the icon name as extracted from the template description together with
 * the path to the archive. Only on demand will the icon file tried to be loaded and
 * returned as pixmap object.
 * To support also legacy template installations with the preview image installed as separate file,
 * the local directory prefix in the QStandardPaths::GenericDataLocation is stored as well
 * to search also those paths as fallback.
 *
 * If the icon name is empty, no matching icon file could be found or no image successfully loaded,
 * the returned pixmap will default to a themed "kdevelop" icon image (128x128).
 **/
class KDEVPLATFORMLANGUAGE_EXPORT TemplatePreviewIcon
{
public:
    /**
     * Creates a new template preview icon object
     *
     * @param iconName the raw icon name as specified in the template description
     * @param archivePath absolute path to the template archive the icon is used by
     * @param dataDir local directory prefix of the icon file if searching it through QStandardPaths::GenericDataLocation
     **/
    TemplatePreviewIcon(const QString& iconName, const QString& archivePath, const QString& dataDir);
    TemplatePreviewIcon();
    TemplatePreviewIcon(const TemplatePreviewIcon& other);
    ~TemplatePreviewIcon();

    TemplatePreviewIcon& operator=(const TemplatePreviewIcon& other);

public:
    /**
     * Generates a pixmap to be used as preview for a template.
     * Either from the preview image file as specified, or falling back
     * to a themed "kdevelop" icon image (128x128).
     * The pixmap is not cached, .
     *
     * @return pixmap to be used as preview for a template
     **/
    QPixmap pixmap() const;

private:
    QSharedDataPointer<TemplatePreviewIconData> d;
};
}

Q_DECLARE_METATYPE(KDevelop::TemplatePreviewIcon)

#endif // KDEVPLATFORM_TEMPLATEPREVIEWICON_H
