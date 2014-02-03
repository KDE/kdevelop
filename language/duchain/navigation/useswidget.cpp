/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "useswidget.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/uses.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <klocalizedstring.h>
#include <QToolButton>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <kicon.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <qtextdocument.h>
#include <QResizeEvent>
#include <language/duchain/duchainutils.h>
#include <language/codegen/coderepresentation.h>
#include <interfaces/iproject.h>
#include <interfaces/foregroundlock.h>

using namespace KDevelop;


const int tooltipContextSize = 2; //How many lines around the use are shown in the tooltip

///The returned text is fully escaped
///@param cutOff The total count of characters that should be cut of, all in all on both sides together.
///@param range The range that is highlighted, and that will be preserved during cutting, given that there is enough room beside it.
QString highlightAndEscapeUseText(QString line, uint cutOff, SimpleRange range) {
  uint leftCutRoom = range.start.column;
  uint rightCutRoom = line.length() - range.end.column;

  if(range.start.column < 0 || range.end.column > line.length() || cutOff > leftCutRoom + rightCutRoom)
    return QString(); //Not enough room for cutting off on sides

  uint leftCut = 0;
  uint rightCut = 0;

  if(leftCutRoom < rightCutRoom) {
    if(leftCutRoom * 2 >= cutOff) {
      //Enough room on both sides. Just cut.
      leftCut = cutOff / 2;
      rightCut = cutOff - leftCut;
    }else{
      //Not enough room in left side, but enough room all together
      leftCut = leftCutRoom;
      rightCut = cutOff - leftCut;
    }
  }else{
    if(rightCutRoom * 2 >= cutOff) {
      //Enough room on both sides. Just cut.
      rightCut = cutOff / 2;
      leftCut = cutOff - rightCut;
    }else{
      //Not enough room in right side, but enough room all together
      rightCut = rightCutRoom;
      leftCut = cutOff - rightCut;
    }
  }
  Q_ASSERT(leftCut + rightCut <= cutOff);

  line = line.left(line.length() - rightCut);
  line = line.mid(leftCut);
  range.start.column -= leftCut;
  range.end.column -= leftCut;

  Q_ASSERT(range.start.column >= 0 && range.end.column <= line.length());

  //TODO: share code with context browser
  // mixing (255, 255, 0, 100) with white yields this:
  const QColor background(251, 250, 150);
  const QColor foreground(0, 0, 0);

  return "<span style=\"font-family:'monospace'\">" + Qt::escape(line.left(range.start.column))
                    + "<span style=\"background-color:" + background.name() + ";color:" + foreground.name() + ";\">"
                    + Qt::escape(line.mid(range.start.column, range.end.column - range.start.column))
                    + "</span>" + Qt::escape(line.mid(range.end.column, line.length() - range.end.column)) + "</span>";
}

OneUseWidget::OneUseWidget(IndexedDeclaration declaration, IndexedString document, SimpleRange range, const CodeRepresentation& code) : m_range(new PersistentMovingRange(range, document)), m_declaration(declaration), m_document(document) {

  //Make the sizing of this widget independent of the content, because we will adapt the content to the size
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

  m_sourceLine = code.line(m_range->range().start.line);

  m_layout = new QHBoxLayout(this);
  setLayout(m_layout);

  m_label = new QLabel(this);
  m_icon = new QLabel(this);
  m_icon->setPixmap(KIcon("code-function").pixmap(16));

  connect(m_label, SIGNAL(linkActivated(QString)), this, SLOT(jumpTo()));

  DUChainReadLocker lock(DUChain::lock());
  QString text = "<a href='open'>" + i18nc("refers to a line in source code", "Line <b>%1</b>:", range.start.line) + QString("</a>");
  if(!m_sourceLine.isEmpty() && m_sourceLine.length() > m_range->range().end.column) {

    text += "&nbsp;&nbsp;" + highlightAndEscapeUseText(m_sourceLine, 0, m_range->range());

    //Useful tooltip:
    int start = m_range->range().start.line - tooltipContextSize;
    int end = m_range->range().end.line + tooltipContextSize + 1;

    QString toolTipText;
    for(int a = start; a < end; ++a) {
      QString lineText = Qt::escape(code.line(a));
      if (m_range->range().start.line <= a && m_range->range().end.line >= a) {
        lineText = QString("<b>") + lineText + QString("</b>");
      }
      if(!lineText.trimmed().isEmpty()) {
        toolTipText += lineText + "<br>";
      }
    }
    if ( toolTipText.endsWith("<br>") ) {
      toolTipText.remove(toolTipText.length() - 4, 4);
    }
    setToolTip(QString("<html><body><pre>") + toolTipText + QString("</pre></body></html>"));
  }
  m_label->setText(text);

  m_layout->addWidget(m_icon);
  m_layout->addWidget(m_label);
  m_layout->setAlignment(Qt::AlignLeft);
}

