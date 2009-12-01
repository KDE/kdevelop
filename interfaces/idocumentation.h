/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef IDOCUMENTATION_H
#define IDOCUMENTATION_H

#include <QtCore/QString>
#include <KDE/KSharedPtr>
#include "interfacesexport.h"

class QWidget;

namespace KDevelop
{
class IDocumentationProvider;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentation : public KShared
{
    public:
        typedef KSharedPtr<IDocumentation> Ptr;
        virtual ~IDocumentation();
        
        /** @returns the name of the documented information*/
        virtual QString name() const = 0;
        
        /** @returns a HTML-formatted short description. */
        virtual QString description() const = 0;
        
        /** @returns a widget with all the needed documentation information.
            \param parent defines the widget's parent
        */
        virtual QWidget* documentationWidget(QWidget* parent=0);
        
        virtual IDocumentationProvider* provider() const = 0;
};

}
#endif
