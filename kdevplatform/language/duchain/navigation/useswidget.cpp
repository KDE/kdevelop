/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "useswidget.h"
#include <debug.h>

#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/uses.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>

#include <language/duchain/duchainutils.h>
#include <language/codegen/coderepresentation.h>
#include <interfaces/iproject.h>
#include <util/foregroundlock.h>

using namespace KDevelop;

namespace {
QString backgroundColor(bool isHighlighted)
{
    if (isHighlighted) {
        return QColor(251, 150, 242).name();
    } else {
        return QColor(251, 250, 150).name();
    }
}
}

const int tooltipContextSize = 2; //How many lines around the use are shown in the tooltip

///The returned text is fully escaped
///@param cutOff The total count of characters that should be cut of, all in all on both sides together.
///@param range The range that is highlighted, and that will be preserved during cutting, given that there is enough room beside it.
QString highlightAndEscapeUseText(QString line, int cutOff, KTextEditor::Range range)
{
    int leftCutRoom = range.start().column();
    int rightCutRoom = line.length() - range.end().column();

    if (range.start().column() < 0 || range.end().column() > line.length() || cutOff > leftCutRoom + rightCutRoom)
        return QString(); //Not enough room for cutting off on sides

    int leftCut = 0;
    int rightCut = 0;

    if (leftCutRoom < rightCutRoom) {
        if (leftCutRoom * 2 >= cutOff) {
            //Enough room on both sides. Just cut.
            leftCut = cutOff / 2;
            rightCut = cutOff - leftCut;
        } else {
            //Not enough room in left side, but enough room all together
            leftCut = leftCutRoom;
            rightCut = cutOff - leftCut;
        }
    } else {
        if (rightCutRoom * 2 >= cutOff) {
            //Enough room on both sides. Just cut.
            rightCut = cutOff / 2;
            leftCut = cutOff - rightCut;
        } else {
            //Not enough room in right side, but enough room all together
            rightCut = rightCutRoom;
            leftCut = cutOff - rightCut;
        }
    }
    Q_ASSERT(leftCut + rightCut <= cutOff);

    line.chop(rightCut);
    line.remove(0, leftCut);
    range += KTextEditor::Range(0, -leftCut, 0, -leftCut);

    Q_ASSERT(range.start().column() >= 0 && range.end().column() <= line.length());

    //TODO: share code with context browser
    // mixing (255, 255, 0, 100) with white yields this:
    const QColor foreground(0, 0, 0);

    return QLatin1String("<code>") + line.left(range.start().column()).toHtmlEscaped()
           + QLatin1String("<span style=\"background-color:") + backgroundColor(false) + QLatin1String(";color:") +
           foreground.name() + QLatin1String(";\">")
           + line.mid(range.start().column(), range.end().column() - range.start().column()).toHtmlEscaped()
           + QLatin1String("</span>") +
           line.mid(range.end().column(), line.length() - range.end().column()).toHtmlEscaped() + QLatin1String(
        "</code>");
}

/**
 * Note: the links in the HTML here are only used for styling
 * the navigation is implemented in the mouse press event handler
 */
OneUseWidget::OneUseWidget(IndexedDeclaration declaration, const IndexedString& document, KTextEditor::Range range,
                           const CodeRepresentation& code) : m_range(new PersistentMovingRange(range, document))
    , m_declaration(declaration)
    , m_document(document)
{
    //Make the sizing of this widget independent of the content, because we will adapt the content to the size
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    m_sourceLine = code.line(m_range->range().start().line());

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    setCursor(Qt::PointingHandCursor);

    m_label = new QLabel(this);
    m_icon = new QLabel(this);
    m_icon->setPixmap(QIcon::fromTheme(QStringLiteral("code-function")).pixmap(16));

    DUChainReadLocker lock(DUChain::lock());
    QString text = QLatin1String("<a>") + i18nc("refers to a line in source code", "Line <b>%1</b>:",
                                                range.start().line()) + QLatin1String("</a>");
    if (!m_sourceLine.isEmpty() && m_sourceLine.length() > m_range->range().end().column()) {
        text += QLatin1String("&nbsp;&nbsp;") + highlightAndEscapeUseText(m_sourceLine, 0, m_range->range());

        //Useful tooltip:
        int start = m_range->range().start().line() - tooltipContextSize;
        int end = m_range->range().end().line() + tooltipContextSize + 1;

        QStringList toolTipLines;
        for (int a = start; a < end; ++a) {
            QString lineText = code.line(a).toHtmlEscaped();
            if (m_range->range().start().line() <= a && m_range->range().end().line() >= a) {
                lineText = QLatin1String("<b>") + lineText + QLatin1String("</b>");
            }
            if (!lineText.trimmed().isEmpty()) {
                toolTipLines.append(lineText);
            }
        }

        setToolTip(QLatin1String("<html><body><pre>") + toolTipLines.join(QLatin1String("<br>")) +
                   QLatin1String("</pre></body></html>"));
    }
    m_label->setText(text);

    m_layout->addWidget(m_icon);
    m_layout->addWidget(m_label);
    m_layout->setAlignment(Qt::AlignLeft);
}

