/* This file is part of KDevelop
 *
 * Copyright 2013 Sven Brauch <svenbrauch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_ICONTENTAWAREVERSIONCONTROL_H
#define KDEVPLATFORM_ICONTENTAWAREVERSIONCONTROL_H

#include <KUrl>
#include <KJob>
#include <QSharedPointer>

#include "vcsexport.h"

namespace KTextEditor {
    class Document;
}

namespace KDevelop {

class KDEVPLATFORMVCS_EXPORT CheckInRepositoryJob : public KJob
{
Q_OBJECT
public:
    CheckInRepositoryJob(KTextEditor::Document* document);
    virtual ~CheckInRepositoryJob();

    KTextEditor::Document* document() const;

public slots:
    /// Abort this request.
    void abort();

signals:
    void finished(bool canRecreate);

protected:
    struct CheckInRepositoryJobPrivate* d;
};

/**
 * This interface is used by version control systems which can tell whether a given
 * blob of data is stored in the repository or not, such as git.
 * This information is used to reload files automatically if that involves no data loss.
 */
class IContentAwareVersionControl
{
public:
    virtual ~IContentAwareVersionControl() {};

    /**
     * @brief Determines whether the given data is stored in the VCS' repository.
     *
     * @param document Document to search for in the repository
     * @returns CheckInRepositoryJob request object to track get notified when this finishes.
     * The request object deletes itself after finished() was emitted.
     */
    virtual CheckInRepositoryJob* isInRepository(KTextEditor::Document* document) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IContentAwareVersionControl, "org.kdevelop.IContentAwareVersionControl" )

#endif

