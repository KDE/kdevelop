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

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATECLASSASSISTANT_H
#define KDEVPLATFORM_PLUGIN_TEMPLATECLASSASSISTANT_H

#include <KAssistantDialog>
#include <KUrl>

namespace KDevelop
{

/**
 * @brief An assistant for creating new source code files using Grantlee templates.
 *
 * This assistant guides the user through generating source code from templates.
 * Currently, class and unit test templates are supported.
 *
 * Depending on the selected template type (@c Class or @c Test, see SourceFileTemplate::type()),
 * The assistant creates appropriate pages for setting the required options.
 *
 * When creating a new class, an ICreateClassHelper is used for the language-specific features.
 * If no such helper exists for the chosen template, a generic implementation is used.
 * The actual generation of classes is done using a TemplateClassGenerator.
 **/
class TemplateClassAssistant : public KAssistantDialog
{
    Q_OBJECT
public:
    /**
     * Creates a new assistant
     *
     * @param parent parent widget
     * @param baseUrl the directory where the new class should be created
     **/
    TemplateClassAssistant(QWidget* parent, const KUrl& baseUrl = KUrl());
    /**
     * Destroys the assistant
     **/
    virtual ~TemplateClassAssistant();

    /**
     * Sets up the template selection page
     */
    void setup();

    /**
     * @return The url from where the assistant was started.
     *
     * If the assistant was started from the context menu of a project item,
     * this function returns that item's URL. Otherwise, this returns an invalid URL.
     */
    KUrl baseUrl() const;

    /**
     * Called when the user selected a template in the first page of the assistant.
     *
     * This function creates all the other pages, depending on the type of the selected template.
     *
     * @param templateDescription template description file of the selected template
     */
    void templateChosen(const QString& templateDescription);

public Q_SLOTS:
    virtual void next();
    virtual void back();
    virtual void accept();

    /**
     * Sets whether the current page is valid or not.
     * If the page is valid, the "Next" or "Finish" button will be displayed.
     *
     * @param valid true if the user-provided information on the current page is valid, false otherwise
     */
    void setCurrentPageValid(bool valid);

private:
    class TemplateClassAssistantPrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATECLASSASSISTANT_H