void OneUseWidget::setHighlighted(bool highlight)
{
    if (highlight == m_isHighlighted) {
        return;
    }

    if (highlight) {
        m_label->setText(m_label->text().replace(QLatin1String("background-color:") + backgroundColor(false),
                                                 QLatin1String("background-color:") + backgroundColor(true)));
        m_isHighlighted = true;
    } else {
        m_label->setText(m_label->text().replace(QLatin1String("background-color:") + backgroundColor(true),
                                                 QLatin1String("background-color:") + backgroundColor(false)));
        m_isHighlighted = false;
    }
}

bool KDevelop::OneUseWidget::isHighlighted() const
{
    return m_isHighlighted;
}

void OneUseWidget::activateLink()
{
    ICore::self()->documentController()->openDocument(m_document.toUrl(), m_range->range().start());
}

void OneUseWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !event->modifiers()) {
        activateLink();
        event->accept();
    }
}

OneUseWidget::~OneUseWidget()
{
}

void OneUseWidget::resizeEvent(QResizeEvent* event)
{
    ///Adapt the content
    QSize size = event->size();

    KTextEditor::Range range = m_range->range();

    int cutOff = 0;
    int maxCutOff = m_sourceLine.length() - (range.end().column() - range.start().column());

    //Reset so we also get more context while up-sizing
    m_label->setText(QLatin1String("<a>") +
                     i18nc("Refers to a line in source code", "Line <b>%1</b>:", range.start().line() + 1)
                     + QLatin1String("</a> ") + highlightAndEscapeUseText(m_sourceLine, cutOff, range));

    /// FIXME: this is incredibly ugly and slow... we could simply paint the text ourselves and elide it properly
    while (sizeHint().width() > size.width() && cutOff < maxCutOff) {
        //We've got to save space
        m_label->setText(QLatin1String("<a>") +
                         i18nc("Refers to a line in source code", "Line <b>%1</b>:", range.start().line() + 1)
                         + QLatin1String("</a> ") + highlightAndEscapeUseText(m_sourceLine, cutOff, range));
        cutOff += 5;
    }

    event->accept();

    QWidget::resizeEvent(event);
}

void NavigatableWidgetList::setShowHeader(bool show)
{
    if (show && !m_headerLayout->parent())
        m_layout->insertLayout(0, m_headerLayout);
    else
        m_headerLayout->setParent(nullptr);
}

NavigatableWidgetList::~NavigatableWidgetList()
{
    delete m_headerLayout;
}

