/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "abstractdeclarationnavigationcontext.h"

#include <KLocalizedString>

#include "../functiondeclaration.h"
#include "../functiondefinition.h"
#include "../classfunctiondeclaration.h"
#include "../namespacealiasdeclaration.h"
#include "../forwarddeclaration.h"
#include "../types/enumeratortype.h"
#include "../types/enumerationtype.h"
#include "../types/functiontype.h"
#include "../duchainutils.h"
#include "../types/pointertype.h"
#include "../types/referencetype.h"
#include "../types/typeutils.h"
#include "../types/typesystem.h"
#include "../persistentsymboltable.h"
#include <debug.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <duchain/types/typealiastype.h>
#include <duchain/types/structuretype.h>
#include <duchain/classdeclaration.h>

#include <QRegularExpression>
#include <QTextDocument>

namespace KDevelop {
class AbstractDeclarationNavigationContextPrivate
{
public:
    DeclarationPointer m_declaration;
    bool m_fullBackwardSearch = false;
};

AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext(const DeclarationPointer& decl,
                                                                           const TopDUContextPointer& topContext,
                                                                           AbstractNavigationContext* previousContext)
    : AbstractNavigationContext((topContext ? topContext : TopDUContextPointer(
                                     decl ? decl->topContext() : nullptr)), previousContext)
    , d_ptr(new AbstractDeclarationNavigationContextPrivate)
{
    Q_D(AbstractDeclarationNavigationContext);

    d->m_declaration = decl;

    //Jump from definition to declaration if possible
    auto* definition = dynamic_cast<FunctionDefinition*>(d->m_declaration.data());
    if (definition && definition->declaration())
        d->m_declaration = DeclarationPointer(definition->declaration());
}

AbstractDeclarationNavigationContext::~AbstractDeclarationNavigationContext()
{
}

QString AbstractDeclarationNavigationContext::name() const
{
    Q_D(const AbstractDeclarationNavigationContext);

    if (d->m_declaration.data())
        return prettyQualifiedIdentifier(d->m_declaration).toString();
    else
        return declarationName(d->m_declaration);
}

QString AbstractDeclarationNavigationContext::html(bool shorten)
{
    Q_D(AbstractDeclarationNavigationContext);

    DUChainReadLocker lock(DUChain::lock(), 300);
    if (!lock.locked()) {
        return {};
    }

    clear();
    AbstractNavigationContext::html(shorten);

    modifyHtml()  += QStringLiteral("<html><body>");

    if (!d->m_declaration.data()) {
        modifyHtml() += QLatin1String("<p>") + i18n("lost declaration") + QLatin1String("</p></body></html>");
        return currentHtml();
    }

    if (auto context = previousContext()) {
        const QString link = createLink(context->name(), context->name(),
                                        NavigationAction(context));
        modifyHtml() += navigationHighlight(i18n("Back to %1<br />", link));
    }

    QExplicitlySharedDataPointer<IDocumentation> doc;

    modifyHtml() += QStringLiteral("<p>");

    if (!shorten) {
        doc = ICore::self()->documentationController()->documentationForDeclaration(d->m_declaration.data());

        const auto* function =
            dynamic_cast<const AbstractFunctionDeclaration*>(d->m_declaration.data());
        if (function) {
            htmlFunction();
        } else if (d->m_declaration->isTypeAlias() || d->m_declaration->type<EnumeratorType>() ||
                   d->m_declaration->kind() == Declaration::Instance) {
            if (d->m_declaration->isTypeAlias())
                modifyHtml() += importantHighlight(QStringLiteral("typedef "));

            if (d->m_declaration->type<EnumeratorType>())
                modifyHtml() += i18n("enumerator ");

            AbstractType::Ptr useType = d->m_declaration->abstractType();
            if (d->m_declaration->isTypeAlias()) {
                //Do not show the own name as type of typedefs
                if (auto alias = useType.dynamicCast<TypeAliasType>())
                    useType = alias->type();
            }

            eventuallyMakeTypeLinks(useType);

            modifyHtml() += QLatin1Char(' ') + identifierHighlight(declarationName(
                                                                       d->m_declaration).toHtmlEscaped(),
                                                                   d->m_declaration);

            if (auto memberDecl = dynamic_cast<const ClassMemberDeclaration*>(d->m_declaration.data())) {
                const auto bitWidth = memberDecl->bitWidth();
                switch (bitWidth) {
                case ClassMemberDeclaration::NotABitField:
                    break;
                case ClassMemberDeclaration::ValueDependentBitWidth:
                    modifyHtml() += i18n(" :[value-dependent]");
                    break;
                default:
                    modifyHtml() += QLatin1String(" :") + QString::number(bitWidth);
                }
            }

            if (auto integralType = d->m_declaration->type<ConstantIntegralType>()) {
                const QString plainValue = integralType->valueAsString();
                if (!plainValue.isEmpty()) {
                    modifyHtml() += QStringLiteral(" = ") + plainValue;
                }
            }

            modifyHtml() += QStringLiteral("<br>");
        } else {
            if (d->m_declaration->kind() == Declaration::Type
                && d->m_declaration->abstractType().dynamicCast<StructureType>()) {
                htmlClass();
            }
            if (d->m_declaration->kind() == Declaration::Namespace) {
                modifyHtml() +=
                    i18n("namespace %1 ",
                         identifierHighlight(d->m_declaration->qualifiedIdentifier().toString().toHtmlEscaped(),
                                             d->m_declaration));
            } else if (d->m_declaration->kind() == Declaration::NamespaceAlias) {
                modifyHtml() +=
                    identifierHighlight(declarationName(d->m_declaration).toHtmlEscaped(), d->m_declaration);
            }

            if (d->m_declaration->type<EnumerationType>()) {
                EnumerationType::Ptr enumeration = d->m_declaration->type<EnumerationType>();
                modifyHtml() +=
                    i18n("enumeration %1 ",
                         identifierHighlight(
                             d->m_declaration->identifier().toString().toHtmlEscaped(), d->m_declaration));
            }

            if (d->m_declaration->isForwardDeclaration()) {
                auto* forwardDec = static_cast<ForwardDeclaration*>(d->m_declaration.data());
                Declaration* resolved = forwardDec->resolve(topContext().data());
                if (resolved) {
                    modifyHtml() += i18n("(resolved forward-declaration: ");
                    makeLink(resolved->identifier().toString(), DeclarationPointer(
                                 resolved), NavigationAction::NavigateDeclaration);
                    modifyHtml() += i18n(") ");
                } else {
                    modifyHtml() += i18n("(unresolved forward-declaration) ");
                    QualifiedIdentifier id = forwardDec->qualifiedIdentifier();
                    const auto& forwardDecFile = forwardDec->topContext()->parsingEnvironmentFile();
                    auto visitor = [&](const IndexedDeclaration& indexedDec) {
                        auto dec = indexedDec.data();
                        if (!dec || dec->isForwardDeclaration()) {
                            return PersistentSymbolTable::VisitorState::Continue;
                        }
                        const auto& decFile = forwardDec->topContext()->parsingEnvironmentFile();
                        if ((static_cast<bool>(decFile) != static_cast<bool>(forwardDecFile)) ||
                            (decFile && forwardDecFile && decFile->language() != forwardDecFile->language())) {
                            // the language of the declarations must match
                            return PersistentSymbolTable::VisitorState::Continue;
                        }
                        modifyHtml() += QStringLiteral("<br />");
                        makeLink(i18n("possible resolution from"), DeclarationPointer(
                                     dec), NavigationAction::NavigateDeclaration);
                        modifyHtml() += QLatin1Char(' ') + dec->url().str();
                        return PersistentSymbolTable::VisitorState::Continue;
                    };
                    PersistentSymbolTable::self().visitDeclarations(id, visitor);
                }
            }
            modifyHtml() += QStringLiteral("<br />");
        }
    } else {
        auto showType = d->m_declaration->abstractType();
        if (auto functionType = showType.dynamicCast<FunctionType>()) {
            showType = functionType->returnType();
            if (showType)
                modifyHtml() += labelHighlight(i18n("Returns: "));
        } else if (showType) {
            modifyHtml() += labelHighlight(i18n("Type: "));
        }

        if (showType) {
            eventuallyMakeTypeLinks(showType);
            modifyHtml() += QStringLiteral(" ");
        }
    }

    QualifiedIdentifier identifier = d->m_declaration->qualifiedIdentifier();
    if (identifier.count() > 1) {
        if (d->m_declaration->context() && d->m_declaration->context()->owner()) {
            Declaration* decl = d->m_declaration->context()->owner();

            auto* definition = dynamic_cast<FunctionDefinition*>(decl);
            if (definition && definition->declaration())
                decl = definition->declaration();

            if (decl->abstractType().dynamicCast<EnumerationType>())
                modifyHtml() += labelHighlight(i18n("Enum: "));
            else
                modifyHtml() += labelHighlight(i18n("Container: "));

            makeLink(declarationName(DeclarationPointer(decl)), DeclarationPointer(
                         decl), NavigationAction::NavigateDeclaration);
            modifyHtml() += QStringLiteral(" ");
        } else {
            QualifiedIdentifier parent = identifier;
            parent.pop();
            modifyHtml() += labelHighlight(i18n("Scope: %1 ", typeHighlight(parent.toString().toHtmlEscaped())));
        }
    }

    if (shorten && !d->m_declaration->comment().isEmpty()) {
        QString comment = QString::fromUtf8(d->m_declaration->comment());
        if (comment.length() > 60) {
            comment.truncate(60);
            comment += QLatin1String("...");
        }
        comment.replace(QLatin1Char('\n'), QLatin1Char(' '));
        comment.replace(QLatin1String("<br />"), QLatin1String(" "));
        comment.replace(QLatin1String("<br/>"), QLatin1String(" "));
        modifyHtml() += commentHighlight(comment.toHtmlEscaped()) + QLatin1String("   ");
    }

    QString access = stringFromAccess(d->m_declaration);
    if (!access.isEmpty())
        modifyHtml() += labelHighlight(i18n("Access: %1 ", propertyHighlight(access.toHtmlEscaped())));

    ///@todo Enumerations

    QString detailsHtml;
    const QStringList details = declarationDetails(d->m_declaration);
    if (!details.isEmpty()) {
        bool first = true;
        for (const QString& str : details) {
            if (!first)
                detailsHtml += QLatin1String(", ");
            first = false;
            detailsHtml += propertyHighlight(str);
        }
    }

    QString kind = declarationKind(d->m_declaration);
    if (!kind.isEmpty()) {
        if (!detailsHtml.isEmpty())
            modifyHtml() += labelHighlight(i18n("Kind: %1 %2 ", importantHighlight(kind.toHtmlEscaped()), detailsHtml));
        else
            modifyHtml() += labelHighlight(i18n("Kind: %1 ", importantHighlight(kind.toHtmlEscaped())));
    }

    if (d->m_declaration->isDeprecated()) {
        modifyHtml() += labelHighlight(i18n("Status: %1 ", propertyHighlight(i18n("Deprecated"))));
    }

    modifyHtml() += QStringLiteral("<br />");

    if (!shorten)
        htmlAdditionalNavigation();

    if (!shorten) {
        if (dynamic_cast<FunctionDefinition*>(d->m_declaration.data()))
            modifyHtml() += labelHighlight(i18n("Def.: "));
        else
            modifyHtml() += labelHighlight(i18n("Decl.: "));

        makeLink(QStringLiteral("%1 :%2").arg(d->m_declaration->url().toUrl().fileName()).arg(d->m_declaration->
                                                                                              rangeInCurrentRevision().
                                                                                              start().line() + 1), d->m_declaration,
                 NavigationAction::JumpToSource);
        modifyHtml() += QStringLiteral(" ");
        //modifyHtml() += "<br />";
        if (!dynamic_cast<FunctionDefinition*>(d->m_declaration.data())) {
            if (auto* definition = FunctionDefinition::definition(d->m_declaration.data())) {
                modifyHtml() += labelHighlight(i18n(" Def.: "));
                makeLink(QStringLiteral("%1 :%2").arg(definition->url().toUrl().fileName()).arg(definition->
                                                                                                rangeInCurrentRevision()
                                                                                                .start().line() + 1), DeclarationPointer(
                             definition), NavigationAction::JumpToSource);
            }
        }

        if (auto* definition = dynamic_cast<FunctionDefinition*>(d->m_declaration.data())) {
            if (definition->declaration()) {
                modifyHtml() += labelHighlight(i18n(" Decl.: "));
                makeLink(QStringLiteral("%1 :%2").arg(definition->declaration()->url().toUrl().fileName()).arg(
                             definition->declaration()->rangeInCurrentRevision().start().line() + 1),
                         DeclarationPointer(definition->declaration()), NavigationAction::JumpToSource);
            }
        }

        modifyHtml() += QStringLiteral(" "); //The action name _must_ stay "show_uses", since that is also used from outside
        makeLink(i18n("Show uses"), QStringLiteral("show_uses"),
                 NavigationAction(d->m_declaration, NavigationAction::NavigateUses));

    }
    modifyHtml() += QStringLiteral("</p>");

    QByteArray declarationComment = d->m_declaration->comment();
    if (!shorten && (!declarationComment.isEmpty() || doc)) {
        if (doc) {
            QString comment = doc->description();
            connect(
                doc.data(), &IDocumentation::descriptionChanged, this,
                &AbstractDeclarationNavigationContext::contentsChanged);

            if (!comment.isEmpty()) {
                modifyHtml() += QLatin1String("<p>") + commentHighlight(comment) + QLatin1String("</p>");
            }
        }

        QString comment = QString::fromUtf8(declarationComment);
        if (!comment.isEmpty()) {
            // if the first paragraph does not contain a tag, we assume that this is a plain-text comment
            if (!Qt::mightBeRichText(comment)) {
                // still might contain extra html tags for line breaks (this is the case for doxygen-style comments sometimes)
                // let's protect them from being removed completely
                static const QRegularExpression brTagRegex(QStringLiteral("<br */>"));
                comment.replace(brTagRegex, QStringLiteral("\n"));
                comment = comment.toHtmlEscaped();
                comment.replace(QLatin1Char('\n'), QLatin1String("<br />")); //Replicate newlines in html
            }
            modifyHtml() += QLatin1String("<p>") + commentHighlight(comment) + QLatin1String("</p>");
        }
    }

    if (!shorten) {
        modifyHtml() += declarationSizeInformation(d->m_declaration, topContext().data());
    }

    if (!shorten && doc) {
        modifyHtml() += QLatin1String("<p>") + i18n("Show documentation for ");
        makeLink(prettyQualifiedName(d->m_declaration),
                 d->m_declaration, NavigationAction::ShowDocumentation);
        modifyHtml() += QStringLiteral("</p>");
    }

    modifyHtml() += QStringLiteral("</body></html>");

    return currentHtml();
}

AbstractType::Ptr AbstractDeclarationNavigationContext::typeToShow(AbstractType::Ptr type)
{
    return type;
}

void AbstractDeclarationNavigationContext::htmlFunction()
{
    Q_D(AbstractDeclarationNavigationContext);

    const auto* function =
        dynamic_cast<const AbstractFunctionDeclaration*>(d->m_declaration.data());
    Q_ASSERT(function);

    const auto* classFunDecl =
        dynamic_cast<const ClassFunctionDeclaration*>(d->m_declaration.data());
    const auto type = d->m_declaration->abstractType().dynamicCast<FunctionType>();
    if (!type) {
        modifyHtml() += errorHighlight(QStringLiteral("Invalid type<br />"));
        return;
    }

    if (!classFunDecl || (!classFunDecl->isConstructor() && !classFunDecl->isDestructor())) {
        // only print return type for global functions and non-ctor/dtor methods
        eventuallyMakeTypeLinks(type->returnType());
    }

    modifyHtml() += QLatin1Char(' ') + identifierHighlight(prettyIdentifier(
                                                               d->m_declaration).toString().toHtmlEscaped(),
                                                           d->m_declaration);

    if (type->indexedArgumentsSize() == 0) {
        modifyHtml() += QStringLiteral("()");
    } else {
        modifyHtml() += QStringLiteral("( ");

        bool first = true;
        int firstDefaultParam = type->indexedArgumentsSize() - function->defaultParametersSize();
        int currentArgNum = 0;

        QVector<Declaration*> decls;
        if (DUContext* argumentContext = DUChainUtils::argumentContext(d->m_declaration.data())) {
            decls = argumentContext->localDeclarations(topContext().data());
        }
        const auto argTypes = type->arguments();
        for (const AbstractType::Ptr& argType : argTypes) {
            if (!first)
                modifyHtml() += QStringLiteral(", ");
            first = false;

            eventuallyMakeTypeLinks(argType);
            // Must count from the back to skip possible template arguments before the function arguments.
            int currentArgIndex = decls.size() - type->arguments().size() + currentArgNum;
            if (currentArgIndex >= 0 && currentArgIndex < decls.size()) {
                modifyHtml() += QLatin1Char(' ') + identifierHighlight(
                    decls[currentArgIndex]->identifier().toString().toHtmlEscaped(), d->m_declaration);
            }

            if (currentArgNum >= firstDefaultParam) {
                IndexedString defaultStr = function->defaultParameters()[currentArgNum - firstDefaultParam];
                if (!defaultStr.isEmpty()) {
                    modifyHtml() += QLatin1String(" = ") + defaultStr.str().toHtmlEscaped();
                }
            }

            ++currentArgNum;
        }

        modifyHtml() += QStringLiteral(" )");
    }
    modifyHtml() += QStringLiteral("<br />");
}

Identifier AbstractDeclarationNavigationContext::prettyIdentifier(const DeclarationPointer& decl) const
{
    Identifier ret;
    QualifiedIdentifier q = prettyQualifiedIdentifier(decl);
    if (!q.isEmpty())
        ret = q.last();

    return ret;
}

QualifiedIdentifier AbstractDeclarationNavigationContext::prettyQualifiedIdentifier(const DeclarationPointer& decl)
const
{
    if (decl)
        return decl->qualifiedIdentifier();
    else
        return QualifiedIdentifier();
}

QString AbstractDeclarationNavigationContext::prettyQualifiedName(const DeclarationPointer& decl) const
{
    const auto qid = prettyQualifiedIdentifier(decl);
    if (qid.isEmpty()) {
        return i18nc("An anonymous declaration (class, function, etc.)", "<anonymous>");
    }

    return qid.toString();
}

void AbstractDeclarationNavigationContext::htmlAdditionalNavigation()
{
    Q_D(AbstractDeclarationNavigationContext);

    ///Check if the function overrides or hides another one
    const auto* classFunDecl =
        dynamic_cast<const ClassFunctionDeclaration*>(d->m_declaration.data());
    if (classFunDecl) {
        Declaration* overridden = DUChainUtils::overridden(d->m_declaration.data());

        if (overridden) {
            modifyHtml() += i18n("Overrides a ");
            makeLink(i18n("function"), QStringLiteral("jump_to_overridden"),
                     NavigationAction(DeclarationPointer(overridden), NavigationAction::NavigateDeclaration));
            modifyHtml() += i18n(" from ");
            makeLink(prettyQualifiedName(DeclarationPointer(overridden->context()->owner())),
                     QStringLiteral("jump_to_overridden_container"),
                     NavigationAction(DeclarationPointer(overridden->context()->owner()),
                                      NavigationAction::NavigateDeclaration));

            modifyHtml() += QStringLiteral("<br />");
        } else {
            //Check if this declarations hides other declarations
            QList<Declaration*> decls;
            const auto importedParentContexts = d->m_declaration->context()->importedParentContexts();
            for (const DUContext::Import& import : importedParentContexts) {
                if (import.context(topContext().data()))
                    decls +=
                        import.context(topContext().data())->findDeclarations(QualifiedIdentifier(d->m_declaration->
                                                                                                  identifier()),
                                                                              CursorInRevision::invalid(),
                                                                              AbstractType::Ptr(),
                                                                              topContext().data(),
                                                                              DUContext::DontSearchInParent);
            }

            uint num = 0;
            for (Declaration* decl : std::as_const(decls)) {
                modifyHtml() += i18n("Hides a ");
                makeLink(i18n("function"), QStringLiteral("jump_to_hide_%1").arg(num),
                         NavigationAction(DeclarationPointer(decl),
                                          NavigationAction::NavigateDeclaration));
                modifyHtml() += i18n(" from ");
                makeLink(prettyQualifiedName(DeclarationPointer(decl->context()->owner())),
                         QStringLiteral("jump_to_hide_container_%1").arg(num),
                         NavigationAction(DeclarationPointer(decl->context()->owner()),
                                          NavigationAction::NavigateDeclaration));

                modifyHtml() += QStringLiteral("<br />");
                ++num;
            }
        }

        ///Show all places where this function is overridden
        if (classFunDecl->isVirtual()) {
            Declaration* classDecl = d->m_declaration->context()->owner();
            if (classDecl) {
                uint maxAllowedSteps = d->m_fullBackwardSearch ? (uint) - 1 : 10;
                const QList<Declaration*> overriders =
                    DUChainUtils::overriders(classDecl, classFunDecl, maxAllowedSteps);

                if (!overriders.isEmpty()) {
                    modifyHtml() += i18n("Overridden in ");
                    bool first = true;
                    for (Declaration* overrider : overriders) {
                        if (!first)
                            modifyHtml() += QStringLiteral(", ");
                        first = false;

                        const auto owner = DeclarationPointer(overrider->context()->owner());
                        const QString name = prettyQualifiedName(owner);
                        makeLink(name, name,
                                 NavigationAction(DeclarationPointer(overrider),
                                                  NavigationAction::NavigateDeclaration));
                    }

                    modifyHtml() += QStringLiteral("<br />");
                }
                if (maxAllowedSteps == 0)
                    createFullBackwardSearchLink(overriders.isEmpty() ? i18n("Overriders possible, show all") : i18n(
                                                     "More overriders possible, show all"));
            }
        }
    }

    ///Show all classes that inherit this one
    uint maxAllowedSteps = d->m_fullBackwardSearch ? (uint) - 1 : 10;
    const QList<Declaration*> inheriters = DUChainUtils::inheriters(d->m_declaration.data(), maxAllowedSteps);

    if (!inheriters.isEmpty()) {
        modifyHtml() += i18n("Inherited by ");
        bool first = true;
        for (Declaration* importer : inheriters) {
            if (!first)
                modifyHtml() += QStringLiteral(", ");
            first = false;

            const QString importerName = prettyQualifiedName(DeclarationPointer(importer));
            makeLink(importerName, importerName,
                     NavigationAction(DeclarationPointer(importer), NavigationAction::NavigateDeclaration));
        }

        modifyHtml() += QStringLiteral("<br />");
    }
    if (maxAllowedSteps == 0)
        createFullBackwardSearchLink(inheriters.isEmpty() ? i18n("Inheriters possible, show all") : i18n(
                                         "More inheriters possible, show all"));
}

void AbstractDeclarationNavigationContext::createFullBackwardSearchLink(const QString& string)
{
    makeLink(string, QStringLiteral("m_fullBackwardSearch=true"),
             NavigationAction(QStringLiteral("m_fullBackwardSearch=true")));
    modifyHtml() += QStringLiteral("<br />");
}

NavigationContextPointer AbstractDeclarationNavigationContext::executeKeyAction(const QString& key)
{
    Q_D(AbstractDeclarationNavigationContext);

    if (key == QLatin1String("m_fullBackwardSearch=true")) {
        d->m_fullBackwardSearch = true;
        clear();
    }
    return NavigationContextPointer(this);
}

void AbstractDeclarationNavigationContext::htmlClass()
{
    Q_D(AbstractDeclarationNavigationContext);

    Q_ASSERT(d->m_declaration->abstractType());
    auto klass = d->m_declaration->abstractType().staticCast<StructureType>();

    ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(klass->declaration(topContext().data()));
    if (classDecl) {
        switch (classDecl->classType()) {
        case ClassDeclarationData::Class:
            modifyHtml() += QStringLiteral("class ");
            break;
        case ClassDeclarationData::Struct:
            modifyHtml() += QStringLiteral("struct ");
            break;
        case ClassDeclarationData::Union:
            modifyHtml() += QStringLiteral("union ");
            break;
        case ClassDeclarationData::Interface:
            modifyHtml() += QStringLiteral("interface ");
            break;
        case ClassDeclarationData::Trait:
            modifyHtml() += QStringLiteral("trait ");
            break;
        }
        eventuallyMakeTypeLinks(klass);

        FOREACH_FUNCTION(const BaseClassInstance &base, classDecl->baseClasses) {
            modifyHtml() += QLatin1String(", ") + stringFromAccess(base.access) + QLatin1Char(' ') +
                            (base.virtualInheritance ? QStringLiteral("virtual") : QString()) + QLatin1Char(' ');
            eventuallyMakeTypeLinks(base.baseClass.abstractType());
        }
    } else {
        /// @todo How can we get here? and should this really be a class?
        modifyHtml() += QStringLiteral("class ");
        eventuallyMakeTypeLinks(klass);
    }
    modifyHtml() += QStringLiteral(" ");
}

void AbstractDeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
    if (!type) {
        qCDebug(LANGUAGE) << "null type!";
        return;
    }

