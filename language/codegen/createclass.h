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

#include "language/duchain/identifier.h"
#include "language/duchain/duchainpointer.h"

#include <KDE/KAssistantDialog>
#include <KDE/KTextEdit>
#include <KDE/KUrl>

#include "../languageexport.h"
#include <language/duchain/types/structuretype.h>

class KLineEdit;
class KUrl;

namespace KDevelop {

class OverridesPage;
class IndexedDeclaration;
class Context;
class Declaration;
class DocumentChangeSet;

class KDEVPLATFORMLANGUAGE_EXPORT ClassIdentifierPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList inheritance READ inheritanceList())

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
    
    // methods
    void        initializeLicenses();
    QString &   readLicense(int licenseIndex);
    bool        saveLicense();
};

/**
 * A Class generator defines the logic to create a language-specific class
 * and optionally declare some members
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassGenerator
{
  public:
    
    ClassGenerator();
    virtual ~ClassGenerator();
      
    /**
     * Generate the actual DocumentChangeSet
     */
    virtual DocumentChangeSet generate() = 0;
    
    /**
     * Remove all previous base classes
     */
    virtual void clearInheritance();
    
    /**
     * Clear all in class declarations
     */
    void clearDeclarations();
    
    /**
     * Add another base class, must be the pure identifier
     * 
     * \return the current list of base classes
     */
    virtual const QList<DeclarationPointer> & addBaseClass(const QString &);
    
    /**
     * Add a declaration to insert to the new Class
     */
    void addDeclaration(DeclarationPointer newDeclaration);
    
    /**
     * @return All the current declarations for this class
     */
    const QList<DeclarationPointer> declarations() const;
    
    /// \return The list of all of the inherited classes
    const QList<DeclarationPointer> & inheritanceList() const;

    /// \return The list of directly inherited classes
    const QList<DeclarationPointer> & directInheritanceList() const;

    /**
     *Should return the suggested url of the header file for the given class-name
     */
    virtual KUrl headerUrlFromBase(KUrl baseUrl, bool toLower=true);

    /**
     *Should return the suggested url of the implementation file for the given class-name,
     *if header and implementation are separate for this language.
     */
    virtual KUrl implementationUrlFromBase(KUrl baseUrl, bool toLower=true);
    
    /**
     * Set the URL where the header will be implemented
     */
    void setHeaderUrl(KUrl header);
    
    /**
     * Set the URL where the implementation will be implemented
     */
    void setImplementationUrl(KUrl implementation);
    
    /**
     * Set the position where the header is to be inserted
     */
    void setHeaderPosition(SimpleCursor position);
    
    /**
     * Set the position where the implementation stubbs are to be inserted
     */
    void setImplementationPosition(SimpleCursor position);
    
    /**
     * \return The name of the class to generate (excluding namespaces)
     */
    const QString & name() const;
    
    /**
     * \param identifier The Qualified identifier that the class will have
     */
    virtual void setIdentifier(const QString & identifier);
    
    /**
     * \return The Identifier of the class to generate (including all used namespaces)
     */
    virtual QString identifier() const;
    
    const QString & license() const;
    void setLicense(const QString & license);

    /**
     * \return The class to be generated as a Type
     */
    virtual StructureType::Ptr objectType() const = 0;

  protected:
    
    /**
     * Set the name (without namespace) for this class
     */
    void setName(const QString &);
    
    SimpleCursor headerPosition();
    SimpleCursor implementationPosition();
    
    KUrl & headerUrl();
    KUrl & implementationUrl();
    
    /**
     * Look recursively for parent classes, and add them to the Inheritance list
     */
    void fetchParentClasses(const Context * baseClass);
    
    QList<DeclarationPointer> m_baseClasses;  //!< These are the base classes, that are directly inherited from
    QList<DeclarationPointer> m_declarations; //!< Declarations 
    
  private:
    struct ClassGeneratorPrivate * const d;
    
    void fetchSuperClasses(DeclarationPointer derivedClass);
};

/**
 * Provides an assistant for creating a new class using a ClassGenerator.
 */
class KDEVPLATFORMLANGUAGE_EXPORT CreateClassAssistant : public KAssistantDialog
{
    Q_OBJECT

public:
    CreateClassAssistant(QWidget* parent, ClassGenerator * generator, KUrl baseUrl = KUrl());
    virtual ~CreateClassAssistant();
    /**
     * Creates the generic parts of the new class assistant.
     */
    virtual void setup();
    
    /**
     * \return The generator that this assistant will use
     */
    virtual ClassGenerator * generator();

    virtual ClassIdentifierPage* newIdentifierPage();

    virtual OverridesPage* newOverridesPage();

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
     */
    virtual void updateHeaderRanges(const QString &);
    
    /**
     * This implementation simply enables the position widgets on a file that exists.
     * Derived classes should overload to set the ranges where class generation should be allowed
     */
    virtual void updateImplementationRanges(const QString &);
};
}

#endif // KDEV_CREATECLASS_H
