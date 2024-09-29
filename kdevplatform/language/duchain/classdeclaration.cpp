/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden@kdevelop.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "classdeclaration.h"
#include "identifier.h"
#include "types/structuretype.h"

#include <debug.h>

#include <language/duchain/declaration.h>
#include <language/duchain/appendedlist.h>
#include <language/duchain/duchainregister.h>
#include <util/algorithm.h>

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(ClassDeclarationData, baseClasses, BaseClassInstance)

ClassDeclaration::ClassDeclaration(const KDevelop::RangeInRevision& range, DUContext* context)
    : ClassMemberDeclaration(*new ClassDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    setContext(context);
}

ClassDeclaration::ClassDeclaration(ClassDeclarationData& data, const KDevelop::RangeInRevision& range,
                                   DUContext* context)
    : ClassMemberDeclaration(data, range)
{
    setContext(context);
}

ClassDeclaration::ClassDeclaration(ClassDeclarationData& data)
    : ClassMemberDeclaration(data)
{
}

REGISTER_DUCHAIN_ITEM(ClassDeclaration);

void ClassDeclaration::clearBaseClasses()
{
    d_func_dynamic()->baseClassesList().clear();
}

uint ClassDeclaration::baseClassesSize() const
{
    return d_func()->baseClassesSize();
}

const BaseClassInstance* ClassDeclaration::baseClasses() const
{
    return d_func()->baseClasses();
}

void ClassDeclaration::addBaseClass(const BaseClassInstance& klass)
{
    d_func_dynamic()->baseClassesList().append(klass);
}

void ClassDeclaration::replaceBaseClass(uint n, const BaseClassInstance& klass)
{
    Q_ASSERT(n <= d_func()->baseClassesSize());
    d_func_dynamic()->baseClassesList()[n] = klass;
}

ClassDeclaration::~ClassDeclaration()
{
}

ClassDeclaration::ClassDeclaration(const ClassDeclaration& rhs)
    : ClassMemberDeclaration(*new ClassDeclarationData(*rhs.d_func()))
{
    d_func_dynamic()->setClassId(this);
}

Declaration* ClassDeclaration::clonePrivate() const
{
    return new ClassDeclaration(*this);
}

namespace {
bool isPublicBaseClassInternal(const ClassDeclaration* self, ClassDeclaration* base,
                               const KDevelop::TopDUContext* topContext,
                               int* baseConversionLevels, int depth, QSet<const ClassDeclaration*>* checked)
{
    if (checked) {
        if (!Algorithm::insert(*checked, self).inserted) {
            return false;
        }
    } else if (depth > 3) {
        //Too much depth, to prevent endless recursion, we control the recursion using the 'checked' set
        QSet<const ClassDeclaration*> checkedSet;
        return isPublicBaseClassInternal(self, base, topContext, baseConversionLevels, depth, &checkedSet);
    }

    if (baseConversionLevels)
        *baseConversionLevels = 0;

    if (self->indexedType() == base->indexedType())
        return true;

    FOREACH_FUNCTION(const BaseClassInstance &b, self->baseClasses)
    {
        if (baseConversionLevels)
            ++(*baseConversionLevels);
        //qCDebug(LANGUAGE) << "public base of" << c->toString() << "is" << b.baseClass->toString();
        if (b.access != KDevelop::Declaration::Private) {
            int nextBaseConversion = 0;
            if (StructureType::Ptr c = b.baseClass.type<StructureType>()) {
                auto* decl = dynamic_cast<ClassDeclaration*>(c->declaration(topContext));
                if (decl &&
                    isPublicBaseClassInternal(decl, base, topContext, &nextBaseConversion, depth + 1, checked)) {
                    if (baseConversionLevels)
                        *baseConversionLevels += nextBaseConversion;
                    return true;
                }
            }
        }
        if (baseConversionLevels)
            --(*baseConversionLevels);
    }
    return false;
}
}

bool ClassDeclaration::isPublicBaseClass(ClassDeclaration* base, const KDevelop::TopDUContext* topContext,
                                         int* baseConversionLevels) const
{
    return isPublicBaseClassInternal(this, base, topContext, baseConversionLevels, 0, nullptr);
}

QString ClassDeclaration::toString() const
{
    QString ret;
    switch (classModifier()) {
    case ClassDeclarationData::None:
        //nothing
        break;
    case ClassDeclarationData::Abstract:
        ret += QLatin1String("abstract ");
        break;
    case ClassDeclarationData::Final:
        ret += QLatin1String("final ");
        break;
    }
    switch (classType()) {
    case ClassDeclarationData::Class:
        ret += QLatin1String("class ");
        break;
    case ClassDeclarationData::Interface:
        ret += QLatin1String("interface ");
        break;
    case ClassDeclarationData::Trait:
        ret += QLatin1String("trait ");
        break;
    case ClassDeclarationData::Union:
        ret += QLatin1String("union ");
        break;
    case ClassDeclarationData::Struct:
        ret += QLatin1String("struct ");
        break;
    }
    return ret + identifier().toString();
}

ClassDeclarationData::ClassType ClassDeclaration::classType() const
{
    return d_func()->m_classType;
}

void ClassDeclaration::setClassType(ClassDeclarationData::ClassType type)
{
    d_func_dynamic()->m_classType = type;
}

ClassDeclarationData::ClassModifier ClassDeclaration::classModifier() const
{
    return d_func()->m_classModifier;
}

void ClassDeclaration::setClassModifier(ClassDeclarationData::ClassModifier modifier)
{
    d_func_dynamic()->m_classModifier = modifier;
}
}
