/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED
#define XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED

#include <KUrl>
#include <QWidget>

class QToolButton;
class QComboBox;
namespace Ui { class VeritasConfig; }

namespace Veritas
{

class ConfigWidget : public QWidget
{
Q_OBJECT
public:
    ConfigWidget(QWidget* parent=0);
    virtual ~ConfigWidget();

    /*! Initializes a framework specific details widget.
     *  Does not take ownership. */
    void setDetailsWidget(QWidget* details);

    /*! Retrieve the test executable URL's the user inserted */
    KUrl::List executables() const;

    /*! Number of test-executable URL input-bars */
    int numberOfTestExecutableFields() const;

    /*! Add a framework to the framework-selection box. */
    void appendFramework(const QString&);

    /*! Returns the framework that is currently selected in the 
     *  framework selection dropwdown box */
    QString currentFramework() const;

    /*! Set the current item in the framework selection dropdown box.
     *  Must be previously added to this box with appendFramework */
    void setCurrentFramework(const QString& name);

public Q_SLOTS:
    /*! Adds a gui element where a single test executable location can be
     *  configured. This widget contains an urlrequester and remove button.
     *  Initial value is set to @p testExecutable. */
    void addTestExecutableField(const KUrl& testExecutable = KUrl());

Q_SIGNALS:
    void frameworkSelected(const QString& name);
    void changed();

private Q_SLOTS:
    /*! Shows or hides the framework specific details widget */
    void expandDetails(bool);

    /*! Removes a test executable input bar from the GUI. The actual field
     *  being removed is accessed through sender(), which is the remove
     *  button right next to the urlrequester. */
    void removeTestExecutableField();

private:
    void setupButtons();
    QToolButton* expandDetailsButton() const;
    QToolButton* addExecutableButton() const;
    QToolButton* removeExecutableButton(int fieldIndex) const;
    QLayout* executableFieldsLayout() const;
    QComboBox* frameworkBox() const;

private:
    Ui::VeritasConfig* m_ui;
    QWidget* m_details; // framework specific details widget
    QList<QToolButton*> m_removeButtons; // used to retrieve exe-url bar location in the widget

private:
    friend class ConfigWidgetTest;

    /*! Fakes a click on the expand details button.
     *  For testers only. */
    void fto_clickExpandDetails() const;

    /*! Fakes a click on the add test executable button.
     *  For testers only. */
    void fto_clickAddTestExeField() const;

    /*! Fakes a click on the remove test executable button.
     *  Since multiple test-exe bars are typically shown, @p fieldIndex says
     *  which one to remove.
     *  For testers only. */
    void fto_clickRemoveTestExeField(int fieldIndex) const;

    /*! Retrieves all the entries in the framework QComboBox
     *  For testers only. */
    QStringList fto_frameworkComboBoxContents() const;
};

}

#endif // XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED
