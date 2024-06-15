/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DELAYEDTYPE_H
#define KDEVPLATFORM_DELAYEDTYPE_H

#include "abstracttype.h"
#include "../identifier.h"

namespace KDevelop {
class DelayedTypeData;

/**
 * \short A type which has not yet been resolved.
 *
 * Delayed types can be used for any types that cannot be resolved in the moment they are encountered.
 * They can be used for example in template-classes, or to store the names of unresolved types.
 * In a template-class, many types can not be evaluated at the time they are used, because they depend on unknown template-parameters.
 * Delayed types store the way the type would be searched, and can be used to find the type once the template-paremeters have values.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT DelayedType
    : public KDevelop::AbstractType
{
public:
    using Ptr = TypePtr<DelayedType>;

    /// An enumeration of
    enum Kind : quint8 {
        Delayed /**< The type should be resolved later. This is the default. */,
        Unresolved /**< The type could not be resolved */
    };

    /// Default constructor
    DelayedType();
    /// Copy constructor. \param rhs type to copy
    DelayedType(const DelayedType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit DelayedType(DelayedTypeData& data);
    /// Destructor
    ~DelayedType() override;

    DelayedType& operator=(const DelayedType& rhs) = delete;

    /**
     * Access the type identifier which this type represents.
     *
     * \returns the type identifier.
     */
    KDevelop::IndexedTypeIdentifier identifier() const;

    /**
     * Set the type identifier which this type represents.
     *
     * \param identifier the type identifier.
     */
    void setIdentifier(const KDevelop::IndexedTypeIdentifier& identifier);

    QString toString() const override;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    Kind kind() const;
    void setKind(Kind kind);

    size_t hash() const override;

    WhichType whichType() const override;

    enum {
        Identity = 8
    };

    using Data = DelayedTypeData;

protected:
    void accept0 (KDevelop::TypeVisitor* v) const override;
    TYPE_DECLARE_DATA(DelayedType)
};
}

#endif
