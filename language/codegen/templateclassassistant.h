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

#include "codedescription.h"

#include <KAssistantDialog>

class QItemSelection;

namespace KDevelop
{

class ClassMembersPage;
class TemplateClassAssistant;
class TemplateSelectionPage;

/**
 * @brief An assistant for creating a new class using Grantlee templates
 * 
 * This assistant extends the generic CreateClassAssistant with functionality, 
 * specific to rendering templates. This includes an extra page for selecting the template
 * to be used. 
 * 
 * For language specific features, an ICreateClassHelper is needed. If no such helper exists for 
 * the chosen template, a generic implementation is used. It uses TemplateClassGenerator for
 * the actual generation of output. 
 * 
 **/
class KDEVPLATFORMLANGUAGE_EXPORT TemplateClassAssistant : public KAssistantDialog
{
    Q_OBJECT
public:
    /**
     * Creates a new assistant
     *
     * @param parent parent widget
     * @param baseUrl the directory where the new class should be created
     **/
    TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl = KUrl());
    /**
     * Destroys the assistant
     **/
    virtual ~TemplateClassAssistant();

    void setup();

    KUrl baseUrl() const;


public Q_SLOTS:
    /**
     * Called when the next button is clicked.
     */
    virtual void next();
    virtual void back();
    /**
     * Called when the assistant completes.
     */
    virtual void accept();

    void setCurrentPageValid(bool valid);

private:
    class TemplateClassAssistantPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATECLASSASSISTANT_H