NavigatableWidgetList::NavigatableWidgetList(bool allowScrolling, uint maxHeight, bool vertical)
    :  m_allowScrolling(allowScrolling)
{
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_layout->setSpacing(0);
    setBackgroundRole(QPalette::Base);
    m_useArrows = false;

    if (vertical)
        m_itemLayout = new QVBoxLayout;
    else
        m_itemLayout = new QHBoxLayout;
    m_itemLayout->setContentsMargins(0, 0, 0, 0);

    m_itemLayout->setContentsMargins(0, 0, 0, 0);
    m_itemLayout->setSpacing(0);
//   m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    setWidgetResizable(true);

    m_headerLayout = new QHBoxLayout;
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(0);

    if (m_useArrows) {
        auto previousButton = new QToolButton();
        previousButton->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));

        auto nextButton = new QToolButton();
        nextButton->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));

        m_headerLayout->addWidget(previousButton);
        m_headerLayout->addWidget(nextButton);
    }

    //hide these buttons for now, they're senseless

    m_layout->addLayout(m_headerLayout);

    auto* spaceLayout = new QHBoxLayout;
    spaceLayout->addSpacing(10);
    spaceLayout->addLayout(m_itemLayout);

    m_layout->addLayout(spaceLayout);

    if (maxHeight)
        setMaximumHeight(maxHeight);

    if (m_allowScrolling) {
        auto* contentsWidget = new QWidget;
        contentsWidget->setLayout(m_layout);
        setWidget(contentsWidget);
    } else {
        setLayout(m_layout);
    }
}

void NavigatableWidgetList::deleteItems()
{
    const auto items = this->items();
    qDeleteAll(items);

    Q_ASSERT(m_itemLayout->count() == 0);
    m_mediumPriorityPosition = 0;
}

void NavigatableWidgetList::addItem(QWidget* widget, ItemPriority priority)
{
    Q_ASSERT(m_mediumPriorityPosition >= 0);
    Q_ASSERT(m_mediumPriorityPosition <= m_itemLayout->count());

    switch (priority) {
    case ItemPriority::High:
        m_itemLayout->insertWidget(0, widget);
        ++m_mediumPriorityPosition;
        break;
    case ItemPriority::Medium:
        m_itemLayout->insertWidget(m_mediumPriorityPosition, widget);
        ++m_mediumPriorityPosition; // increment to insert later Medium-priority items after earlier ones
        break;
    case ItemPriority::Low:
        m_itemLayout->addWidget(widget);
        break;
    }
}

QList<QWidget*> NavigatableWidgetList::items() const
{
    QList<QWidget*> ret;
    for (int a = 0; a < m_itemLayout->count(); ++a) {
        auto* widgetItem = dynamic_cast<QWidgetItem*>(m_itemLayout->itemAt(a));
        if (widgetItem) {
            ret << widgetItem->widget();
        }
    }

    return ret;
}

bool NavigatableWidgetList::hasItems() const
{
    return ( bool )m_itemLayout->count();
}

void NavigatableWidgetList::addHeaderItem(QWidget* widget, Qt::Alignment alignment)
{
    if (m_useArrows) {
        Q_ASSERT(m_headerLayout->count() >= 2); //At least the 2 back/next buttons
        m_headerLayout->insertWidget(m_headerLayout->count() - 1, widget, alignment);
    } else {
        //We need to do this so the header doesn't get stretched
        widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_headerLayout->insertWidget(m_headerLayout->count(), widget, alignment);
//     widget->setMaximumHeight(20);
    }
}

///Returns whether the uses in the child should be a new uses-group
bool isNewGroup(DUContext* parent, DUContext* child)
{
    if (parent->type() == DUContext::Other && child->type() == DUContext::Other)
        return false;
    else
        return true;
}

