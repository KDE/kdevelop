/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H

#include <QWidget>
#include <QVariantHash>

#include "ipagefocus.h"


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
class TemplateOptionsPage : public QWidget, public IPageFocus
{
    Q_OBJECT
    Q_PROPERTY(QVariantHash templateOptions READ templateOptions)

public:
    /**
     * @brief Create a new template options page
     *
     * @param parent the parent template class assistant
     **/
    explicit TemplateOptionsPage(QWidget* parent);
    /**
     * Destructor
     **/
    ~TemplateOptionsPage() override;

    /**
     * Parses template archive file and creates the UI for setting template options.
     *
     * @param fileTemplate The template archive file
     * @param renderer A KTextTemplate wrapper used to render all the templates
     **/
    void load(const SourceFileTemplate& fileTemplate, TemplateRenderer* renderer);
    /**
     * @property templateOptions
     * 
     * The user-set options. Keys are the options' names, and values are their values. 
     *
     **/
    QVariantHash templateOptions() const;

    void setFocusToFirstEditWidget() override;

private:
    class TemplateOptionsPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATEOPTIONSPAGE_H
