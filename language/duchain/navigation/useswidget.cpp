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
#include <QCompleter>
#include <KComboBox>
#include <qpushbutton.h>
#include <limits>
#include <klocalizedstring.h>
#include <qabstractitemview.h>
#include <ktexteditor/smartrange.h>
#include <QToolButton>
#include <language/duchain/use.h>
#include <kicon.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <QFile>
#include <interfaces/iprojectcontroller.h>
#include <qtextdocument.h>
#include <qevent.h>
#include <qtextlayout.h>
#include <language/duchain/duchainutils.h>
#include "language/duchain/parsingenvironment.h"
#include "language/duchain/types/indexedtype.h"
#include <language/backgroundparser/parsejob.h>


using namespace KDevelop;

const int tooltipContextSize = 3; //How many lines around the use are shown in the tooltip
const bool showUsesHeader = false;

///Allows getting code-lines conveniently, either through an open editor, or from a disk-loaded file.
class CodeRepresentation {
  public:
    virtual ~CodeRepresentation() {
    }
    virtual QString line(int line) const = 0;
};

class EditorCodeRepresentation : public CodeRepresentation {
  public:
  EditorCodeRepresentation(KTextEditor::Document* document) : m_document(document) {
  }
  QString line(int line) const {
    if(line < 0 || line >= m_document->lines())
      return QString();
    return m_document->line(line);
  }
  private:
    KTextEditor::Document* m_document;
};

class FileCodeRepresentation : public CodeRepresentation {
  public:
    FileCodeRepresentation(IndexedString document) {
    QString localFile(document.toUrl().toLocalFile());
  
        QFile file( localFile );
        if ( file.open(QIODevice::ReadOnly) )
          lines = file.readAll().split('\n');
    }
    
    QString line(int line) const {
    if(line < 0 || line >= lines.size())
      return QString();
      
      return QString::fromLocal8Bit(lines[line]);
    }
  private:
    //We use QByteArray, because the column-numbers are measured in utf-8
    QList<QByteArray> lines;
};

CodeRepresentation* createCodeRepresentation(IndexedString url) {
  IDocument* document = ICore::self()->documentController()->documentForUrl(url.toUrl());
  if(document && document->textDocument())
    return new EditorCodeRepresentation(document->textDocument());
  else
    return new FileCodeRepresentation(url);
}

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
  
  return Qt::escape(line.left(range.start.column)) + "<span style=\"background-color:yellow\">" + Qt::escape(line.mid(range.start.column, range.end.column - range.start.column)) + "</span>" + Qt::escape(line.mid(range.end.column, line.length() - range.end.column)) ;
}

OneUseWidget::OneUseWidget(IndexedDeclaration declaration, IndexedString document, SimpleRange range, const CodeRepresentation& code, KTextEditor::SmartRange* smartRange) : m_range(range), m_document(document), m_smartRange(smartRange), m_declaration(declaration) {
  
  //Make the sizing of this widget independent of the content, because we will adapt the content to the size
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  
  m_sourceLine = code.line(m_range.start.line);
  if(m_smartRange)
    m_smartRange->addWatcher(this);
  
  connect(this, SIGNAL(linkActivated(QString)), this, SLOT(jumpTo()));
  
  DUChainReadLocker lock(DUChain::lock());
  QString text = i18n("<a href='open'>Line <b>%1</b></a>", range.start.line);
  if(!m_sourceLine.isEmpty() && m_sourceLine.length() > m_range.end.column) {
    
    text += " " + highlightAndEscapeUseText(m_sourceLine, 0, m_range);
    
    //Useful tooltip:
    int start = m_range.start.line - tooltipContextSize;
    int end = m_range.end.line + tooltipContextSize + 1;
    
    QString toolTipText;
    for(int a = start; a < end; ++a) {
      QString lineText = code.line(a);
      if(!lineText.isEmpty())
        toolTipText += lineText + "\n";
    }
    setToolTip(toolTipText);
  }
  setText(text);
}

void OneUseWidget::jumpTo() {
  if(m_smartRange)
    m_range = *m_smartRange; ///@todo smart-locking
        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
  ICore::self()->documentController()->openDocument(m_document.toUrl(), m_range.start.textCursor());
}

OneUseWidget::~OneUseWidget() {
  if(m_smartRange)
    m_smartRange->removeWatcher(this);
}

void OneUseWidget::resizeEvent ( QResizeEvent * event ) {
  ///Adapt the content
  QSize size = event->size();

  int cutOff = 0;
  uint maxCutOff = m_sourceLine.length() - (m_range.end.column - m_range.start.column);
  
  //Reset so we also get more context while up-sizing
  setText(i18n("<a href='open'>Line <b>%1</b></a>", m_range.start.line) + " " + highlightAndEscapeUseText(m_sourceLine, cutOff, m_range));
  
  while(sizeHint().width() > size.width() && cutOff < maxCutOff) {
    //We've got to save space
    setText(i18n("<a href='open'>Line <b>%1</b></a>", m_range.start.line) + " " + highlightAndEscapeUseText(m_sourceLine, cutOff, m_range));
    cutOff += 5;
  }
  
  event->accept();
  
  QLabel::resizeEvent(event);
}

