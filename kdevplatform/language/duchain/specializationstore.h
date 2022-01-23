/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_SPECIALIZATIONSTORE_H
#define KDEVPLATFORM_SPECIALIZATIONSTORE_H

#include <QHash>

#include <language/languageexport.h>

namespace KDevelop {
class DeclarationId;
class Declaration;
class DUContext;
class TopDUContext;
class IndexedInstantiationInformation;

/**
 * This class allows dynamic management of "current" specializations for declarations.
 *
 * The specializations will be applied in editors, and wherever it makes sense.
 * For example, this is used in C++ to get code-completion and use-building within
 * instantiated template-classes/functions.
 */
class KDEVPLATFORMLANGUAGE_EXPORT SpecializationStore
{
public:
    static SpecializationStore& self();

    /**
     * Adds/updates the current specialization for the given declaration-id
     * */
    void set(const DeclarationId& declaration, const IndexedInstantiationInformation& specialization);
    /**
     * Gets the registered specialization for the given declaration-id, or zero.
     */
    IndexedInstantiationInformation get(const DeclarationId& declaration);
    /**
     * Clears the specialization registered for the given declaration-id
     */
    void clear(const DeclarationId& declaration);
    /**
     * Clears all registered specializations
     */
    void clear();

    /**
     * Applies the known specializations for the given declaration using the Declaration::specialize() function.
     *
     * If no specializations are known, the original declaration is returned.
     *
     * @param declaration The declaration to specialize
     * @param source The top-context from where to start searching
     * @param recursive Whether parent-contexts should be checked for known specializations, and those applied.
     *                  This is a bit more expensive then just doing a local check.
     */
    KDevelop::Declaration* applySpecialization(KDevelop::Declaration* declaration,
                                               KDevelop::TopDUContext* source, bool recursive = true);
    /**
     * Applies the known specializations for the given context using the DUContext::specialize() function.
     *
     * If no specializations are known, returns the original context.
     *
     * @param context The context to specialize
     * @param source The top-context from where to start searching
     * @param recursive Whether parent-contexts should be checked for known specializations, and those applied.
     *                  This is a bit more expensive then just doing a local check.
     */
    DUContext* applySpecialization(KDevelop::DUContext* context,
                                   KDevelop::TopDUContext* source,
                                   bool recursive = true);

private:
    SpecializationStore();
    ~SpecializationStore();
    QHash<DeclarationId, IndexedInstantiationInformation> m_specializations;
};
}

#endif
