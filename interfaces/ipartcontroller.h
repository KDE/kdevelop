/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
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
#ifndef KDEVPLATFORM_IPARTCONTROLLER_H
#define KDEVPLATFORM_IPARTCONTROLLER_H

#include <kparts/partmanager.h>

#include "interfacesexport.h"

class KPluginFactory;

namespace KParts
{
class Factory;
}

namespace KTextEditor
{
class Editor;
}

namespace KDevelop {

class ICore;

class KDEVPLATFORMINTERFACES_EXPORT IPartController : public KParts::PartManager 
{
    Q_OBJECT
public:
    IPartController( QWidget* parent );
    static KPluginFactory* findPartFactory( const QString& mimetype,
                                             const QString& parttype,
                                             const QString& preferredName = QString() );
    KParts::Part* createPart( const QString& mimetype, const QString& prefName = QString() );

    /**
     * Returns the global editor instance.
     */
    virtual KTextEditor::Editor* editorPart() const = 0;
};

}

#endif

