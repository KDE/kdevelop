/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <pakulat@rostock.zgdv.de>                     *
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

#include "qmakemkspecs.h"

#include <QStringList>
#include <QDebug>

QMakeMkSpecs::QMakeMkSpecs( const QString& basicmkspecs, const QHash<QString,QString>& variables )
    : QMakeFile( basicmkspecs ), m_qmakeInternalVariables( variables )
{
}

QString QMakeMkSpecs::qmakeInternalVariable( const QString& var ) const
{
    return m_qmakeInternalVariables.value(var, QString());
}

bool QMakeMkSpecs::isQMakeInternalVariable(const QString& var ) const
{
    return m_qmakeInternalVariables.contains(var);
}
