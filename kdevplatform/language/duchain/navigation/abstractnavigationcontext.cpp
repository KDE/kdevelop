/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "abstractnavigationcontext.h"

#include <KColorScheme>
#include <KLocalizedString>

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

#include <QGuiApplication>
#include <QRegExp>

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
    TopDUContextPointer m_topContext;

    QString m_currentText; //Here the text is built
};

void AbstractNavigationContext::setTopContext(const TopDUContextPointer& context)
{
    Q_D(AbstractNavigationContext);

    d->m_topContext = context;
}

TopDUContextPointer AbstractNavigationContext::topContext() const
{
    Q_D(const AbstractNavigationContext);

    return d->m_topContext;
}

AbstractNavigationContext::AbstractNavigationContext(const TopDUContextPointer& topContext,
                                                     AbstractNavigationContext* previousContext)
    : d_ptr(new AbstractNavigationContextPrivate)
{
    Q_D(AbstractNavigationContext);

    d->m_previousContext = previousContext;
    d->m_topContext = topContext;

    qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");
    qRegisterMetaType<IDocumentation::Ptr>("IDocumentation::Ptr");
}

AbstractNavigationContext::~AbstractNavigationContext()
{
}

void AbstractNavigationContext::makeLink(const QString& name, const DeclarationPointer& declaration,
                                         NavigationAction::Type actionType)
{
    NavigationAction action(declaration, actionType);
    makeLink(name, QString(), action);
}

QString AbstractNavigationContext::createLink(const QString& name, const QString&, const NavigationAction& action)
{
    Q_D(AbstractNavigationContext);

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
        str = QLatin1String("<font color=\"#880088\">") + str + QLatin1String("</font>");

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
    Q_D(AbstractNavigationContext);

    d->m_linkCount = 0;
    d->m_currentLine = 0;
    d->m_currentText.clear();
    d->m_links.clear();
    d->m_intLinks.clear();
    d->m_linkLines.clear();
}

void AbstractNavigationContext::executeLink(const QString& link)
{
    Q_D(AbstractNavigationContext);

    const auto actionIt = d->m_links.constFind(link);
    if (actionIt == d->m_links.constEnd())
        return;

    execute(*actionIt);
}

