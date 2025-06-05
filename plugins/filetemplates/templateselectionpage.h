/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H

#include <QWidget>

#include <memory>

namespace KDevelop
{
class ITemplateProvider;
class TemplateClassAssistant;

class TemplateSelectionPagePrivate;

/**
 * An assistant page for selecting a class template
 **/
class TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)

public:
    explicit TemplateSelectionPage(ITemplateProvider& templateProvider, TemplateClassAssistant* parent);
    ~TemplateSelectionPage() override;

    /**
     * @property selectedTemplate
     *
     * The class template, selected by the user.
     * This property stores the full path to the template description (.desktop) file
     **/
    QString selectedTemplate() const;

    QSize minimumSizeHint() const override;

public Q_SLOTS:
    /**
     * Saves the selected template setting into the current project's configuration.
     *
     * If the assistant's base URL does not point to any project, this function does nothing.
     */
    void saveConfig();

private:
    const std::unique_ptr<TemplateSelectionPagePrivate> d_ptr;
    Q_DECLARE_PRIVATE(TemplateSelectionPage)
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
