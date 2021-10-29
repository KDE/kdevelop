/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H
#define KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H

#include <QWidget>

#include <language/codegen/codedescription.h>

#include "ipagefocus.h"

namespace KDevelop
{

/**
 * Assistant dialog page for declaring data members of a new class
 *
 */
class ClassMembersPage : public QWidget, public IPageFocus
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::VariableDescriptionList members READ members WRITE setMembers)

public:
    explicit ClassMembersPage(QWidget* parent);
    ~ClassMembersPage() override;

    /**
     * @return The list of data members, as entered by the user.
     */
    KDevelop::VariableDescriptionList members() const;
    /**
     * Set the list of data members to @p members.
     */
    void setMembers(const KDevelop::VariableDescriptionList& members);

    void setFocusToFirstEditWidget() override;

private:
    class ClassMembersPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H
