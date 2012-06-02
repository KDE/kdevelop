/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#ifndef KDEVELOP_TEMPLATECLASSASSISTANT_H
#define KDEVELOP_TEMPLATECLASSASSISTANT_H

#include <codegen/createclass.h>

namespace KDevelop
{
    
    class TemplateClassAssistant;
    
class KDEVPLATFORMLANGUAGE_EXPORT TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)
    
public:
    explicit TemplateSelectionPage (TemplateClassAssistant* parent, Qt::WindowFlags f = 0);
    virtual ~TemplateSelectionPage();
    
    QString selectedTemplate() const;
    
private:
    class TemplateSelectionPagePrivate* const d;
    
public slots:
    void currentTemplateChanged (QModelIndex index);
};

class KDEVPLATFORMLANGUAGE_EXPORT TemplateClassAssistant : public CreateClassAssistant
{
    Q_OBJECT
public:
    TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl = KUrl());
    virtual ~TemplateClassAssistant();
    
    virtual void setup();
    
    virtual TemplateSelectionPage* newTemplateSelectionPage();
    
private:
    class TemplateClassAssistantPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATECLASSASSISTANT_H