NavigationContextPointer AbstractNavigationContext::executeKeyAction(const QString& key)
{
    Q_UNUSED(key);
    return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::execute(const NavigationAction& action)
{
    Q_D(AbstractNavigationContext);

    if (action.targetContext)
        return NavigationContextPointer(action.targetContext);

    if (action.type == NavigationAction::ExecuteKey)
        return executeKeyAction(action.key);

    if (!action.decl && (action.type != NavigationAction::JumpToSource || action.document.isEmpty())) {
        qCDebug(LANGUAGE) << "Navigation-action has invalid declaration";
        return NavigationContextPointer(this);
    }

    switch (action.type) {
    case NavigationAction::ExecuteKey:
        break;
    case NavigationAction::None:
        qCDebug(LANGUAGE) << "Tried to execute an invalid action in navigation-widget";
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
        auto* browser = ICore::self()->pluginController()->extensionForPlugin<IContextBrowser>();
        if (browser) {
            browser->showUses(action.decl);
            return NavigationContextPointer(this);
        }
        [[fallthrough]];
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
    Q_D(const AbstractNavigationContext);

    return d->m_previousContext;
}

void AbstractNavigationContext::setPreviousContext(AbstractNavigationContext* previous)
{
    Q_D(AbstractNavigationContext);

    d->m_previousContext = previous;
}

NavigationContextPointer AbstractNavigationContext::registerChild(AbstractNavigationContext* context)
{
    Q_D(AbstractNavigationContext);

    d->m_children << NavigationContextPointer(context);
    return d->m_children.last();
}

NavigationContextPointer AbstractNavigationContext::registerChild(const DeclarationPointer& declaration)
{
    Q_D(AbstractNavigationContext);

    // Lock the DUChain for reading, since the createNavigationWidget
    // method will eventually need read access to the DUChain and
    // does not lock it itself.
    DUChainReadLocker lock;

    // Check that declaration and its context are still valid
    // (they might have been deleted while we were waiting for the lock)
    if (!declaration || !declaration->context()) {
        return NavigationContextPointer(this);
    }

    //We create a navigation-widget here, and steal its context.. evil ;)
    QScopedPointer<AbstractNavigationWidget> navigationWidget(
        declaration->context()->createNavigationWidget(declaration.data()));
    if (navigationWidget) {
        NavigationContextPointer ret = navigationWidget->context();
        ret->setPreviousContext(this);
        d->m_children << ret;
        return ret;
    } else {
        return NavigationContextPointer(this);
    }
}

const int lineJump = 3;

bool AbstractNavigationContext::down()
{
    Q_D(AbstractNavigationContext);

    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    // select first link when we enter via down
    if (d->m_selectedLink == -1 && d->m_linkCount) {
        d->m_selectedLink = 0;
        d->m_currentPositionLine = -1;
        return true;
    }

    int fromLine = d->m_currentPositionLine;

    // try to select the next link within our lineJump distance
    if (d->m_selectedLink >= 0 && d->m_selectedLink < d->m_linkCount) {
        if (fromLine == -1)
            fromLine = d->m_linkLines[d->m_selectedLink];

        for (int newSelectedLink = d->m_selectedLink + 1; newSelectedLink < d->m_linkCount; ++newSelectedLink) {
            if (d->m_linkLines[newSelectedLink] > fromLine && d->m_linkLines[newSelectedLink] - fromLine <= lineJump) {
                d->m_selectedLink = newSelectedLink;
                d->m_currentPositionLine = -1;
                return true;
            }
        }
    }

    if (fromLine == d->m_currentLine - 1) // nothing to do, we are at the end of the document
        return false;

    // scroll down by applying the lineJump
    if (fromLine == -1)
        fromLine = 0;

    d->m_currentPositionLine = fromLine + lineJump;

    if (d->m_currentPositionLine >= d->m_currentLine) {
        d->m_currentPositionLine = d->m_currentLine - 1;
    }
    return fromLine != d->m_currentPositionLine;
}

bool AbstractNavigationContext::up()
{
    Q_D(AbstractNavigationContext);

    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    // select last link when we enter via up
    if (d->m_selectedLink == -1 && d->m_linkCount) {
        d->m_selectedLink = d->m_linkCount - 1;
        d->m_currentPositionLine = -1;
        return true;
    }

    int fromLine = d->m_currentPositionLine;

    if (d->m_selectedLink >= 0 && d->m_selectedLink < d->m_linkCount) {
        if (fromLine == -1)
            fromLine = d->m_linkLines[d->m_selectedLink];

        for (int newSelectedLink = d->m_selectedLink - 1; newSelectedLink >= 0; --newSelectedLink) {
            if (d->m_linkLines[newSelectedLink] < fromLine && fromLine - d->m_linkLines[newSelectedLink] <= lineJump) {
                d->m_selectedLink = newSelectedLink;
                d->m_currentPositionLine = -1;
                return true;
            }
        }
    }

    if (fromLine == -1)
        fromLine = d->m_currentLine - 1;

    d->m_currentPositionLine = fromLine - lineJump;
    if (d->m_currentPositionLine < 0)
        d->m_currentPositionLine = 0;

    return fromLine || d->m_currentPositionLine;
}

bool AbstractNavigationContext::nextLink()
{
    Q_D(AbstractNavigationContext);

    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    if (!d->m_linkCount)
        return false;

    d->m_currentPositionLine = -1;

    d->m_selectedLink++;
    if (d->m_selectedLink >= d->m_linkCount) {
        d->m_selectedLink = 0;
        return false;
    }
    return true;
}

bool AbstractNavigationContext::previousLink()
{
    Q_D(AbstractNavigationContext);

    //Make sure link-count is valid
    if (d->m_linkCount == -1) {
        DUChainReadLocker lock;
        html();
    }

    if (!d->m_linkCount)
        return false;

    d->m_currentPositionLine = -1;

    d->m_selectedLink--;
    if (d->m_selectedLink < 0) {
        d->m_selectedLink = d->m_linkCount - 1;
        return false;
    }
    return true;
}

int AbstractNavigationContext::linkCount() const
{
    Q_D(const AbstractNavigationContext);

    return d->m_linkCount;
}

void AbstractNavigationContext::resetNavigation()
{
    Q_D(AbstractNavigationContext);

    d->m_currentPositionLine = -1;
    d->m_selectedLink = -1;
    d->m_selectedLinkAction = {};
}

NavigationContextPointer AbstractNavigationContext::back()
{
    Q_D(AbstractNavigationContext);

    if (d->m_previousContext)
        return NavigationContextPointer(d->m_previousContext);
    else
        return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::accept()
{
    Q_D(AbstractNavigationContext);

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
    Q_D(AbstractNavigationContext);

    const auto actionIt = d->m_links.constFind(link);
    if (actionIt == d->m_links.constEnd()) {
        qCDebug(LANGUAGE) << "Executed unregistered link " << link;
        return NavigationContextPointer(this);
    }

    return execute(*actionIt);
}

NavigationAction AbstractNavigationContext::currentAction() const
{
    Q_D(const AbstractNavigationContext);

    return d->m_selectedLinkAction;
}

QString AbstractNavigationContext::declarationKind(const DeclarationPointer& decl)
{
    const auto* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl.data());

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

    if (auto* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data())) {
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
    Q_D(AbstractNavigationContext);

    d->m_shorten = shorten;
    return QString();
}

bool AbstractNavigationContext::alreadyComputed() const
{
    Q_D(const AbstractNavigationContext);

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
    Q_D(AbstractNavigationContext);

    QRegExp newLineRegExp(QStringLiteral("<br>|<br */>|</p>"));
    const auto lines = splitAndKeep(html, newLineRegExp);
    for (const QString& line : lines) {
        d->m_currentText +=  line;
        if (newLineRegExp.indexIn(line) != -1) {
            ++d->m_currentLine;
            if (d->m_currentLine == d->m_currentPositionLine) {
                d->m_currentText += QLatin1String(
                    "<font color=\"#880088\"> <a name = \"currentPosition\" >&lt;-&gt;</a> </font>");                        // >&lt;-&gt; is <->
            }
        }
    }
}

QString AbstractNavigationContext::currentHtml() const
{
    Q_D(const AbstractNavigationContext);

    return d->m_currentText;
}

/** A helper-class for elegant colorization of html-strings .
 *
 * Initialize it with a html-color like "990000". and colorize strings
 * using operator()
 */
struct KDEVPLATFORMLANGUAGE_EXPORT Colorizer : public QObject
{
    enum FormattingFlag {
        Nothing = 0x0,
        Bold = 0x1,
        Italic = 0x2,
        Fixed = 0x4
    };
    Q_DECLARE_FLAGS(Formatting, FormattingFlag)
    explicit Colorizer(KColorScheme::ForegroundRole paletteColor, Formatting formatting = Nothing)
        : m_foreground(paletteColor)
        , m_formatting(formatting)
    {
        init();
        QCoreApplication::instance()->installEventFilter(this);
    }

    void init()
    {
        const auto colors = KColorScheme(QPalette::Normal, KColorScheme::View);
        m_color = colors.foreground(m_foreground).color().name();
    }

    bool eventFilter(QObject* watched, QEvent* event)
    {
        if (watched == QCoreApplication::instance() && event->type() == QEvent::ApplicationPaletteChange) {
            init();
        }
        return false;
    }
    QString operator()(const QString& str) const
    {
        QString ret = QLatin1String("<font color=\"") + m_color + QLatin1String("\">") + str + QLatin1String("</font>");

        if (m_formatting & Fixed)
            ret = QLatin1String("<tt>") + ret + QLatin1String("</tt>");
        if (m_formatting & Bold)
            ret = QLatin1String("<b>") + ret + QLatin1String("</b>");
        if (m_formatting & Italic)
            ret = QLatin1String("<i>") + ret + QLatin1String("</i>");

        return ret;
    }

private:
    const KColorScheme::ForegroundRole m_foreground;
    const Formatting m_formatting;
    QString m_color;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Colorizer::Formatting)

QString AbstractNavigationContext::typeHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::PositiveText);
    return c(str);
}
QString AbstractNavigationContext::errorHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NegativeText);
    return c(str);
}
QString AbstractNavigationContext::labelHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NormalText);
    return c(str);
}
QString AbstractNavigationContext::codeHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NeutralText);
    return c(str);
}
QString AbstractNavigationContext::propertyHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::PositiveText);
    return c(str);
}
QString AbstractNavigationContext::navigationHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::LinkText);
    return c(str);
}
QString AbstractNavigationContext::importantHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NormalText, Colorizer::Bold | Colorizer::Italic);
    return c(str);
}
QString AbstractNavigationContext::commentHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NormalText, Colorizer::Italic);
    return c(str);
}
QString AbstractNavigationContext::nameHighlight(const QString& str)
{
    static const Colorizer c(KColorScheme::NormalText, Colorizer::Bold);
    return c(str);
}
}

#include "moc_abstractnavigationcontext.cpp"
