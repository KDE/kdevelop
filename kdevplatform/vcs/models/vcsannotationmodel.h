/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSANNOTATIONMODEL_H
#define KDEVPLATFORM_VCSANNOTATIONMODEL_H

#include <vcs/vcsexport.h>
#include "../vcsrevision.h"

#include <KTextEditor/AnnotationInterface>

#include <QColor>

#include <memory>

class QUrl;
template<typename T> class QList;

namespace KDevelop
{

class VcsJob;
class VcsAnnotationLine;
class VcsAnnotationModelPrivate;

class KDEVPLATFORMVCS_EXPORT VcsAnnotationModel : public KTextEditor::AnnotationModel,
                                                  public std::enable_shared_from_this<VcsAnnotationModel>
{
Q_OBJECT
public:
    explicit VcsAnnotationModel(const QUrl& url, QObject* parent, const QColor& foreground = Qt::black,
                                const QColor& background = Qt::white);
    ~VcsAnnotationModel() override;

    /**
     * Remove existing annotation lines if any, associate and run a VCS annotation job.
     * Or do nothing if @p job is already associated with this model.
     *
     * @param job a non-null not started VCS annotation job
     */
    void setAnnotationJob(VcsJob* job);

    VcsRevision revisionForLine(int line) const;

    QVariant data( int line, Qt::ItemDataRole role = Qt::DisplayRole ) const override;

    // given "role" argument is of type Qt::ItemDataRole and not int, we cannot use custom roles
    // to access custom data, so providing a custom API instead
    VcsAnnotationLine annotationLine(int line) const;

private:
    const QScopedPointer<class VcsAnnotationModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsAnnotationModel)
    friend class VcsAnnotationModelPrivate;
};

}

#endif
