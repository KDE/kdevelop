/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
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

#ifndef SCRIPTTOOLS_H
#define SCRIPTTOOLS_H

#include <QtCore/qobject.h>
#include "idocument.h"
#include "idocumentcontroller.h"

Q_DECLARE_METATYPE(KDevelop::IDocument*)
Q_DECLARE_METATYPE(KDevelop::IDocumentController*)

namespace KDevelop
{

/*
   Kross doesn't allow automatic meta-type conversions at the moment,
   so we need this helper to "cast" between IDocument* and IDocumentController*
   and the corresponding scripting classes.
 */
class ScriptTools : public QObject
{
    Q_OBJECT
public:
    ScriptTools();

    // implemented in scriptdocument.cpp
    Q_SCRIPTABLE QObject *toDocument(const QVariant &variant);

    Q_SCRIPTABLE QObject *toDocumentController(const QVariant &variant);
};

}
#endif

