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

#include <ktexteditor/annotationinterface.h>
#include <vcs/vcsexport.h>
#include "../vcsrevision.h"

#include <QColor>
#include <QUrl>

class QModelIndex;
template<typename T> class QList;

namespace KDevelop
{

class VcsAnnotation;
class VcsAnnotationLine;
class VcsJob;
    
class KDEVPLATFORMVCS_EXPORT VcsAnnotationModel : public KTextEditor::AnnotationModel
{
Q_OBJECT
public:
    VcsAnnotationModel( VcsJob* job, const QUrl&, QObject*,
                        const QColor& foreground = QColor(Qt::black), const QColor& background = QColor(Qt::white) );
    ~VcsAnnotationModel() override;

    VcsRevision revisionForLine(int line) const;

    QVariant data( int line, Qt::ItemDataRole role = Qt::DisplayRole ) const override;

private:
    class VcsAnnotationModelPrivate* const d;
    friend class VcsAnnotationModelPrivate;
};

}

#endif