void OneUseWidget::jumpTo() {
        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
  ICore::self()->documentController()->openDocument(m_document.toUrl(), m_range->range().start.textCursor());
}

OneUseWidget::~OneUseWidget() {
}

void OneUseWidget::resizeEvent ( QResizeEvent * event ) {
  ///Adapt the content
  QSize size = event->size();

  SimpleRange range = m_range->range();
  
  int cutOff = 0;
  int maxCutOff = m_sourceLine.length() - (range.end.column - range.start.column);

  //Reset so we also get more context while up-sizing
  m_label->setText(QString("<a href='open'>") + i18nc("Refers to a line in source code", "Line <b>%1</b>", range.start.line+1)
                 + QString("</a> %2").arg(highlightAndEscapeUseText(m_sourceLine, cutOff, range)));

  while(sizeHint().width() > size.width() && cutOff < maxCutOff) {
    //We've got to save space
    m_label->setText(QString("<a href='open'>") + i18nc("Refers to a line in source code", "Line <b>%1</b>", range.start.line+1)
                   + QString("</a> %2").arg(highlightAndEscapeUseText(m_sourceLine, cutOff, range)));
    cutOff += 5;
  }

  event->accept();

  QWidget::resizeEvent(event);
}

void NavigatableWidgetList::setShowHeader(bool show) {
  if(show && !m_headerLayout->parent())
    m_layout->insertLayout(0, m_headerLayout);
  else
    m_headerLayout->setParent(0);
}

NavigatableWidgetList::~NavigatableWidgetList() {
  delete m_headerLayout;
}

NavigatableWidgetList::NavigatableWidgetList(bool allowScrolling, uint maxHeight, bool vertical)
:  m_allowScrolling(allowScrolling)
{
  m_layout = new QVBoxLayout;
  m_layout->setMargin(0);
  m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  m_layout->setSpacing(0);
  setBackgroundRole(QPalette::Base);
  m_useArrows = false;

  if(vertical)
    m_itemLayout = new QVBoxLayout;
  else
    m_itemLayout = new QHBoxLayout;

  m_itemLayout->setMargin(0);
  m_itemLayout->setSpacing(0);
//   m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
  setWidgetResizable(true);

  m_headerLayout = new QHBoxLayout;
  m_headerLayout->setMargin(0);
  m_headerLayout->setSpacing(0);

  if(m_useArrows) {
    m_previousButton = new QToolButton();
    m_previousButton->setIcon(KIcon("go-previous"));

    m_nextButton = new QToolButton();
    m_nextButton->setIcon(KIcon("go-next"));

    m_headerLayout->addWidget(m_previousButton);
    m_headerLayout->addWidget(m_nextButton);
  }

  //hide these buttons for now, they're senseless

  m_layout->addLayout(m_headerLayout);
  
  QHBoxLayout* spaceLayout = new QHBoxLayout;
  spaceLayout->addSpacing(10);
  spaceLayout->addLayout(m_itemLayout);
  
  m_layout->addLayout(spaceLayout);

  if(maxHeight)
    setMaximumHeight(maxHeight);

  if(m_allowScrolling) {
    QWidget* contentsWidget = new QWidget;
    contentsWidget->setLayout(m_layout);
    setWidget(contentsWidget);
  }else{
    setLayout(m_layout);
  }
}

void NavigatableWidgetList::deleteItems() {
  foreach(QWidget* item, items())
    delete item;
}

void NavigatableWidgetList::addItem(QWidget* widget, int pos) {
  if(pos == -1)
    m_itemLayout->addWidget(widget);
  else
    m_itemLayout->insertWidget(pos, widget);
}

QList<QWidget*> NavigatableWidgetList::items() const {
  QList<QWidget*> ret;
  for(int a = 0; a < m_itemLayout->count(); ++a) {
    QWidgetItem* widgetItem = dynamic_cast<QWidgetItem*>(m_itemLayout->itemAt(a));
    if(widgetItem) {
      ret << widgetItem->widget();
    }
  }
  return ret;
}

