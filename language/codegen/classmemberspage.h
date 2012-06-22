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

#ifndef KDEVELOP_CLASSMEMBERSPAGE_H
#define KDEVELOP_CLASSMEMBERSPAGE_H

#include <QWidget>

#include "../languageexport.h"
#include "codedescription.h"

class QItemSelection;

namespace KDevelop
{

class TemplateClassAssistant;
class TemplateSelectionPage;

class KDEVPLATFORMLANGUAGE_EXPORT ClassMembersPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::VariableDescriptionList members READ members WRITE setMembers)

public:
    explicit ClassMembersPage (QWidget* parent);
    virtual ~ClassMembersPage();

    VariableDescriptionList members() const;
    void setMembers(const VariableDescriptionList& members);

    void moveRowTo(int destination, bool relative);

private:
    int rows();

private Q_SLOTS:
    void moveTop();
    void moveUp();
    void moveDown();
    void moveBottom();
    void currentSelectionChanged(const QItemSelection& current);
    void addItem();
    void removeItem();

private:
    class ClassMembersPagePrivate* const d;
};

}

#endif // KDEVELOP_CLASSMEMBERSPAGE_H
