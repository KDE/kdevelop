/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "qmakeincludefile.h"
#include "qmakemkspecs.h"
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

QMakeIncludeFile::QMakeIncludeFile( const QString& incfile,
                                    const QMap<QString,QStringList>& variables  )
    : QMakeProjectFile( incfile )
{
    foreach( QString variable, variables.keys() )
    {
        m_variableValues[variable] = variables[variable];
    }
}

void QMakeIncludeFile::setParent( QMakeFile* file )
{
    QMakeProjectFile* parent = dynamic_cast<QMakeProjectFile*>(file);
    if( parent )
    {
        setMkSpecs( parent->mkSpecs() );
        setQMakeCache( parent->qmakeCache() );
    }else
    {
        QMakeMkSpecs* specs = dynamic_cast<QMakeMkSpecs*>(file);
        setMkSpecs( specs );
    }
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