    if (!idType) {
        qCDebug(LANGUAGE) << "no identified type for" << type->toString();
        modifyHtml() += typeHighlight(type->toString().toHtmlEscaped());
        return;
    }

    auto* decl = idType->declaration(topContext().data());
    if (!decl) {
        qCDebug(LANGUAGE) << "could not resolve declaration:" << idType->declarationId().isDirect() <<
            idType->qualifiedIdentifier().toString() << "in top-context" << topContext()->url().str();
        modifyHtml() += typeHighlight(type->toString().toHtmlEscaped());
        return;
    }

    //Remove the last template-identifiers, because we create those directly
    QualifiedIdentifier id = prettyQualifiedIdentifier(DeclarationPointer(decl));
    Identifier lastId = id.last();
    id.pop();
    lastId.clearTemplateIdentifiers();
    id.push(lastId);

    if (decl->context() && decl->context()->owner()) {
        //Also create full type-links for the context around
        AbstractType::Ptr contextType = decl->context()->owner()->abstractType();
        auto* contextIdType = dynamic_cast<IdentifiedType*>(contextType.data());
        if (contextIdType && !contextIdType->equals(idType)) {
            //Create full type information for the context
            if (!id.isEmpty())
                id = id.mid(id.count() - 1);
            htmlIdentifiedType(contextType, contextIdType);
            modifyHtml() += QStringLiteral("::").toHtmlEscaped();
        }
    }

