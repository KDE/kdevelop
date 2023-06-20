/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icontentawareversioncontrol.h"


namespace KDevelop {

class CheckInRepositoryJobPrivate
{
    explicit CheckInRepositoryJobPrivate(KTextEditor::Document* document)
        : document(document) { };
    friend class CheckInRepositoryJob;
    KTextEditor::Document* document;
};

CheckInRepositoryJob::CheckInRepositoryJob(KTextEditor::Document* document)
    : KJob()
    , d_ptr(new CheckInRepositoryJobPrivate(document))
{
    connect(this, &CheckInRepositoryJob::finished, this, &CheckInRepositoryJob::deleteLater);
    setCapabilities(Killable);
}

CheckInRepositoryJob::~CheckInRepositoryJob() = default;

KTextEditor::Document* CheckInRepositoryJob::document() const
{
    Q_D(const CheckInRepositoryJob);

    return d->document;
}

void CheckInRepositoryJob::abort()
{
    kill();
}

} // namespace KDevelop

#include "moc_icontentawareversioncontrol.cpp"
