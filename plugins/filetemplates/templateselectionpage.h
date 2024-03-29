/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H

#include <QWidget>

namespace KDevelop
{

class TemplateClassAssistant;

/**
 * An assistant page for selecting a class template
 **/
class TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)

public:
    explicit TemplateSelectionPage (TemplateClassAssistant* parent);
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
    class TemplateSelectionPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
