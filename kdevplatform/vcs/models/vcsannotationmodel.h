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

class QUrl;
template<typename T> class QList;

namespace KDevelop
{

class VcsJob;
class VcsAnnotationLine;
class VcsAnnotationModelPrivate;

class KDEVPLATFORMVCS_EXPORT VcsAnnotationModel : public KTextEditor::AnnotationModel
{
Q_OBJECT
public:
    VcsAnnotationModel( VcsJob* job, const QUrl&, QObject*,
                        const QColor& foreground = QColor(Qt::black), const QColor& background = QColor(Qt::white) );
    ~VcsAnnotationModel() override;

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