uint countUses(int usedDeclarationIndex, DUContext* context)
{
    uint ret = 0;

    for (int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
        if (context->uses()[useIndex].m_declarationIndex == usedDeclarationIndex)
            ++ret;

    const auto childContexts = context->childContexts();
    for (DUContext* child : childContexts) {
        if (!isNewGroup(context, child))
            ret += countUses(usedDeclarationIndex, child);
    }

    return ret;
}

QList<OneUseWidget*> createUseWidgets(const CodeRepresentation& code, int usedDeclarationIndex, IndexedDeclaration decl,
                                      DUContext* context)
{
    QList<OneUseWidget*> ret;
    VERIFY_FOREGROUND_LOCKED

    for (int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
        if (context->uses()[useIndex].m_declarationIndex == usedDeclarationIndex)
            ret <<
                new OneUseWidget(decl, context->url(), context->transformFromLocalRevision(
                                     context->uses()[useIndex].m_range), code);

    const auto childContexts = context->childContexts();
    for (DUContext* child : childContexts) {
        if (!isNewGroup(context, child))
            ret += createUseWidgets(code, usedDeclarationIndex, decl, child);
    }

    return ret;
}

ContextUsesWidget::ContextUsesWidget(const CodeRepresentation& code, const QList<IndexedDeclaration>& usedDeclarations,
                                     IndexedDUContext context) : m_context(context)
{
    setFrameShape(NoFrame);

    DUChainReadLocker lock(DUChain::lock());
    QString headerText = i18n("Unknown context");
    setUpdatesEnabled(false);

    if (context.data()) {
        DUContext* ctx = context.data();

        if (ctx->scopeIdentifier(true).isEmpty())
            headerText = i18n("Global");
        else {
            headerText = ctx->scopeIdentifier(true).toString();
            if (ctx->type() == DUContext::Function || (ctx->owner() && ctx->owner()->isFunctionDeclaration()))
                headerText += QLatin1String("(...)");
        }

        QSet<int> hadIndices;

        for (const IndexedDeclaration usedDeclaration : usedDeclarations) {
            int usedDeclarationIndex = ctx->topContext()->indexForUsedDeclaration(usedDeclaration.data(), false);
            if (hadIndices.contains(usedDeclarationIndex))
                continue;

            hadIndices.insert(usedDeclarationIndex);

            if (usedDeclarationIndex != std::numeric_limits<int>::max()) {
                const auto useWidgets = createUseWidgets(code, usedDeclarationIndex, usedDeclaration, ctx);
                for (OneUseWidget* widget : useWidgets) {
                    addItem(widget);
                }
            }
        }
    }

    auto* headerLabel = new QLabel(i18nc("%1: source file", "In %1", QLatin1String("<a href='navigateToFunction'>")
                                           + headerText.toHtmlEscaped() + QLatin1String("</a>: ")));
    addHeaderItem(headerLabel);
    setUpdatesEnabled(true);
    connect(headerLabel, &QLabel::linkActivated, this, &ContextUsesWidget::linkWasActivated);
}

void ContextUsesWidget::linkWasActivated(const QString& link)
{
    if (link == QLatin1String("navigateToFunction")) {
        DUChainReadLocker lock(DUChain::lock());
        DUContext* context = m_context.context();
        if (context) {
            CursorInRevision contextStart = context->range().start;
            KTextEditor::Cursor cursor(contextStart.line, contextStart.column);
            QUrl url = context->url().toUrl();
            lock.unlock();
            ForegroundLock fgLock;
            ICore::self()->documentController()->openDocument(url, cursor);
        }
    }
}

DeclarationWidget::DeclarationWidget(const CodeRepresentation& code, const IndexedDeclaration& decl)
{
    setFrameShape(NoFrame);
    DUChainReadLocker lock(DUChain::lock());

    setUpdatesEnabled(false);
    if (Declaration* dec = decl.data()) {
        auto* headerLabel = new QLabel(dec->isDefinition() ? i18n("Definition") : i18n("Declaration"));
        addHeaderItem(headerLabel);
        addItem(new OneUseWidget(decl, dec->url(), dec->rangeInCurrentRevision(), code));
    }

    setUpdatesEnabled(true);
}

TopContextUsesWidget::TopContextUsesWidget(IndexedDeclaration declaration,
                                           const QList<IndexedDeclaration>& allDeclarations,
                                           IndexedTopDUContext topContext)
    : m_topContext(topContext)
    , m_declaration(declaration)
    , m_allDeclarations(allDeclarations)
    , m_usesCount(0)
{
    m_itemLayout->setContentsMargins(10, 0, 0, 5);
    setFrameShape(NoFrame);
    setUpdatesEnabled(false);
    DUChainReadLocker lock(DUChain::lock());
    auto* labelLayout = new QHBoxLayout;
    labelLayout->setContentsMargins(0, -1, 0, 0); // let's keep the spacing *above* the line
    auto* headerWidget = new QWidget;
    headerWidget->setLayout(labelLayout);
    headerWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    auto* label = new QLabel(this);
    m_icon = new QLabel(this);
    m_toggleButton = new QLabel(this);
    m_icon->setPixmap(QIcon::fromTheme(QStringLiteral("code-class")).pixmap(16));
    labelLayout->addWidget(m_icon);
    labelLayout->addWidget(label);
    labelLayout->addWidget(m_toggleButton);
    labelLayout->setAlignment(Qt::AlignLeft);

    if (topContext.isLoaded())
        m_usesCount = DUChainUtils::contextCountUses(topContext.data(), declaration.data());

    QString labelText = i18ncp("%1: number of uses, %2: filename with uses", "%2: 1 use", "%2: %1 uses",
                               m_usesCount,
                               ICore::self()->projectController()->prettyFileName(topContext.url().toUrl()));
    label->setText(labelText);

    m_toggleButton->setText(QLatin1String("&nbsp;&nbsp; <a href='toggleCollapsed'>[") +
                            i18nc("Refers to closing a UI element", "Collapse") + QLatin1String("]</a>"));

    connect(m_toggleButton, &QLabel::linkActivated, this, &TopContextUsesWidget::labelClicked);
    addHeaderItem(headerWidget);
    setUpdatesEnabled(true);
}

int TopContextUsesWidget::usesCount() const
{
    return m_usesCount;
}

QList<ContextUsesWidget*> buildContextUses(const CodeRepresentation& code,
                                           const QList<IndexedDeclaration>& declarations, DUContext* context)
{
    QList<ContextUsesWidget*> ret;

    if (!context->parentContext() || isNewGroup(context->parentContext(), context)) {
        auto* created = new ContextUsesWidget(code, declarations, context);
        if (created->hasItems())
            ret << created;
        else
            delete created;
    }

    const auto childContexts = context->childContexts();
    for (DUContext* child : childContexts) {
        ret += buildContextUses(code, declarations, child);
    }

    return ret;
}

void TopContextUsesWidget::setExpanded(bool expanded)
{
    if (!expanded) {
        m_toggleButton->setText(QLatin1String("&nbsp;&nbsp; <a href='toggleCollapsed'>[") +
                                i18nc("Refers to opening a UI element", "Expand") + QLatin1String("]</a>"));
        deleteItems();
    } else {
        m_toggleButton->setText(QLatin1String("&nbsp;&nbsp; <a href='toggleCollapsed'>[") +
                                i18nc("Refers to closing a UI element", "Collapse") + QLatin1String("]</a>"));
        if (hasItems())
            return;
        DUChainReadLocker lock(DUChain::lock());
        TopDUContext* topContext = m_topContext.data();

        if (topContext && m_declaration.data()) {
            CodeRepresentation::Ptr code = createCodeRepresentation(topContext->url());
            setUpdatesEnabled(false);

            IndexedTopDUContext localTopContext(topContext);
            for (const IndexedDeclaration& decl : std::as_const(m_allDeclarations)) {
                if (decl.indexedTopContext() == localTopContext) {
                    addItem(new DeclarationWidget(*code, decl));
                }
            }

            const auto contextUseWidgets = buildContextUses(*code, m_allDeclarations, topContext);
            for (ContextUsesWidget* usesWidget : contextUseWidgets) {
                addItem(usesWidget);
            }

            setUpdatesEnabled(true);
        }
    }
}

void TopContextUsesWidget::labelClicked()
{
    if (hasItems()) {
        setExpanded(false);
    } else {
        setExpanded(true);
    }
}

UsesWidget::~UsesWidget()
{
    if (m_collector) {
        m_collector->setWidget(nullptr);
    }
}

UsesWidget::UsesWidget(const IndexedDeclaration& declaration,
                       const QSharedPointer<UsesWidgetCollector>& customCollector)
    : NavigatableWidgetList(true)
{
    DUChainReadLocker lock(DUChain::lock());
    setUpdatesEnabled(false);

    m_headerLine = new QLabel;
    redrawHeaderLine();
    connect(m_headerLine, &QLabel::linkActivated, this, &UsesWidget::headerLinkActivated);
    m_layout->insertWidget(0, m_headerLine, 0, Qt::AlignTop);

    m_layout->setAlignment(Qt::AlignTop);
    m_itemLayout->setAlignment(Qt::AlignTop);

    m_progressBar = new QProgressBar;
    addHeaderItem(m_progressBar);

    if (!customCollector) {
        m_collector = QSharedPointer<UsesWidgetCollector>(new UsesWidget::UsesWidgetCollector(declaration));
    } else {
        m_collector = customCollector;
    }

    m_collector->setProcessDeclarations(true);
    m_collector->setWidget(this);
    m_collector->startCollecting();

    setUpdatesEnabled(true);
}

void UsesWidget::redrawHeaderLine()
{
    m_headerLine->setText(headerLineText());
}

const QString UsesWidget::headerLineText() const
{
    return i18np("1 use found", "%1 uses found", countAllUses()) + QLatin1String(" &bull; "
                                                                                 "<a href='expandAll'>[") + i18n(
        "Expand all") + QLatin1String("]</a> &bull; "
                                      "<a href='collapseAll'>[")
           + i18n("Collapse all") + QLatin1String("]</a>");
}

unsigned int UsesWidget::countAllUses() const
{
    unsigned int totalUses = 0;
    const auto items = this->items();
    for (QWidget* w : items) {
        if (auto* useWidget = qobject_cast<TopContextUsesWidget*>(w)) {
            totalUses += useWidget->usesCount();
        }
    }

    return totalUses;
}

void UsesWidget::setAllExpanded(bool expanded)
{
    const auto items = this->items();
    for (QWidget* w : items) {
        if (auto* useWidget = qobject_cast<TopContextUsesWidget*>(w)) {
            useWidget->setExpanded(expanded);
        }
    }
}

void UsesWidget::headerLinkActivated(const QString& linkName)
{
    if (linkName == QLatin1String("expandAll")) {
        setAllExpanded(true);
    } else if (linkName == QLatin1String("collapseAll")) {
        setAllExpanded(false);
    }
}

UsesWidget::UsesWidgetCollector::UsesWidgetCollector(IndexedDeclaration decl) : UsesCollector(decl)
    , m_widget(nullptr)
{
}

void UsesWidget::UsesWidgetCollector::setWidget(UsesWidget* widget)
{
    m_widget = widget;
}

void UsesWidget::UsesWidgetCollector::maximumProgress(uint max)
{
    if (!m_widget) {
        return;
    }

    if (m_widget->m_progressBar) {
        m_widget->m_progressBar->setMaximum(max);
        m_widget->m_progressBar->setMinimum(0);
        m_widget->m_progressBar->setValue(0);
    } else {
        qCWarning(LANGUAGE) << "maximumProgress called twice";
    }
}

void UsesWidget::UsesWidgetCollector::progress(uint processed, uint total)
{
    if (!m_widget) {
        return;
    }

    m_widget->redrawHeaderLine();

    if (m_widget->m_progressBar) {
        m_widget->m_progressBar->setValue(processed);

        if (processed == total) {
            m_widget->setUpdatesEnabled(false);
            delete m_widget->m_progressBar;
            m_widget->m_progressBar = nullptr;
            m_widget->setShowHeader(false);
            m_widget->setUpdatesEnabled(true);
        }
    } else {
        qCWarning(LANGUAGE) << "progress() called too often";
    }
}

void UsesWidget::UsesWidgetCollector::processUses(KDevelop::ReferencedTopDUContext topContext)
{
    if (!m_widget) {
        return;
    }

    DUChainReadLocker lock;

    qCDebug(LANGUAGE) << "processing" << topContext->url().str();
    auto* widget = new TopContextUsesWidget(declaration(), declarations(), topContext.data());

    // move to back if it's just the declaration/definition
    bool toBack = widget->usesCount() == 0;
    // move to front the item belonging to the current open document
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    bool toFront = doc && (doc->url() == topContext->url().toUrl());

    widget->setExpanded(true);

    m_widget->addItem(widget, toFront ? ItemPriority::High : toBack ? ItemPriority::Low : ItemPriority::Medium);
    m_widget->redrawHeaderLine();
}

QSize KDevelop::UsesWidget::sizeHint() const
{
    QSize ret = QWidget::sizeHint();
    if (ret.height() < 300)
        ret.setHeight(300);
    return ret;
}

#include "moc_useswidget.cpp"
