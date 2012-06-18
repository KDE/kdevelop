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
#include "language/duchain/types/structuretype.h"

#include <KAssistantDialog>
#include <KUrl>

#include "../languageexport.h"

class KLineEdit;
class KTextEdit;

namespace KDevelop {

struct ClassDescription;

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
    virtual QList<DeclarationPointer> addBaseClass(const QString&);

    /**
     * Add a declaration to insert to the new Class
     */
    void addDeclaration(const DeclarationPointer& newDeclaration);

    /**
     * @return All the current declarations for this class
     */
    QList<DeclarationPointer> declarations() const;

    /// \return The list of all of the inherited classes
    QList<DeclarationPointer> inheritanceList() const;

    /// \return The list of directly inherited classes
    QList<DeclarationPointer> directInheritanceList() const;

    /**
     * Should return user-readable labels for all files for the gives class-name
     *
     * The default implementation returns "Header" and "Implementation"
     **/
    virtual QStringList fileLabels();

    /**
     * Should return the suggested urls of all files for the gives class-name
     * The keys should match labels returned by fileLabels()
     *
     * The default implementation calls headerUrlFromBase() and implementationUrlFromBase()
     *
     * @param baseUrl The base url, where the files should be located
     * @param toLowor Whether filenames should be all lowercase, defaults to true.
     **/
    virtual QHash<QString, KUrl> fileUrlsFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     *Should return the suggested url of the header file for the given class-name
     */
    virtual KUrl headerUrlFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     *Should return the suggested url of the implementation file for the given class-name,
     *if header and implementation are separate for this language.
     */
    virtual KUrl implementationUrlFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     * Set the URL where the file of type @p fileType will be implemented
     **/
    void setFileUrl(const QString& fileType, const KUrl url);

    /**
     * Set the URL where the header will be implemented
     */
    void setHeaderUrl(const KUrl& header);

    /**
     * Set the URL where the implementation will be implemented
     */
    void setImplementationUrl(const KUrl& implementation);

    /**
     * Set the position where the contents of @p fileType are to be inserted
     **/
    void setFilePosition(const QString& fileType, const SimpleCursor& position);

    /**
     * Set the position where the header is to be inserted
     */
    void setHeaderPosition(const SimpleCursor& position);

    /**
     * Set the position where the implementation stubbs are to be inserted
     */
    void setImplementationPosition(const SimpleCursor& position);

    /**
     * \return The name of the class to generate (excluding namespaces)
     */
    QString name() const;

    /**
     * \param identifier The Qualified identifier that the class will have
     */
    virtual void setIdentifier(const QString& identifier);

    /**
     * \return The Identifier of the class to generate (including all used namespaces)
     */
    virtual QString identifier() const;

    QString license() const;
    void setLicense(const QString& license);

    /**
     * \return The class to be generated as a Type
     */
    virtual StructureType::Ptr objectType() const = 0;

    virtual void setDescription(const ClassDescription& description);

    virtual ClassDescription description();

  protected:

    /**
     * Set the name (without namespace) for this class
     */
    void setName(const QString&);

    SimpleCursor headerPosition();
    SimpleCursor implementationPosition();

    KUrl headerUrl();
    KUrl implementationUrl();

    /**
     * Look recursively for parent classes, and add them to the Inheritance list
     */
    void fetchParentClasses(const Context * baseClass);

    QList<DeclarationPointer> m_baseClasses;  //!< These are the base classes, that are directly inherited from
    QList<DeclarationPointer> m_declarations; //!< Declarations

  private:
    struct ClassGeneratorPrivate * const d;

    ///FIXME: move to *Private
    void fetchSuperClasses(const DeclarationPointer& derivedClass);
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
