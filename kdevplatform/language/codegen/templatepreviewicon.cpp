/*  This file is part of KDevelop
    Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "templatepreviewicon.h"

#include <debug.h>

#include <KTar>
#include <KZip>

#include <QSharedData>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QStandardPaths>

using namespace KDevelop;

class KDevelop::TemplatePreviewIconData
    : public QSharedData
{
public:
    QString iconName;
    QString archivePath;
    QString dataDir;
};

TemplatePreviewIcon::TemplatePreviewIcon(const QString& iconName, const QString& archivePath, const QString& dataDir)
    : d(new TemplatePreviewIconData)
{
    d->iconName = iconName;
    d->archivePath = archivePath;
    d->dataDir = dataDir;
}

TemplatePreviewIcon::TemplatePreviewIcon()
    : d(new TemplatePreviewIconData)
{
}

TemplatePreviewIcon::TemplatePreviewIcon(const TemplatePreviewIcon& other)
    : d(other.d)
{
}

TemplatePreviewIcon::~TemplatePreviewIcon() = default;

TemplatePreviewIcon& TemplatePreviewIcon::operator=(const TemplatePreviewIcon& other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QPixmap TemplatePreviewIcon::pixmap() const
{
    if (!d->iconName.isEmpty()) {
        // read icon from archive
        QScopedPointer<KArchive> templateArchive;
        if (QFileInfo(d->archivePath).completeSuffix() == QLatin1String("zip")) {
            templateArchive.reset(new KZip(d->archivePath));
        } else {
            templateArchive.reset(new KTar(d->archivePath));
        }

        if (templateArchive->open(QIODevice::ReadOnly)) {
            const KArchiveFile* iconFile = templateArchive->directory()->file(d->iconName);
            if (iconFile) {
                QPixmap pixmap;
                const bool loadSuccess = pixmap.loadFromData(iconFile->data());
                if (loadSuccess) {
                    return pixmap;
                }
                qCWarning(LANGUAGE) << "Could not load preview icon" << d->iconName << "from" << d->archivePath;
            }
        }

        // support legacy templates with image files installed separately in the filesystem
        const QString iconFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                            d->dataDir + d->iconName);
        if (!iconFilePath.isEmpty()) {
            QPixmap pixmap(iconFilePath);
            if (!pixmap.isNull()) {
                return pixmap;
            }
            qCWarning(LANGUAGE) << "Could not load preview icon" << iconFilePath << "as wanted for" << d->archivePath;
        }
    }

    // try theme icon or default to a kdevelop icon
    // QIcon::hasThemeIcon for empty string can yield true with some engines, not wanted here
    const bool isThemeIcon = (!d->iconName.isEmpty() && QIcon::hasThemeIcon(d->iconName));
    const QString iconName = isThemeIcon ? d->iconName : QStringLiteral("kdevelop");

    const QIcon icon = QIcon::fromTheme(iconName);
    return icon.pixmap(128, 128);
}
