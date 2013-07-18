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

#ifndef KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H
#define KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H

#include <QWidget>

#include <language/codegen/codedescription.h>

/**
 * Assistant dialog page for declaring data members of a new class
 *
 */
class ClassMembersPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::VariableDescriptionList members READ members WRITE setMembers)

public:
    explicit ClassMembersPage(QWidget* parent);
    virtual ~ClassMembersPage();

    /**
     * @return The list of data members, as entered by the user.
     */
    KDevelop::VariableDescriptionList members() const;
    /**
     * Set the list of data members to @p members.
     */
    void setMembers(const KDevelop::VariableDescriptionList& members);

private:
    class ClassMembersPagePrivate* const d;
};

#endif // KDEVPLATFORM_PLUGIN_CLASSMEMBERSPAGE_H
