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

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H

#include <QWidget>


class QModelIndex;

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
    explicit TemplateSelectionPage (TemplateClassAssistant* parent, Qt::WindowFlags f = 0);
    virtual ~TemplateSelectionPage();

    /**
     * @property selectedTemplate
     *
     * The class template, selected by the user.
     * This property stores the full path to the template description (.desktop) file
     **/
    QString selectedTemplate() const;

    virtual QSize minimumSizeHint() const override;

public Q_SLOTS:
    /**
     * Saves the selected template setting into the current project's configuration.
     *
     * If the assistant's base URL does not point to any project, this function does nothing.
     */
    void saveConfig();

private:
    class TemplateSelectionPagePrivate* const d;

    Q_PRIVATE_SLOT(d, void currentTemplateChanged(const QModelIndex& index))
    Q_PRIVATE_SLOT(d, void getMoreClicked())
    Q_PRIVATE_SLOT(d, void loadFileClicked())
};

}

#endif // KDEVPLATFORM_PLUGIN_TEMPLATESELECTIONPAGE_H