    //We leave out the * and & reference and pointer signs, those are added to the end
    makeLink(id.toString(), DeclarationPointer(idType->declaration(
                                                   topContext().data())), NavigationAction::NavigateDeclaration);
}

void AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks(AbstractType::Ptr type)
{
    type = typeToShow(type);

    if (!type) {
        modifyHtml() += typeHighlight(QStringLiteral("<no type>").toHtmlEscaped());
        return;
    }

    AbstractType::Ptr target = TypeUtils::targetTypeKeepAliases(type, topContext().data());
    const auto* idType = dynamic_cast<const IdentifiedType*>(target.data());

    qCDebug(LANGUAGE) << "making type-links for" << type->toString();

    if (idType && idType->declaration(topContext().data())) {
        ///@todo This is C++ specific, move into subclass

        if (target->modifiers() & AbstractType::ConstModifier)
            modifyHtml() += typeHighlight(QStringLiteral("const "));

        htmlIdentifiedType(target, idType);

        //We need to exchange the target type, else template-parameters may confuse this
        SimpleTypeExchanger exchangeTarget(target, AbstractType::Ptr());

        AbstractType::Ptr exchanged = exchangeTarget.exchange(type);

        if (exchanged) {
            QString typeSuffixString = exchanged->toString();
            static const QRegularExpression suffixExp(QStringLiteral("&|\\*"));
            int suffixPos = typeSuffixString.indexOf(suffixExp);
            if (suffixPos != -1)
                modifyHtml() += typeHighlight(typeSuffixString.mid(suffixPos));
        }
    } else {
        if (idType) {
            qCDebug(LANGUAGE) << "identified type could not be resolved:" << idType->qualifiedIdentifier() <<
                idType->declarationId().isValid() << idType->declarationId().isDirect();
        }
        modifyHtml() += typeHighlight(type->toString().toHtmlEscaped());
    }
}

