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

#ifndef KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H
#define KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H

#include <QWidget>

/**
 * Assistant dialog page for setting the identifier and inheritances of a new class
 */
class ClassIdentifierPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList inheritance READ inheritanceList)

public:
    ClassIdentifierPage(QWidget* parent);
    virtual ~ClassIdentifierPage();

    /**
     * @return The full identifier of the new class, with namespaces,
     *         as entered by the user.
     */
    QString identifier() const;

    /**
     * @return A list of inheritances for the new class.
     *
     * Each list elements contains both inheritance type and the base class name,
     * such as "public QObject" or "implements Serializable"
     */
    QStringList inheritanceList() const;

    /**
     * Sets the contents of the inheritance list to @p list
     *
     * @param list list of inheritance descriptions. 
     */
    void setInheritanceList(const QStringList& list);

Q_SIGNALS:
    void inheritanceChanged();
    /**
     * Emitted whenever the content of the page changes.
     *
     * @param valid @c true if the content is valid and the user can procede to
     *        the next page, @c false otherwise.
     */
    void isValid(bool valid);

private Q_SLOTS:
    void checkIdentifier();

private:
    struct ClassIdentifierPagePrivate* const d;
};

#endif // KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H