void OneUseWidget::rangeDeleted(KTextEditor::SmartRange* smartRange) {
  Q_ASSERT(smartRange == m_smartRange);
  m_smartRange = 0;
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

NavigatableWidgetList::NavigatableWidgetList(bool allowScrolling, uint maxHeight, bool vertical) : m_maxHeight(maxHeight), m_allowScrolling(allowScrolling) {
  m_layout = new QVBoxLayout;
  m_layout->setMargin(0);
  m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  m_layout->setSpacing(0);
  
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
  
  
  m_previousButton = new QToolButton();
  m_previousButton->setIcon(KIcon("go-previous"));
  
  m_nextButton = new QToolButton();
  m_nextButton->setIcon(KIcon("go-next"));
  
  m_headerLayout->addWidget(m_previousButton);
  m_headerLayout->addWidget(m_nextButton);
  
  m_layout->addLayout(m_headerLayout);
  m_layout->addLayout(m_itemLayout);

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

void NavigatableWidgetList::addItem(QWidget* widget) {
  m_itemLayout->addWidget(widget);
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

void NavigatableWidgetList::addHeaderItem(QWidget* widget) {
  Q_ASSERT(m_headerLayout->count() >= 2); //At least the 2 back/next buttons
  m_headerLayout->insertWidget(m_headerLayout->count()-1, widget);
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
  for(int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
    if(context->uses()[useIndex].m_declarationIndex == usedDeclarationIndex)
      ret << new OneUseWidget(decl, context->url(), context->uses()[useIndex].m_range, code, context->useSmartRange(useIndex));
    
  foreach(DUContext* child, context->childContexts())
    if(!isNewGroup(context, child))
      ret += createUseWidgets(code, usedDeclarationIndex, decl, child);
  
  return ret;
}

ContextUsesWidget::ContextUsesWidget(const CodeRepresentation& code, IndexedDeclaration usedDeclaration, IndexedDUContext context) : m_usedDeclaration(usedDeclaration), m_context(context) {

  DUChainReadLocker lock(DUChain::lock());
    QString headerText = i18n("Unknown context");
    uint usesCount = 0;
    
    if(context.data() && m_usedDeclaration.data()) {
      DUContext* ctx = context.data();
      
      if(ctx->scopeIdentifier(true).isEmpty())
        headerText = i18n("Unnamed scope");
      else {
        headerText = ctx->scopeIdentifier(true).toString();
        if(ctx->type() == DUContext::Function || (ctx->owner() && ctx->owner()->isFunctionDeclaration()))
          headerText += "(..)";
      }
      
      int usedDeclarationIndex = ctx->topContext()->indexForUsedDeclaration(usedDeclaration.data(), false);
      if(usedDeclarationIndex != std::numeric_limits<int>::max()) {
        foreach(OneUseWidget* widget, createUseWidgets(code, usedDeclarationIndex, m_usedDeclaration, ctx))
          addItem(widget);
        
        usesCount = countUses(usedDeclarationIndex, ctx);
      }
    }
    
    addHeaderItem(new QLabel("<a href='navigateToFunction'>" + Qt::escape(headerText) + "</a>"));
    if(usesCount)
      addHeaderItem(new QLabel(" " + i18n("Count: %1", usesCount)));
}

TopContextUsesWidget::TopContextUsesWidget(IndexedDeclaration declaration, IndexedTopDUContext topContext) : m_topContext(topContext), m_declaration(declaration) {
    DUChainReadLocker lock(DUChain::lock());
    QPushButton* label = new QPushButton;
    label->setText(KUrl(topContext.url().str()).fileName());
    connect(label, SIGNAL(clicked(bool)), this, SLOT(labelClicked()));
    addHeaderItem(label);
}

QList<ContextUsesWidget*> buildContextUses(const CodeRepresentation& code, IndexedDeclaration declaration, DUContext* context) {
  QList<ContextUsesWidget*> ret;
  
  if(!context->parentContext() || isNewGroup(context->parentContext(), context)) {
    ContextUsesWidget* created = new ContextUsesWidget(code, declaration, context);
    if(created->hasItems())
      ret << created;
    else
        delete created;
  }
  
  foreach(DUContext* child, context->childContexts())
    ret += buildContextUses(code, declaration, child);
  
  return ret;
}

void TopContextUsesWidget::setExpanded(bool expanded) {
  if(!expanded) {
    deleteItems();
  }else{
    if(hasItems())
      return;
    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* topContext = m_topContext.data();
    
    if(topContext && m_declaration.data()) {
      
      CodeRepresentation* code = createCodeRepresentation(topContext->url());
      setUpdatesEnabled(false);
      foreach(ContextUsesWidget* useWidget, buildContextUses(*code, m_declaration, topContext))
        addItem(useWidget);
      setUpdatesEnabled(true);
      
      delete code;
    }
  }
}

void TopContextUsesWidget::labelClicked() {
  if(hasItems())
    setExpanded(false);
  else
    setExpanded(true);
}

UsesWidget::~UsesWidget() {
  delete m_collector;
}

UsesWidget::UsesWidget(IndexedDeclaration declaration) {
    DUChainReadLocker lock(DUChain::lock());
    setUpdatesEnabled(false);
    
    QLabel* label = new QLabel(i18n("Uses:"));
    addHeaderItem(label);

    if(!showUsesHeader)
      setShowHeader(false);
    
    m_collector = new UsesWidgetCollector(declaration, this);
    
    setUpdatesEnabled(true);
}

UsesWidget::UsesWidgetCollector::UsesWidgetCollector(IndexedDeclaration decl, UsesWidget* widget) : UsesCollector(decl), m_widget(widget) {
  
}

void UsesWidget::UsesWidgetCollector::progress(uint processed, uint total) {
}

void UsesWidget::UsesWidgetCollector::processUses( KDevelop::ReferencedTopDUContext topContext ) {
    TopContextUsesWidget* widget = new TopContextUsesWidget(declaration().data(), topContext.data());
    m_widget->addItem(widget);
}

UsesCollector::~UsesCollector() {
  foreach(IndexedString file, m_staticFeaturesManipulated)
    ParseJob::unsetStaticMinimumFeatures(file, TopDUContext::AllDeclarationsContextsAndUses);
}



#include "useswidget.moc"
