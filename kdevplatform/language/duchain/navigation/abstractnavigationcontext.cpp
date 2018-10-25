/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "abstractnavigationcontext.h"

#include <KLocalizedString>

#include <qtcompat_p.h>

#include "abstractdeclarationnavigationcontext.h"
#include "abstractnavigationwidget.h"
#include "usesnavigationcontext.h"
#include "../../../interfaces/icore.h"
#include "../../../interfaces/idocumentcontroller.h"
#include "../functiondeclaration.h"
#include "../namespacealiasdeclaration.h"
#include "../types/functiontype.h"
#include "../types/structuretype.h"
#include <debug.h>
#include <interfaces/icontextbrowser.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>

namespace KDevelop {
class AbstractNavigationContextPrivate
{
public:
    QVector<NavigationContextPointer> m_children; //Used to keep alive all children until this is deleted

    int m_selectedLink = 0; //The link currently selected
    NavigationAction m_selectedLinkAction; //Target of the currently selected link

    bool m_shorten = false;

    //A counter used while building the html-code to count the used links.
    int m_linkCount = -1;
    //Something else than -1 if the current position is represented by a line-number, not a link.
    int m_currentLine = 0;
    int m_currentPositionLine = 0;
    QMap<QString, NavigationAction> m_links;
    QMap<int, int> m_linkLines; //Holds the line for each link
    QMap<int, NavigationAction> m_intLinks;
    AbstractNavigationContext* m_previousContext;
    QString m_prefix, m_suffix;
    TopDUContextPointer m_topContext;

