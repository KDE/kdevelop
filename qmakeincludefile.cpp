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

QMakeIncludeFile::QMakeIncludeFile( const QString& incfile, QMakeFile* parent,
                                    const VariableMap& variables )
    : QMakeProjectFile( incfile )
    , m_parent(parent)
{
    m_variableValues = variables;

    QMakeProjectFile* pro = dynamic_cast<QMakeProjectFile*>(parent);
    if( pro ) {
        setMkSpecs( pro->mkSpecs() );
        setQMakeCache( pro->qmakeCache() );
        setProject( pro->project() );
    } else {
        QMakeMkSpecs* specs = dynamic_cast<QMakeMkSpecs*>(parent);
        setMkSpecs( specs );
    }
}

QString QMakeIncludeFile::pwd() const
{
    return absoluteDir();
}

QString QMakeIncludeFile::outPwd() const
{
    QMakeProjectFile* pro = dynamic_cast<QMakeProjectFile*>(m_parent);
    if (pro) {
        return pro->outPwd();
    } else {
        return absoluteDir();
    }
}

QString QMakeIncludeFile::proFile() const
{
    return m_parent->absoluteFile();
}

QString QMakeIncludeFile::proFilePwd() const
{
    return m_parent->absoluteDir();
}
