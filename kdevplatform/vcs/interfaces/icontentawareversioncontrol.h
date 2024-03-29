/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ICONTENTAWAREVERSIONCONTROL_H
#define KDEVPLATFORM_ICONTENTAWAREVERSIONCONTROL_H

#include <KJob>

#include <vcs/vcsexport.h>

namespace KTextEditor {
    class Document;
}

namespace KDevelop {
class CheckInRepositoryJobPrivate;

class KDEVPLATFORMVCS_EXPORT CheckInRepositoryJob : public KJob
{
Q_OBJECT
public:
    explicit CheckInRepositoryJob(KTextEditor::Document* document);
    ~CheckInRepositoryJob() override;

    KTextEditor::Document* document() const;

public Q_SLOTS:
    /// Abort this request.
    void abort();

Q_SIGNALS:
    void finished(bool canRecreate);

private:
    const QScopedPointer<class CheckInRepositoryJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(CheckInRepositoryJob)
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

