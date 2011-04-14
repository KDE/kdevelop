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

#ifndef QMAKEMKSPECS_H
#define QMAKEMKSPECS_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include "qmakefile.h"

class QMakeMkSpecs : public QMakeFile
{
public:
    QMakeMkSpecs( const QString& basicmkspec, const QHash<QString, QString>& variables );

    QString qmakeInternalVariable( const QString& ) const;
    bool isQMakeInternalVariable( const QString& ) const;

private:
    QHash<QString, QString> m_qmakeInternalVariables;
};

#endif

