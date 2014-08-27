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

#ifndef KDEVPLATFORM_IDOCUMENTATION_H
#define KDEVPLATFORM_IDOCUMENTATION_H

#include <QtCore/QString>
#include <QExplicitlySharedDataPointer>
#include "interfacesexport.h"
#include <util/ksharedobject.h>

class QWidget;

namespace KDevelop
{

class DocumentationFindWidget;
class IDocumentationProvider;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentation : public QObject, public KSharedObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(IDocumentationProvider* provider READ provider CONSTANT)
    public:
        typedef QExplicitlySharedDataPointer<IDocumentation> Ptr;
        IDocumentation();

        virtual ~IDocumentation();
        
        /** @returns the name of the documented information*/
        virtual QString name() const = 0;
        
        /** @returns a HTML-formatted short description. */
        virtual QString description() const = 0;
        
        /** @returns a widget with all the needed documentation information.
            @param parent defines the widget's parent
            @param findWidget can be used to tell how do we want to deal with Search
                inside the documentation widget. The implementation will have to enable the
                widget if it means to support the search feature.
        */
        virtual QWidget* documentationWidget(DocumentationFindWidget* findWidget, QWidget* parent=0) = 0;
        
        virtual IDocumentationProvider* provider() const = 0;

    Q_SIGNALS:
        void descriptionChanged();
};

}
#endif
