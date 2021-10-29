/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H
#define KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H

#include <QWidget>

#include "ipagefocus.h"

namespace KDevelop
{

/**
 * Assistant dialog page for setting the identifier and inheritances of a new class
 */
class ClassIdentifierPage : public QWidget, public IPageFocus
{
    Q_OBJECT
    Q_PROPERTY(QStringList inheritance READ inheritanceList)

public:
    explicit ClassIdentifierPage(QWidget* parent);
    ~ClassIdentifierPage() override;

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

    void setFocusToFirstEditWidget() override;

Q_SIGNALS:
    void inheritanceChanged();
    /**
     * Emitted whenever the content of the page changes.
     *
     * @param valid @c true if the content is valid and the user can proceed to
     *        the next page, @c false otherwise.
     */
    void isValid(bool valid);

private Q_SLOTS:
    void checkIdentifier();

private:
    struct ClassIdentifierPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_CLASSIDENTIFIERPAGE_H