    QString m_currentText; //Here the text is built
};

void AbstractNavigationContext::setTopContext(const TopDUContextPointer& context)
{
    d->m_topContext = context;
}

TopDUContextPointer AbstractNavigationContext::topContext() const
{
    return d->m_topContext;
}

AbstractNavigationContext::AbstractNavigationContext(const TopDUContextPointer& topContext,
                                                     AbstractNavigationContext* previousContext)
    : d(new AbstractNavigationContextPrivate)
{
    d->m_previousContext = previousContext;
    d->m_topContext = topContext;

    qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");
    qRegisterMetaType<IDocumentation::Ptr>("IDocumentation::Ptr");
}

AbstractNavigationContext::~AbstractNavigationContext()
{
}

void AbstractNavigationContext::addExternalHtml(const QString& text)
{
    int lastPos = 0;
    int pos = 0;
    QString fileMark = QStringLiteral("KDEV_FILE_LINK{");
    while (pos < text.length() && (pos = text.indexOf(fileMark, pos)) != -1) {
        modifyHtml() +=  text.mid(lastPos, pos - lastPos);

        pos += fileMark.length();

        if (pos != text.length()) {
            int fileEnd = text.indexOf(QLatin1Char('}'), pos);
            if (fileEnd != -1) {
                QString file = text.mid(pos, fileEnd - pos);
                pos = fileEnd + 1;
                const QUrl url = QUrl::fromUserInput(file);
                makeLink(url.fileName(), file, NavigationAction(url, KTextEditor::Cursor()));
            }
        }

        lastPos = pos;
    }

    modifyHtml() +=  text.mid(lastPos, text.length() - lastPos);
}

void AbstractNavigationContext::makeLink(const QString& name, const DeclarationPointer& declaration,
                                         NavigationAction::Type actionType)
{
    NavigationAction action(declaration, actionType);
    makeLink(name, QString(), action);
}

QString AbstractNavigationContext::createLink(const QString& name, const QString&, const NavigationAction& action)
{
    if (d->m_shorten) {
        //Do not create links in shortened mode, it's only for viewing
        return typeHighlight(name.toHtmlEscaped());
    }

    // NOTE: Since the by definition in the HTML standard some uri components
    //       are case-insensitive, we define a new lowercase link-id for each
    //       link. Otherwise Qt 5 seems to mess up the casing and the link
    //       cannot be matched when it's executed.
    QString hrefId = QStringLiteral("link_%1").arg(d->m_links.count());

    d->m_links[hrefId] = action;
    d->m_intLinks[d->m_linkCount] = action;
    d->m_linkLines[d->m_linkCount] = d->m_currentLine;
    if (d->m_currentPositionLine == d->m_currentLine) {
        d->m_currentPositionLine = -1;
        d->m_selectedLink = d->m_linkCount;
    }

    QString str = name.toHtmlEscaped();
    if (d->m_linkCount == d->m_selectedLink)
        str = QLatin1String("<font style=\"background-color:#f1f1f1;\" color=\"#880088\">") + str + QLatin1String(
            "</font>");

    QString ret = QLatin1String("<a href=\"") + hrefId + QLatin1Char('\"') +
                  ((d->m_linkCount == d->m_selectedLink &&
                    d->m_currentPositionLine ==
                    -1) ? QStringLiteral(" name = \"currentPosition\"") : QString()) + QLatin1Char('>') + str +
                  QLatin1String("</a>");

    if (d->m_selectedLink == d->m_linkCount)
        d->m_selectedLinkAction = action;

    ++d->m_linkCount;
    return ret;
}

void AbstractNavigationContext::makeLink(const QString& name, const QString& targetId, const NavigationAction& action)
{
    modifyHtml() += createLink(name, targetId, action);
}

void AbstractNavigationContext::clear()
{
    d->m_linkCount = 0;
    d->m_currentLine = 0;
    d->m_currentText.clear();
    d->m_links.clear();
    d->m_intLinks.clear();
    d->m_linkLines.clear();
}

void AbstractNavigationContext::executeLink(const QString& link)
{
    if (!d->m_links.contains(link))
        return;

    execute(d->m_links[link]);
}

NavigationContextPointer AbstractNavigationContext::executeKeyAction(const QString& key)
{
    Q_UNUSED(key);
    return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::execute(const NavigationAction& action)
{
    if (action.targetContext)
        return NavigationContextPointer(action.targetContext);

    if (action.type == NavigationAction::ExecuteKey)
        return executeKeyAction(action.key);

    if (!action.decl && (action.type != NavigationAction::JumpToSource || action.document.isEmpty())) {
        qCDebug(LANGUAGE) << "Navigation-action has invalid declaration" << endl;
        return NavigationContextPointer(this);
    }

    switch (action.type) {
    case NavigationAction::ExecuteKey:
        break;
    case NavigationAction::None:
        qCDebug(LANGUAGE) << "Tried to execute an invalid action in navigation-widget" << endl;
        break;
    case NavigationAction::NavigateDeclaration:
    {
        auto ctx = dynamic_cast<AbstractDeclarationNavigationContext*>(d->m_previousContext);
        if (ctx && ctx->declaration() == action.decl)
            return NavigationContextPointer(d->m_previousContext);
        return registerChild(action.decl);
    }
    case NavigationAction::NavigateUses:
    {
        IContextBrowser* browser = ICore::self()->pluginController()->extensionForPlugin<IContextBrowser>();
        if (browser) {
            browser->showUses(action.decl);
            return NavigationContextPointer(this);
        }
        Q_FALLTHROUGH();
    }
    case NavigationAction::ShowUses: {
        return registerChild(new UsesNavigationContext(action.decl.data(), this));
    }
    case NavigationAction::JumpToSource:
    {
        QUrl doc = action.document;
        KTextEditor::Cursor cursor = action.cursor;
        {
            DUChainReadLocker lock(DUChain::lock());
            if (action.decl) {
                if (doc.isEmpty()) {
                    doc = action.decl->url().toUrl();
                    /*          if(action.decl->internalContext())
                                cursor = action.decl->internalContext()->range().start() + KTextEditor::Cursor(0, 1);
                              else*/
                    cursor = action.decl->rangeInCurrentRevision().start();
                }

                action.decl->activateSpecialization();
            }
        }

        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
        QMetaObject::invokeMethod(ICore::self()->documentController(), "openDocument", Qt::QueuedConnection,
                                  Q_ARG(QUrl, doc), Q_ARG(KTextEditor::Cursor, cursor));
        break;
    }
    case NavigationAction::ShowDocumentation: {
        auto doc = ICore::self()->documentationController()->documentationForDeclaration(action.decl.data());
        // This is used to execute the slot delayed in the event-loop, so crashes are avoided
        // which can happen e.g. due to focus change events resulting in tooltip destruction and thus this object
        QMetaObject::invokeMethod(
            ICore::self()->documentationController(), "showDocumentation", Qt::QueuedConnection,
            Q_ARG(IDocumentation::Ptr, doc));
    }
    break;
    }

    return NavigationContextPointer(this);
}

AbstractNavigationContext* AbstractNavigationContext::previousContext() const
{
    return d->m_previousContext;
}

void AbstractNavigationContext::setPreviousContext(AbstractNavigationContext* previous)
{
    d->m_previousContext = previous;
}

NavigationContextPointer AbstractNavigationContext::registerChild(AbstractNavigationContext* context)
{
    d->m_children << NavigationContextPointer(context);
    return d->m_children.last();
}

NavigationContextPointer AbstractNavigationContext::registerChild(const DeclarationPointer& declaration)
{
    //We create a navigation-widget here, and steal its context.. evil ;)
    QScopedPointer<QWidget> navigationWidget(declaration->context()->createNavigationWidget(declaration.data()));
    if (AbstractNavigationWidget* abstractNavigationWidget =
            dynamic_cast<AbstractNavigationWidget*>(navigationWidget.data())) {
        NavigationContextPointer ret = abstractNavigationWidget->context();
        ret->setPreviousContext(this);
        d->m_children << ret;
        return ret;
    } else {
        return NavigationContextPointer(this);
    }
}

const int lineJump = 3;

void AbstractNavigationContext::down()
{
    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    int fromLine = d->m_currentPositionLine;

    if (d->m_selectedLink >= 0 && d->m_selectedLink < d->m_linkCount) {
        if (fromLine == -1)
            fromLine = d->m_linkLines[d->m_selectedLink];

        for (int newSelectedLink = d->m_selectedLink + 1; newSelectedLink < d->m_linkCount; ++newSelectedLink) {
            if (d->m_linkLines[newSelectedLink] > fromLine && d->m_linkLines[newSelectedLink] - fromLine <= lineJump) {
                d->m_selectedLink = newSelectedLink;
                d->m_currentPositionLine = -1;
                return;
            }
        }
    }
    if (fromLine == -1)
        fromLine = 0;

    d->m_currentPositionLine = fromLine + lineJump;

    if (d->m_currentPositionLine > d->m_currentLine)
        d->m_currentPositionLine = d->m_currentLine;
}

void AbstractNavigationContext::up()
{
    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    int fromLine = d->m_currentPositionLine;

    if (d->m_selectedLink >= 0 && d->m_selectedLink < d->m_linkCount) {
        if (fromLine == -1)
            fromLine = d->m_linkLines[d->m_selectedLink];

        for (int newSelectedLink = d->m_selectedLink - 1; newSelectedLink >= 0; --newSelectedLink) {
            if (d->m_linkLines[newSelectedLink] < fromLine && fromLine - d->m_linkLines[newSelectedLink] <= lineJump) {
                d->m_selectedLink = newSelectedLink;
                d->m_currentPositionLine = -1;
                return;
            }
        }
    }

    if (fromLine == -1)
        fromLine = d->m_currentLine;

    d->m_currentPositionLine = fromLine - lineJump;
    if (d->m_currentPositionLine < 0)
        d->m_currentPositionLine = 0;
}

void AbstractNavigationContext::nextLink()
{
    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    d->m_currentPositionLine = -1;

    if (d->m_linkCount > 0)
        d->m_selectedLink = (d->m_selectedLink + 1) % d->m_linkCount;
}

void AbstractNavigationContext::previousLink()
{
    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    d->m_currentPositionLine = -1;

    if (d->m_linkCount > 0) {
        --d->m_selectedLink;
        if (d->m_selectedLink <  0)
            d->m_selectedLink += d->m_linkCount;
    }

    Q_ASSERT(d->m_selectedLink >= 0);
}

int AbstractNavigationContext::linkCount() const
{
    return d->m_linkCount;
}

QString AbstractNavigationContext::prefix() const
{
    return d->m_prefix;
}

QString AbstractNavigationContext::suffix() const
{
    return d->m_suffix;
}

void AbstractNavigationContext::setPrefixSuffix(const QString& prefix, const QString& suffix)
{
    d->m_prefix = prefix;
    d->m_suffix = suffix;
}

NavigationContextPointer AbstractNavigationContext::back()
{
    if (d->m_previousContext)
        return NavigationContextPointer(d->m_previousContext);
    else
        return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::accept()
{
    if (d->m_selectedLink >= 0 &&  d->m_selectedLink < d->m_linkCount) {
        NavigationAction action = d->m_intLinks[d->m_selectedLink];
        return execute(action);
    }
    return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::accept(IndexedDeclaration decl)
{
    if (decl.data()) {
        NavigationAction action(DeclarationPointer(decl.data()), NavigationAction::NavigateDeclaration);
        return execute(action);
    } else {
        return NavigationContextPointer(this);
    }
}

NavigationContextPointer AbstractNavigationContext::acceptLink(const QString& link)
{
    if (!d->m_links.contains(link)) {
        qCDebug(LANGUAGE) << "Executed unregistered link " << link << endl;
        return NavigationContextPointer(this);
    }

    return execute(d->m_links[link]);
}

NavigationAction AbstractNavigationContext::currentAction() const
{
    return d->m_selectedLinkAction;
}

QString AbstractNavigationContext::declarationKind(const DeclarationPointer& decl)
{
    const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl.data());

    QString kind;

    if (decl->isTypeAlias())
        kind = i18n("Typedef");
    else if (decl->kind() == Declaration::Type) {
        if (decl->type<StructureType>())
            kind = i18n("Class");
    } else if (decl->kind() == Declaration::Instance) {
        kind = i18n("Variable");
    } else if (decl->kind() == Declaration::Namespace) {
        kind = i18n("Namespace");
    }

    if (NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data())) {
        if (alias->identifier().isEmpty())
            kind = i18n("Namespace import");
        else
            kind = i18n("Namespace alias");
    }

    if (function)
        kind = i18n("Function");

    if (decl->isForwardDeclaration())
        kind = i18n("Forward Declaration");

    return kind;
}

QString AbstractNavigationContext::html(bool shorten)
{
    d->m_shorten = shorten;
    return QString();
}

bool AbstractNavigationContext::alreadyComputed() const
{
    return !d->m_currentText.isEmpty();
}

bool AbstractNavigationContext::isWidgetMaximized() const
{
    return true;
}

QWidget* AbstractNavigationContext::widget() const
{
    return nullptr;
}

///Splits the string by the given regular expression, but keeps the split-matches at the end of each line
static QStringList splitAndKeep(QString str, const QRegExp& regExp)
{
    QStringList ret;
    int place = regExp.indexIn(str);
    while (place != -1) {
        ret << str.left(place + regExp.matchedLength());
        str.remove(0, place + regExp.matchedLength());
        place = regExp.indexIn(str);
    }
    ret << str;
    return ret;
}

void AbstractNavigationContext::addHtml(const QString& html)
{
    QRegExp newLineRegExp(QStringLiteral("<br>|<br */>"));
    foreach (const QString& line, splitAndKeep(html, newLineRegExp)) {
        d->m_currentText +=  line;
        if (line.indexOf(newLineRegExp) != -1) {
            ++d->m_currentLine;
            if (d->m_currentLine == d->m_currentPositionLine) {
                d->m_currentText += QStringLiteral(
                    "<font color=\"#880088\"> <a name = \"currentPosition\" >&lt;-&gt;</a> </font>");                        // >&lt;-&gt; is <->
            }
        }
    }
}

QString AbstractNavigationContext::currentHtml() const
{
    return d->m_currentText;
}

QString AbstractNavigationContext::fontSizePrefix(bool /*shorten*/) const
{
    return QString();
}

QString AbstractNavigationContext::fontSizeSuffix(bool /*shorten*/) const
{
    return QString();
}

QString Colorizer::operator()(const QString& str) const
{
    QString ret = QLatin1String("<font color=\"#") + m_color + QLatin1String("\">") + str + QLatin1String("</font>");

    if (m_formatting & Fixed)
        ret = QLatin1String("<tt>") + ret + QLatin1String("</tt>");
    if (m_formatting & Bold)
        ret = QLatin1String("<b>") + ret + QLatin1String("</b>");
    if (m_formatting & Italic)
        ret = QLatin1String("<i>") + ret + QLatin1String("</i>");

    return ret;
}

const Colorizer AbstractNavigationContext::typeHighlight(QStringLiteral("006000"));
const Colorizer AbstractNavigationContext::errorHighlight(QStringLiteral("990000"));
const Colorizer AbstractNavigationContext::labelHighlight(QStringLiteral("000000"));
const Colorizer AbstractNavigationContext::codeHighlight(QStringLiteral("005000"));
const Colorizer AbstractNavigationContext::propertyHighlight(QStringLiteral("009900"));
const Colorizer AbstractNavigationContext::navigationHighlight(QStringLiteral("000099"));
const Colorizer AbstractNavigationContext::importantHighlight(QStringLiteral(
        "000000"), Colorizer::Bold | Colorizer::Italic);
const Colorizer AbstractNavigationContext::commentHighlight(QStringLiteral("303030"));
const Colorizer AbstractNavigationContext::nameHighlight(QStringLiteral("000000"), Colorizer::Bold);
}
