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

#ifndef KDEV_CLASSGENERATOR_H
#define KDEV_CLASSGENERATOR_H

#include "language/duchain/duchainpointer.h"
#include "language/duchain/types/structuretype.h"

#include "../languageexport.h"

namespace KDevelop {

struct ClassDescription;
class IndexedDeclaration;
class Context;
class Declaration;
class DocumentChangeSet;

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
     * @return A hash of all files for the given class name.
     *
     * The keys are unique IDs and are never shown to the user.
     * The values are user-visible labels.
     *
     * The default implementation returns:
     * @code
     * "header" => i18n("Header"),
     * "implementation" => i18n("Implementation")
     * @endcode
     **/
    virtual QHash<QString, QString> fileLabels();

    /**
     * @return The suggested URLs of all files for the gives class name.
     *
     * The keys should match the keys returned by @c fileLabels()
     *
     * The default implementation calls @c headerUrlFromBase() and
     * @c implementationUrlFromBase().
     *
     * @param baseUrl The base URL, where the files should be located.
     * @param toLowor Whether filenames should be all lowercase, defaults to true.
     **/
    virtual QHash<QString, KUrl> fileUrlsFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     * @return The suggested URL of the header file for the given class name.
     */
    virtual KUrl headerUrlFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     * @return The suggested URL of the implementation file for the given class
     *         name, if header and implementation are separate for this language.
     *         Otherwise, return an invalid URL which is also the default.
     */
    virtual KUrl implementationUrlFromBase(const KUrl& baseUrl, bool toLower = true);

    /**
     * Set the URL where the file of type @p fileType will be implemented.
     **/
    void setFileUrl(const QString& fileType, const KUrl url);

    /**
     * Set the URL where the header will be implemented.
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

    /**
     * \param namespaces The list of nested namespaces in which this class is to be declared
     */
    virtual void setNamespaces(const QStringList& namespaces) const;

    /**
     * \return The list of nested namespace in which this class will be declared
     */
    virtual QStringList namespaces() const;

    /**
     * Get the license specified for this class.
     */
    QString license() const;
    /**
     * Specify license for this class.
     */
    void setLicense(const QString& license);

    virtual void setDescription(const ClassDescription& description);

    virtual ClassDescription description();

  protected:

    /**
     * Set the name (without namespace) for this class
     */
    void setName(const QString&);

    SimpleCursor headerPosition();
    SimpleCursor implementationPosition();

    /**
     * Returns the position in the file where the contents of type @p fileType will be generated
     *
     * @param fileType the id of this file, should correspond to a key in fileLabels()
     **/
    SimpleCursor filePosition(const QString& fileType);

    KUrl headerUrl();
    KUrl implementationUrl();

    /**
     * Returns the URLs where the files will be generated
     **/
    QHash< QString, KUrl > fileUrls();

    /**
     * Look recursively for parent classes, and add them to the Inheritance list
     */
    void fetchParentClasses(const Context * baseClass);

    /**
     * Adds the newly-created class to a project target.
     *
     * A dialog for selecting the target is shown to the user.
     * If it is cancelled, the class is not added to any target,
     * but it is still generated.
     *
     * @param url the folder where the class is generated
     */
    void addToTarget(const KUrl& url);

    QList<DeclarationPointer> m_baseClasses;  //!< These are the base classes, that are directly inherited from
    QList<DeclarationPointer> m_declarations; //!< Declarations

  private:
    struct ClassGeneratorPrivate * const d;

    ///FIXME: move to *Private
    void fetchSuperClasses(const DeclarationPointer& derivedClass);
};

}

#endif // KDEV_CLASSGENERATOR_H