DeclarationPointer AbstractDeclarationNavigationContext::declaration() const
{
    Q_D(const AbstractDeclarationNavigationContext);

    return d->m_declaration;
}

QString AbstractDeclarationNavigationContext::identifierHighlight(const QString& identifier,
                                                                  const DeclarationPointer& decl) const
{
    QString ret = nameHighlight(identifier);
    if (!decl) {
        return ret;
    }

    if (decl->isDeprecated()) {
        ret = QLatin1String("<i>") + ret + QLatin1String("</i>");
    }
    return ret;
}

QString AbstractDeclarationNavigationContext::stringFromAccess(Declaration::AccessPolicy access)
{
    switch (access) {
    case Declaration::Private:
        return QStringLiteral("private");
    case Declaration::Protected:
        return QStringLiteral("protected");
    case Declaration::Public:
        return QStringLiteral("public");
    default:
        break;
    }
    return QString();
}

QString AbstractDeclarationNavigationContext::stringFromAccess(const DeclarationPointer& decl)
{
    const auto* memberDecl = dynamic_cast<const ClassMemberDeclaration*>(decl.data());
    if (memberDecl) {
        return stringFromAccess(memberDecl->accessPolicy());
    }
    return QString();
}

QString AbstractDeclarationNavigationContext::declarationName(const DeclarationPointer& decl) const
{
    if (auto* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data())) {
        if (alias->identifier().isEmpty())
            return QLatin1String("using namespace ") + alias->importIdentifier().toString();
        else
            return QLatin1String("namespace ") + alias->identifier().toString() + QLatin1String(" = ") +
                   alias->importIdentifier().toString();
    }

    if (!decl)
        return i18nc("A declaration that is unknown", "Unknown");
    else
        return prettyIdentifier(decl).toString();
}

