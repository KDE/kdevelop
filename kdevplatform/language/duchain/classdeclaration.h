/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden@kdevelop.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CLASSDECLARATION_H
#define KDEVPLATFORM_CLASSDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/classmemberdeclarationdata.h>
#include <language/duchain/classmemberdeclaration.h>

namespace KDevelop {
class DUContext;
class TopDUContext;
}

namespace KDevelop {
struct KDEVPLATFORMLANGUAGE_EXPORT BaseClassInstance
{
    KDevelop::IndexedType baseClass; //May either be StructureType, or DelayedType
    KDevelop::Declaration::AccessPolicy access;
    bool virtualInheritance;
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ClassDeclarationData, baseClasses, BaseClassInstance)

class KDEVPLATFORMLANGUAGE_EXPORT ClassDeclarationData
    : public KDevelop::ClassMemberDeclarationData
{
public:
    enum ClassType {
        Class,
        Struct,
        Union,
        Interface,
        Trait
    };

    enum ClassModifier {
        Final,
        Abstract,
        None
    };

    ClassDeclarationData()
        : m_classType(Class)
        , m_classModifier(None)
    {
        initializeAppendedLists();
    }

    ~ClassDeclarationData()
    {
        freeAppendedLists();
    }

    ClassDeclarationData(const ClassDeclarationData& rhs)
        : KDevelop::ClassMemberDeclarationData(rhs)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
        m_classType = rhs.m_classType;
        m_classModifier = rhs.m_classModifier;
    }

    ClassDeclarationData& operator=(const ClassDeclarationData& rhs) = delete;

    /// Type of the class (struct, class, etc.)
    ClassType m_classType;
    /// Modifier of the class (final, abstract, etc.)
    ClassModifier m_classModifier;

    START_APPENDED_LISTS_BASE(ClassDeclarationData, KDevelop::ClassMemberDeclarationData);
    APPENDED_LIST_FIRST(ClassDeclarationData, BaseClassInstance, baseClasses);
    END_APPENDED_LISTS(ClassDeclarationData, baseClasses);
};

/**
 * Represents a single template-parameter definition
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassDeclaration
    : public KDevelop::ClassMemberDeclaration
{
public:
    ClassDeclaration(const ClassDeclaration& rhs);
    explicit ClassDeclaration(ClassDeclarationData& data);
    ClassDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    ClassDeclaration(ClassDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    ~ClassDeclaration() override;

    void clearBaseClasses();
    ///Count of base-classes
    uint baseClassesSize() const;
    ///The types this class is based on
    const BaseClassInstance* baseClasses() const;
    void addBaseClass(const BaseClassInstance& klass);
    //Replaces the n'th base-class with the given one. The replaced base-class must have existed.
    void replaceBaseClass(uint n, const BaseClassInstance& klass);

    /**Returns whether base is a public base-class of this class
     * @param baseConversionLevels If nonzero, this will count the distance of the classes.
     * */
    bool isPublicBaseClass(ClassDeclaration* base, const KDevelop::TopDUContext* topContext,
                           int* baseConversionLevels = nullptr) const;

    QString toString() const override;

    void setClassType(ClassDeclarationData::ClassType type);

    ClassDeclarationData::ClassType classType() const;

    void setClassModifier(ClassDeclarationData::ClassModifier modifier);

    ClassDeclarationData::ClassModifier classModifier() const;

    enum {
        Identity = 17
    };

private:
    KDevelop::Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(ClassDeclaration)
};
}

Q_DECLARE_TYPEINFO(KDevelop::BaseClassInstance, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_CLASSDECLARATION_H
