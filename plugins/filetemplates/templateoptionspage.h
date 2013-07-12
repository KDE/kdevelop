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

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H

#include <QWidget>
#include <QVariantHash>



namespace KDevelop
{

class TemplateRenderer;

class SourceFileTemplate;

class TemplateClassAssistant;

/**
 * @brief Shows a page for configuring options specified by a class template
 * 
 * Templates can include a file that specify configuration options. 
 * These can be set by the user before creating the class and are passed to the template.
 * 
 * @sa SourceFileTemplate::customOptions()
 **/
class TemplateOptionsPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantHash templateOptions READ templateOptions)

public:
    /**
     * @brief Create a new template options page
     *
     * @param parent the parent template class assistant
     * @param f window flags, passed to QWidget
     **/
    explicit TemplateOptionsPage(QWidget* parent, Qt::WindowFlags f = 0);
    /**
     * Destructor
     **/
    virtual ~TemplateOptionsPage();

    /**
     * Parses the XML-formatted .kcfg file contents and creates the UI for setting template options. 
     *
     * @param contents the file contents
     **/
    void load(const SourceFileTemplate& fileTemplate, TemplateRenderer* renderer);
    /**
     * @property templateOptions
     * 
     * The user-set options. Keys are the options' names, and values are their values. 
     *
     **/
    QVariantHash templateOptions() const;

private:
    class TemplateOptionsPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H
