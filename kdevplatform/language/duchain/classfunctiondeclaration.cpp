/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "classfunctiondeclaration.h"
#include "ducontext.h"
#include "types/functiontype.h"
#include "duchainregister.h"
#include <debug.h>

namespace KDevelop {
static Identifier& conversionIdentifier()
{
    static Identifier conversionIdentifierObject(QStringLiteral("operator{...cast...}"));
    return conversionIdentifierObject;
}

REGISTER_DUCHAIN_ITEM(ClassFunctionDeclaration);

ClassFunctionDeclaration::ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs)
    : ClassFunctionDeclarationBase(*new ClassFunctionDeclarationData(*rhs.d_func()))
{
}

void ClassFunctionDeclaration::setAbstractType(AbstractType::Ptr type)
{
    ///TODO: write testcase for typealias case which used to trigger this warning:
    ///      typedef bool (*EventFilter)(void *message, long *result);
    ///      in e.g. qcoreapplication.h:172
    if (type && !dynamic_cast<FunctionType*>(type.data()) && type->whichType() != AbstractType::TypeAlias) {
        qCWarning(LANGUAGE) << "WARNING: Non-function type assigned to function declaration. Type is: "
                            << type->toString() << "whichType:" << type->whichType()
                            << "Declaration is:" << toString()
                            << topContext()->url().str() << range().castToSimpleRange();
    }
    ClassMemberDeclaration::setAbstractType(type);
}

DEFINE_LIST_MEMBER_HASH(ClassFunctionDeclarationData, m_defaultParameters, IndexedString)

ClassFunctionDeclaration::ClassFunctionDeclaration(ClassFunctionDeclarationData& data) : ClassFunctionDeclarationBase(
        data)
{
}

ClassFunctionDeclaration::ClassFunctionDeclaration(const RangeInRevision& range, DUContext* context)
    : ClassFunctionDeclarationBase(*new ClassFunctionDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

ClassFunctionDeclaration::ClassFunctionDeclaration(ClassFunctionDeclarationData& data, const RangeInRevision& range,
                                                   DUContext* context)
    : ClassFunctionDeclarationBase(data, range)
{
    if (context)
        setContext(context);
}

Declaration* ClassFunctionDeclaration::clonePrivate() const
{
    return new ClassFunctionDeclaration(*this);
}

ClassFunctionDeclaration::~ClassFunctionDeclaration()
{
}

bool ClassFunctionDeclaration::isFunctionDeclaration() const
{
    return true;
}

QString ClassFunctionDeclaration::toString() const
{
    if (!abstractType())
        return ClassMemberDeclaration::toString();

    TypePtr<FunctionType> function = type<FunctionType>();
    if (function) {
        return QStringLiteral("%1 %2 %3").arg(function->partToString(FunctionType::SignatureReturn),
                                              identifier().toString(),
                                              function->partToString(FunctionType::SignatureArguments));
    } else {
        QString type = abstractType() ? abstractType()->toString() : QStringLiteral("<notype>");
        qCDebug(LANGUAGE) << "A function has a bad type attached:" << type;
        return i18n("invalid member-function %1 type %2", identifier().toString(), type);
    }
}

/*bool ClassFunctionDeclaration::isSimilar(KDevelop::CodeItem *other, bool strict ) const
   {
   if (!CppClassMemberType::isSimilar(other,strict))
    return false;

   FunctionModelItem func = dynamic_cast<ClassFunctionDeclaration*>(other);

   if (isConstant() != func->isConstant())
    return false;

   if (arguments().count() != func->arguments().count())
    return false;

   for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

   return true;
   }*/

uint setFlag(bool enable, uint flag, uint flags)
{
    if (enable)
        return flags | flag;
    else
        return flags & (~flag);
}

bool ClassFunctionDeclaration::isAbstract() const
{
    return d_func()->m_functionFlags & AbstractFunctionFlag;
}

void ClassFunctionDeclaration::setIsAbstract(bool abstract)
{
    d_func_dynamic()->m_functionFlags = ( ClassFunctionFlags )setFlag(abstract, AbstractFunctionFlag,
                                                                      d_func()->m_functionFlags);
}

bool ClassFunctionDeclaration::isFinal() const
{
    return d_func()->m_functionFlags & FinalFunctionFlag;
}

void ClassFunctionDeclaration::setIsFinal(bool final)
{
    d_func_dynamic()->m_functionFlags = ( ClassFunctionFlags )setFlag(final, FinalFunctionFlag,
                                                                      d_func()->m_functionFlags);
}

bool ClassFunctionDeclaration::isSignal() const
{
    return d_func()->m_functionFlags & FunctionSignalFlag;
}

void ClassFunctionDeclaration::setIsSignal(bool isSignal)
{
    d_func_dynamic()->m_functionFlags = ( ClassFunctionFlags )setFlag(isSignal, FunctionSignalFlag,
                                                                      d_func()->m_functionFlags);
}

bool ClassFunctionDeclaration::isSlot() const
{
    return d_func()->m_functionFlags & FunctionSlotFlag;
}

void ClassFunctionDeclaration::setIsSlot(bool isSlot)
{
    d_func_dynamic()->m_functionFlags = ( ClassFunctionFlags )setFlag(isSlot, FunctionSlotFlag,
                                                                      d_func()->m_functionFlags);
}

bool ClassFunctionDeclaration::isConversionFunction() const
{
    return identifier() == conversionIdentifier();
}

bool ClassFunctionDeclaration::isConstructor() const
{
    DUContext* ctx = context();
    if (ctx && ctx->type() == DUContext::Class && ctx->localScopeIdentifier().top().nameEquals(identifier()))
        return true;
    return false;
}

bool ClassFunctionDeclaration::isDestructor() const
{
    DUContext* ctx = context();
    QString id = identifier().toString();
    return ctx && ctx->type() == DUContext::Class && id.startsWith(QLatin1Char('~')) &&
           QStringView{id}.sliced(1) == ctx->localScopeIdentifier().top().toString();
}

uint ClassFunctionDeclaration::additionalIdentity() const
{
    if (abstractType())
        return abstractType()->hash();
    else
        return 0;
}

const IndexedString* ClassFunctionDeclaration::defaultParameters() const
{
    return d_func()->m_defaultParameters();
}

unsigned int ClassFunctionDeclaration::defaultParametersSize() const
{
    return d_func()->m_defaultParametersSize();
}

void ClassFunctionDeclaration::addDefaultParameter(const IndexedString& str)
{
    d_func_dynamic()->m_defaultParametersList().append(str);
}

void ClassFunctionDeclaration::clearDefaultParameters()
{
    d_func_dynamic()->m_defaultParametersList().clear();
}
}