QStringList AbstractDeclarationNavigationContext::declarationDetails(const DeclarationPointer& decl)
{
    QStringList details;
    const auto* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl.data());
    const auto* memberDecl = dynamic_cast<const ClassMemberDeclaration*>(decl.data());
    if (memberDecl) {
        if (memberDecl->isMutable())
            details << QStringLiteral("mutable");
        if (memberDecl->isRegister())
            details << QStringLiteral("register");
        if (memberDecl->isStatic())
            details << QStringLiteral("static");
        if (memberDecl->isAuto())
            details << QStringLiteral("auto");
        if (memberDecl->isExtern())
            details << QStringLiteral("extern");
        if (memberDecl->isFriend())
            details << QStringLiteral("friend");
    }

    if (decl->isDefinition())
        details << i18nc("tells if a declaration is defining the variable's value", "definition");
    if (decl->isExplicitlyDeleted())
        details << QStringLiteral("deleted");

    if (memberDecl && memberDecl->isForwardDeclaration())
        details << i18nc("as in c++ forward declaration", "forward");

    AbstractType::Ptr t(decl->abstractType());
    if (t) {
        if (t->modifiers() & AbstractType::ConstModifier)
            details << i18nc("a variable that won't change, const", "constant");
        if (t->modifiers() & AbstractType::VolatileModifier)
            details << QStringLiteral("volatile");
        if (t->modifiers() & AbstractType::AtomicModifier)
            details << QStringLiteral("_Atomic");
    }
    if (function) {
        if (function->isInline())
            details << QStringLiteral("inline");
        if (function->isExplicit())
            details << QStringLiteral("explicit");
        if (function->isVirtual())
            details << QStringLiteral("virtual");

        const auto* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(decl.data());
        if (classFunDecl) {
            if (classFunDecl->isSignal())
                details << QStringLiteral("signal");
            if (classFunDecl->isSlot())
                details << QStringLiteral("slot");
            if (classFunDecl->isFinal())
                details << QStringLiteral("final");
            if (classFunDecl->isConstructor())
                details << QStringLiteral("constructor");
            if (classFunDecl->isDestructor())
                details << QStringLiteral("destructor");
            if (classFunDecl->isConversionFunction())
                details << QStringLiteral("conversion-function");
            if (classFunDecl->isAbstract())
                details << QStringLiteral("abstract");
        }
    }

    return details;
}