bool NavigatableWidgetList::hasItems() const {
  return (bool)m_itemLayout->count();
}

void NavigatableWidgetList::addHeaderItem(QWidget* widget, Qt::Alignment alignment) {
  if(m_useArrows) {
    Q_ASSERT(m_headerLayout->count() >= 2); //At least the 2 back/next buttons
    m_headerLayout->insertWidget(m_headerLayout->count()-1, widget, alignment);
  }else{
    //We need to do this so the header doesn't get stretched
    widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_headerLayout->insertWidget(m_headerLayout->count(), widget, alignment);
//     widget->setMaximumHeight(20);
  }
}

///Returns whether the uses in the child should be a new uses-group
bool isNewGroup(DUContext* parent, DUContext* child) {
  if(parent->type() == DUContext::Other && child->type() == DUContext::Other)
    return false;
  else
    return true;
}

uint countUses(int usedDeclarationIndex, DUContext* context) {
  uint ret = 0;

  for(int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
    if(context->uses()[useIndex].m_declarationIndex == usedDeclarationIndex)
      ++ret;

  foreach(DUContext* child, context->childContexts())
    if(!isNewGroup(context, child))
      ret += countUses(usedDeclarationIndex, child);
  return ret;
}

QList<OneUseWidget*> createUseWidgets(const CodeRepresentation& code, int usedDeclarationIndex, IndexedDeclaration decl, DUContext* context) {
  QList<OneUseWidget*> ret;
  VERIFY_FOREGROUND_LOCKED
  
  for(int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
    if(context->uses()[useIndex].m_declarationIndex == usedDeclarationIndex)
      ret << new OneUseWidget(decl, context->url(), context->transformFromLocalRevision(context->uses()[useIndex].m_range), code);

  foreach(DUContext* child, context->childContexts())
    if(!isNewGroup(context, child))
      ret += createUseWidgets(code, usedDeclarationIndex, decl, child);

  return ret;
}

ContextUsesWidget::ContextUsesWidget(const CodeRepresentation& code, QList<IndexedDeclaration> usedDeclarations, IndexedDUContext context) : m_context(context) {

  setFrameShape(NoFrame);

  DUChainReadLocker lock(DUChain::lock());
    QString headerText = i18n("Unknown context");
    setUpdatesEnabled(false);

    if(context.data()) {
      DUContext* ctx = context.data();

      if(ctx->scopeIdentifier(true).isEmpty())
        headerText = i18n("Global");
      else {
        headerText = ctx->scopeIdentifier(true).toString();
        if(ctx->type() == DUContext::Function || (ctx->owner() && ctx->owner()->isFunctionDeclaration()))
          headerText += "(...)";
      }

      QSet<int> hadIndices;

      foreach(const IndexedDeclaration &usedDeclaration, usedDeclarations) {
        int usedDeclarationIndex = ctx->topContext()->indexForUsedDeclaration(usedDeclaration.data(), false);
        if(hadIndices.contains(usedDeclarationIndex))
          continue;

        hadIndices.insert(usedDeclarationIndex);

        if(usedDeclarationIndex != std::numeric_limits<int>::max()) {
          foreach(OneUseWidget* widget, createUseWidgets(code, usedDeclarationIndex, usedDeclaration, ctx))
            addItem(widget);
        }
      }
    }

    QLabel* headerLabel = new QLabel(i18nc("%1: source file", "In %1", "<a href='navigateToFunction'>" 
                                          + Qt::escape(headerText) + "</a>: "));
    addHeaderItem(headerLabel);
    setUpdatesEnabled(true);
    connect(headerLabel, SIGNAL(linkActivated(QString)), this, SLOT(linkWasActivated(QString)));
}

void ContextUsesWidget::linkWasActivated(QString link) {
  if ( link == "navigateToFunction" ) {
    DUChainReadLocker lock(DUChain::lock());
    DUContext* context = m_context.context();
    if(context) {
      CursorInRevision contextStart = context->range().start;
      KTextEditor::Cursor cursor(contextStart.line, contextStart.column);
      KUrl url = context->url().toUrl();
      lock.unlock();
      ForegroundLock fgLock;
      ICore::self()->documentController()->openDocument(url, cursor);
    }
  }
}

DeclarationWidget::DeclarationWidget(const CodeRepresentation& code, const IndexedDeclaration& decl) {

  setFrameShape(NoFrame);
  DUChainReadLocker lock(DUChain::lock());

  setUpdatesEnabled(false);
  if (Declaration* dec = decl.data()) {
    QLabel* headerLabel = new QLabel(dec->isDefinition() ? i18n("Definition") : i18n("Declaration"));
    addHeaderItem(headerLabel);
    addItem(new OneUseWidget(decl, dec->url(), dec->rangeInCurrentRevision(), code));
  }

  setUpdatesEnabled(true);
}

TopContextUsesWidget::TopContextUsesWidget(IndexedDeclaration declaration, QList<IndexedDeclaration> allDeclarations, IndexedTopDUContext topContext)
  : m_topContext(topContext)
  , m_declaration(declaration)
  , m_allDeclarations(allDeclarations)
  , m_usesCount(0)
{
    m_itemLayout->setContentsMargins(10, 0, 0, 5);
    setFrameShape(NoFrame);
    setUpdatesEnabled(false);
    DUChainReadLocker lock(DUChain::lock());
    QHBoxLayout * labelLayout = new QHBoxLayout;
    QWidget* headerWidget = new QWidget;
    headerWidget->setLayout(labelLayout);
    headerWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    QLabel* label = new QLabel(this);
    m_icon = new QLabel(this);
    m_toggleButton = new QLabel(this);
    m_icon->setPixmap(KIcon("code-class").pixmap(16));
    labelLayout->addWidget(m_icon);
    labelLayout->addWidget(label);
    labelLayout->addWidget(m_toggleButton);
    labelLayout->setAlignment(Qt::AlignLeft);

    if(topContext.isLoaded())
      m_usesCount = DUChainUtils::contextCountUses(topContext.data(), declaration.data());

    QString labelText = i18ncp("%1: number of uses, %2: filename with uses", "%2: 1 use", "%2: %1 uses",
                              m_usesCount, ICore::self()->projectController()->prettyFileName(topContext.url().toUrl()));
    label->setText(labelText);

    m_toggleButton->setText("&nbsp;&nbsp; <a href='toggleCollapsed'>[" + i18nc("Refers to closing a UI element", "Collapse") + "]</a>");

    connect(m_toggleButton, SIGNAL(linkActivated(QString)), this, SLOT(labelClicked()));
    addHeaderItem(headerWidget);
    setUpdatesEnabled(true);
}

int TopContextUsesWidget::usesCount() const
{
  return m_usesCount;
}

QList<ContextUsesWidget*> buildContextUses(const CodeRepresentation& code, QList<IndexedDeclaration> declarations, DUContext* context) {
  QList<ContextUsesWidget*> ret;

  if(!context->parentContext() || isNewGroup(context->parentContext(), context)) {
    ContextUsesWidget* created = new ContextUsesWidget(code, declarations, context);
    if(created->hasItems())
      ret << created;
    else
        delete created;
  }

  foreach(DUContext* child, context->childContexts())
    ret += buildContextUses(code, declarations, child);

  return ret;
}

void TopContextUsesWidget::setExpanded(bool expanded) {
  if(!expanded) {
    m_toggleButton->setText("&nbsp;&nbsp; <a href='toggleCollapsed'>[" + i18nc("Refers to opening a UI element", "Expand") + "]</a>");
    deleteItems();
  }else{
    m_toggleButton->setText("&nbsp;&nbsp; <a href='toggleCollapsed'>[" + i18nc("Refers to closing a UI element", "Collapse") + "]</a>");
    if(hasItems())
      return;
    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* topContext = m_topContext.data();

    if(topContext && m_declaration.data()) {

      CodeRepresentation::Ptr code = createCodeRepresentation(topContext->url());
      setUpdatesEnabled(false);

      IndexedTopDUContext localTopContext(topContext);
      foreach(const IndexedDeclaration &decl, m_allDeclarations) {
        if(decl.indexedTopContext() == localTopContext) {
          addItem(new DeclarationWidget(*code, decl));
        }
      }

      foreach(ContextUsesWidget* usesWidget, buildContextUses(*code, m_allDeclarations, topContext)) {
        addItem(usesWidget);
      }
      setUpdatesEnabled(true);
    }
  }
}

void TopContextUsesWidget::labelClicked() {
  if(hasItems()) {
    setExpanded(false);
  }else{
    setExpanded(true);
  }
}

UsesWidget::~UsesWidget()
{
    if (m_collector) {
        m_collector->setWidget(0);
    }
}

UsesWidget::UsesWidget(const IndexedDeclaration& declaration, QSharedPointer<UsesWidgetCollector> customCollector)
    : NavigatableWidgetList(true)
{
    DUChainReadLocker lock(DUChain::lock());
    setUpdatesEnabled(false);

    m_headerLine = new QLabel;
    redrawHeaderLine();
    connect(m_headerLine, SIGNAL(linkActivated(QString)), this, SLOT(headerLinkActivated(QString)));
    m_layout->insertWidget(0, m_headerLine, 0, Qt::AlignTop);

    m_layout->setAlignment(Qt::AlignTop);
    m_itemLayout->setAlignment(Qt::AlignTop);

    m_progressBar = new QProgressBar;
    addHeaderItem(m_progressBar);

    if (!customCollector) {
        m_collector = QSharedPointer<UsesWidgetCollector>(new UsesWidgetCollector(declaration));
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
  return i18np("1 use found", "%1 uses found", countAllUses()) + " &bull; "
              "<a href='expandAll'>[" + i18n("Expand all") + "]</a> &bull; "
              "<a href='collapseAll'>[" + i18n("Collapse all") + "]</a>";
}

unsigned int UsesWidget::countAllUses() const
{
  unsigned int totalUses = 0;
  foreach ( QWidget* w, items() ) {
    if ( TopContextUsesWidget* useWidget = dynamic_cast<TopContextUsesWidget*>(w) ) {
      totalUses += useWidget->usesCount();
    }
  }
  return totalUses;
}

void UsesWidget::setAllExpanded(bool expanded)
{
  foreach ( QWidget* w, items() ) {
    if ( TopContextUsesWidget* useWidget = dynamic_cast<TopContextUsesWidget*>(w) ) {
      useWidget->setExpanded(expanded);
    }
  }
}

void UsesWidget::headerLinkActivated(QString linkName)
{
  if(linkName == "expandAll") {
    setAllExpanded(true);
  }
  else if(linkName == "collapseAll") {
    setAllExpanded(false);
  }
}

UsesWidget::UsesWidgetCollector::UsesWidgetCollector(IndexedDeclaration decl) : UsesCollector(decl), m_widget(0) {

}

void UsesWidget::UsesWidgetCollector::setWidget(UsesWidget* widget ) {
  m_widget = widget;
}


void UsesWidget::UsesWidgetCollector::maximumProgress(uint max) {
  if (!m_widget) {
    return;
  }

  if(m_widget->m_progressBar) {
    m_widget->m_progressBar->setMaximum(max);
    m_widget->m_progressBar->setMinimum(0);
    m_widget->m_progressBar->setValue(0);
  }else{
    kWarning() << "maximumProgress called twice";
  }
}

void UsesWidget::UsesWidgetCollector::progress(uint processed, uint total) {
  if (!m_widget) {
    return;
  }

  m_widget->redrawHeaderLine();

  if(m_widget->m_progressBar) {
    m_widget->m_progressBar->setValue(processed);

    if(processed == total) {
      m_widget->setUpdatesEnabled(false);
      delete m_widget->m_progressBar;
      m_widget->m_progressBar = 0;
      m_widget->setShowHeader(false);
      m_widget->setUpdatesEnabled(true);
    }
  }else{
    kWarning() << "progress() called too often";
  }
}

void UsesWidget::UsesWidgetCollector::processUses( KDevelop::ReferencedTopDUContext topContext ) {
  if (!m_widget) {
    return;
  }

  DUChainReadLocker lock;

  kDebug() << "processing" << topContext->url().str();
  TopContextUsesWidget* widget = new TopContextUsesWidget(declaration(), declarations(), topContext.data());
  
  // move to back if it's just the declaration/definition
  bool toBack = widget->usesCount() == 0;
  // move to front the item belonging to the current open document
  IDocument* doc = ICore::self()->documentController()->activeDocument();
  bool toFront = doc && doc->url().equals(topContext->url().toUrl());

  widget->setExpanded(true);

  m_widget->addItem(widget, toFront ? 0 : toBack ? widget->items().size() : -1);
  m_widget->redrawHeaderLine();
}

QSize KDevelop::UsesWidget::sizeHint() const {
  QSize ret = QWidget::sizeHint();
  if(ret.height() < 300)
    ret.setHeight(300);
  return ret;
}


#include "useswidget.moc"
