/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEV_CLASSIDENTIFIERPAGE_H
#define KDEV_CLASSIDENTIFIERPAGE_H

#include <QWidget>
#include "../languageexport.h"

#include "../duchain/identifier.h"

class KLineEdit;

namespace KDevelop {

class KDEVPLATFORMLANGUAGE_EXPORT ClassIdentifierPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList inheritance READ inheritanceList)

public:
    ClassIdentifierPage(QWidget* parent);
    virtual ~ClassIdentifierPage();

    /// Returns the line edit which contains the new class identifier.
    KLineEdit* identifierLineEdit() const;

    /// Returns the line edit which contains a base class identifier.
    KLineEdit* inheritanceLineEdit() const;

    /// Returns a list of inheritances for the new class
    QStringList inheritanceList() const;

    /**
     * Parses a parent class into a QualifiedIdentifier, the default implementation
     * Just returns the string converted to a QualifiedIdentifier
     */
    virtual QualifiedIdentifier parseParentClassId(const QString& inheritedObject);

Q_SIGNALS:
    void inheritanceChanged();
    void isValid(bool valid);

private Q_SLOTS:
    void checkIdentifier();

private:
    class ClassIdentifierPagePrivate* const d;
};

}

#endif // KDEV_CLASSIDENTIFIERPAGE_H
