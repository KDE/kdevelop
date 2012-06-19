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

#ifndef KDEV_CREATECLASS_H
#define KDEV_CREATECLASS_H

// Included for source compatibility
#include "classgenerator.h"

#include "language/duchain/identifier.h"
#include "language/duchain/duchainpointer.h"
#include "language/duchain/types/structuretype.h"

#include <KAssistantDialog>
#include <KUrl>

#include "../languageexport.h"

class KLineEdit;
class KTextEdit;

namespace KDevelop {

struct ClassDescription;
class ClassGenerator;

class OverridesPage;
class IndexedDeclaration;
class Context;
class Declaration;
class DocumentChangeSet;

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

Q_SIGNALS:
    void inheritanceChanged();
    void isValid(bool valid);

public Q_SLOTS:
    /// Called when an inheritance is to be added.  To override in subclasses,
    /// (eg. if there is a problem with the base class proposed),
    /// don't call this implementation.
    virtual void addInheritance();

    /**
     * Called when an inheritance is to be removed.
     *
     * To override in subclasses, don't call this implementation.
     */
    virtual void removeInheritance();

    /**
     * Called when an inheritance is to be moved up.
     *
     * To override in subclasses, don't call this implementation.
     */
    virtual void moveUpInheritance();

    /**
     * Called when an inheritance is to be moved up.
     *
     * To override in subclasses, don't call this implementation.
     */
    virtual void moveDownInheritance();

    /**
     * Parses a parent class into a QualifiedIdentifier, the default implementation
     * Just returns the string converted to a QualifiedIdentifier
     */
    virtual QualifiedIdentifier parseParentClassId(const QString& inheritedObject);

private Q_SLOTS:
    void checkMoveButtonState();
    void checkIdentifier();

private:

    class ClassIdentifierPagePrivate* const d;
};

//!@todo Add the name of the Author at the top of the license
class KDEVPLATFORMLANGUAGE_EXPORT LicensePage : public QWidget
{
    Q_OBJECT

public:
    LicensePage(QWidget* parent);
    virtual ~LicensePage();

    KTextEdit* licenseTextEdit();
    bool validatePage();

public Q_SLOTS:
    virtual void licenseComboChanged(int license);

private:
    // data
    class LicensePagePrivate* const d;

    ///FIXME: ugly internal api, move to *Private
    // methods
    void        initializeLicenses();
    QString&    readLicense(int licenseIndex);
    bool        saveLicense();
};

/**
 * Provides an assistant for creating a new class using a ClassGenerator.
 */
class KDEVPLATFORMLANGUAGE_EXPORT CreateClassAssistant : public KAssistantDialog
{
    Q_OBJECT

public:
    CreateClassAssistant(QWidget* parent, ClassGenerator * generator, const KUrl& baseUrl = KUrl());
    CreateClassAssistant(QWidget* parent, const KUrl& baseUrl = KUrl());
    virtual ~CreateClassAssistant();
    /**
     * Creates the generic parts of the new class assistant.
     */
    virtual void setup();

    /**
     * \return The generator that this assistant will use
     */
    virtual ClassGenerator * generator();

    void setGenerator(ClassGenerator* generator);

    virtual ClassIdentifierPage* newIdentifierPage();

    virtual OverridesPage* newOverridesPage();

    /**
     * Returns the base url from which the assistant was called
     **/
    KUrl baseUrl();


public Q_SLOTS:
    /**
     * Called when the next button is clicked.
     */
    virtual void next();
    /**
     * Called when the assistant completes.
     */
    virtual void accept();

private Q_SLOTS:
    void checkClassIdentifierPage(bool valid);
    void checkOutputPage(bool valid);

private:
    bool validateClassIdentifierPage();
    bool validateOverridesPage();
    bool validateLicensePage();
    bool validateOutputPage();

    friend class OutputPage;
    class CreateClassAssistantPrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT OutputPage : public QWidget
{
    Q_OBJECT

public:
    OutputPage(CreateClassAssistant* parent);
    virtual ~OutputPage();

    virtual void initializePage();

    virtual bool validatePage();

    virtual bool isComplete() const;

Q_SIGNALS:
    void isValid(bool valid);

private:
    class OutputPagePrivate* const d;

private Q_SLOTS:
    virtual void updateFileNames();

    /**
     * This implementation simply enables the position widgets on a file that exists.
     * Derived classes should overload to set the ranges where class generation should be allowed
     *
     * @param field the name of the file to be generated (Header, Implementation, etc)
     */
    virtual void updateFileRange (const QString& field);
};
}

#endif // KDEV_CREATECLASS_H
