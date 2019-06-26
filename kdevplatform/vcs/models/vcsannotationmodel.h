/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