QString AbstractDeclarationNavigationContext::declarationSizeInformation(const DeclarationPointer& decl,
                                                                         const TopDUContext* topContext)
{
    if (!decl) {
        return {};
    }
    const auto type = TypeUtils::unAliasedType(decl->abstractType());
    if (!type) {
        return {};
    }

    if (type->sizeOf() > 0 || type->alignOf() > 0) {
        QString sizeInfo = QStringLiteral("<p>");
        const auto memberDecl = decl.dynamicCast<ClassMemberDeclaration>();
        if (memberDecl && memberDecl->bitOffsetOf() > 0) {
            const auto byteOffset = memberDecl->bitOffsetOf() / 8;
            const auto bitOffset = memberDecl->bitOffsetOf() % 8;
            const QString byteOffsetStr = i18np("1 Byte", "%1 Bytes", byteOffset);
            const QString bitOffsetStr = bitOffset ? i18np("1 Bit", "%1 Bits", bitOffset) : QString();
            sizeInfo
                += i18n("offset in parent: %1",
                        bitOffset ? i18nc("%1: bytes, %2: bits", "%1, %2", byteOffsetStr, bitOffsetStr) : byteOffsetStr)
                + QLatin1String("; ");
        }

        if (type->sizeOf() > 0) {
            sizeInfo += i18n("size: %1 Bytes", type->sizeOf()) + QLatin1String("; ");
        }

        if (type->alignOf() > 0) {
            sizeInfo += i18n("aligned to: %1 Bytes", type->alignOf());
        }

        sizeInfo += QLatin1String("</p>");
        return sizeInfo;
    } else if (decl->isTypeAlias()) {
        // show size information for underlying type of aliases / typedefs etc.
        if (const auto* idType = dynamic_cast<const IdentifiedType*>(type.data())) {
            DeclarationPointer ptr(idType->declaration(topContext));
            if (ptr != decl) {
                return declarationSizeInformation(ptr, topContext);
            }
        }
    }
    return {};
}
}

#include "moc_abstractdeclarationnavigationcontext.cpp"
