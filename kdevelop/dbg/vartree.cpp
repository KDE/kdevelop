// **************************************************************************
//                          vartree.cpp  -  description
//                             -------------------
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jb.nz@writeme.com
// **************************************************************************

// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "vartree.h"
#include "gdbparser.h"

#include <kapp.h>     // here for i18n only! yuck!
#include <kpopupmenu.h>
#include <klineedit.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <klocale.h>

#if defined(DBG_MONITOR)
  #define DBG_DISPLAY(X)          {emit rawData(QString(X));}
#else
  #define DBG_DISPLAY(X)          {;}
#endif

// **************************************************************************

//TODO - change to a base class parser and setup a factory
static GDBParser* parser = 0;

static GDBParser* getParser()
{
  if (!parser)
    parser = new GDBParser;

  return parser;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarViewer::VarViewer( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout(this, 2);

  varTree_ = new VarTree(this);
  varTree_->setFocusPolicy(QWidget::NoFocus);
  topLayout->addWidget( varTree_, 10 );

  QBoxLayout *watchEntry = new QHBoxLayout();
  topLayout->addLayout( watchEntry );

  QLabel *label = new QLabel( i18n("Watch: "), this );
//  label->setMinimumSize( label->sizeHint() );
//  label->setMaximumSize( label->sizeHint() );
  watchEntry->addWidget( label );

  // make the size small so that it can fit within the parent widget
  // size. The parents size is currently 4 tabs wide with <=3chars
  // in each tab. (ie quite small!) 
  watchVarEntry_ = new KLineEdit(this);
//  watchVarEntry_->setMinimumSize(0,0); //watchVarEntry_->sizeHint() );
  watchVarEntry_->setFocusPolicy(QWidget::ClickFocus);
  watchEntry->addWidget( watchVarEntry_ );

  // just add a bit of space at the end of the entry widget
//  QLabel *blank = new QLabel( " ", this );
//  blank->setMinimumSize( blank->sizeHint() );
//  blank->setMaximumSize( blank->sizeHint() );
//  watchEntry->addWidget( blank );

  QPushButton* addButton = new QPushButton( i18n("Add"), this );
//  addButton->setMinimumSize( addButton->sizeHint() );
//  addButton->setMaximumSize( addButton->sizeHint() );
  addButton->setFocusPolicy(QWidget::NoFocus);
  watchEntry->addWidget( addButton );

  connect(addButton, SIGNAL(clicked()), SLOT(slotAddWatchVariable()));
  connect(watchVarEntry_, SIGNAL(returnPressed()), SLOT(slotAddWatchVariable()));

  topLayout->activate();
}

// **************************************************************************

void VarViewer::clear()
{
  varTree_->clear();
}

// **************************************************************************

void VarViewer::slotAddWatchVariable()
{
  QString watchVar(watchVarEntry_->text());
  if (!watchVar.isEmpty())
    varTree_->slotAddWatchVariable(watchVar);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarTree::VarTree( QWidget *parent, const char *name ) :
  QListView(parent, name),
  activeFlag_(0)
{
  setRootIsDecorated(true);
  setSorting(-1);
  setFrameStyle(Panel | Sunken);
  setLineWidth(2);
  addColumn(i18n("Variable"));
  addColumn(i18n("Value"));
  setMultiSelection (false);

  connect (this,  SIGNAL(rightButtonClicked ( QListViewItem *, const QPoint &, int )),
                  SLOT(slotRightButtonClicked ( QListViewItem *, const QPoint &, int )));
}

// **************************************************************************

VarTree::~VarTree()
{
}

// **************************************************************************

void VarTree::slotRightButtonClicked( QListViewItem* selectedItem,
                                      const QPoint &,
                                      int)
{
  setSelected (selectedItem, true);    // Need to select this item.
  if (selectedItem->parent())
  {
    QListViewItem* item = findRoot(selectedItem);
    KPopupMenu popup(selectedItem->text(VarNameCol));
    if (dynamic_cast<WatchRoot*>(item))
    {
      popup.insertItem( i18n("Delete watch variable"), this, SLOT(slotRemoveWatchVariable()) );
    }

    popup.insertItem( i18n("Toggle watchpoint"), this, SLOT(slotToggleWatchpoint()) );
    popup.exec(QCursor::pos());
  }
}

// **************************************************************************

void VarTree::slotToggleWatchpoint()
{
  if (VarItem* item = dynamic_cast<VarItem*>(currentItem()))
    emit toggleWatchpoint(item->fullName());
}

// **************************************************************************

void VarTree::slotRemoveWatchVariable()
{
  delete currentItem();
}

// **************************************************************************

void VarTree::slotAddWatchVariable(const QString& watchVar)
{
  VarItem* varItem = new VarItem(findWatch(), watchVar, typeUnknown);
  emitExpandItem(varItem);
}

// **************************************************************************

void VarTree::emitSetLocalViewState(bool localsOn, int frameNo)
{
  // When they want to _close_ a frame then we need to check the state of
  // all other frames to determine whether we still need the locals.
  if (!localsOn)
  {
    QListViewItem* sibling = firstChild();
    while (sibling)
    {
      FrameRoot* frame = dynamic_cast<FrameRoot*> (sibling);
      if (frame && frame->isOpen())
      {
        localsOn = true;
        break;
      }

      sibling = sibling->nextSibling();
    }
  }

  emit setLocalViewState(localsOn);
  emit selectFrame(frameNo);
}

// **************************************************************************

QListViewItem* VarTree::findRoot(QListViewItem* item) const
{
  while (item->parent())
    item = item->parent();

  return item;
}

// **************************************************************************

FrameRoot* VarTree::findFrame(int frameNo) const
{
  QListViewItem* sibling = firstChild();

  // frames only exist on th top level so we only need to
  // check the siblings
  while (sibling)
  {
    FrameRoot* frame = dynamic_cast<FrameRoot*> (sibling);
    if (frame && frame->getFrameNo() == frameNo)
      return frame;

    sibling = sibling->nextSibling();
  }

  return 0;
}

// **************************************************************************

WatchRoot* VarTree::findWatch()
{
  QListViewItem* sibling = firstChild();

  while (sibling)
  {
    if (WatchRoot* watch = dynamic_cast<WatchRoot*> (sibling))
      return watch;

    sibling = sibling->nextSibling();
  }

  return new WatchRoot(this);
}

// **************************************************************************

void VarTree::trim()
{
  QListViewItem* child = firstChild();
  while (child)
  {
    QListViewItem* nextChild = child->nextSibling();

    // don't trim the watch root
    if (!(dynamic_cast<WatchRoot*> (child)))
    {
      if (TrimmableItem* item = dynamic_cast<TrimmableItem*> (child))
      {
        if (item->isActive())
          item->trim();
        else
          delete item;
      }
    }
    child = nextChild;
  }
}

// **************************************************************************

void VarTree::trimExcessFrames()
{
  QListViewItem* child = firstChild();
  while (child)
  {
    QListViewItem* nextChild = child->nextSibling();
    if (FrameRoot* frame = dynamic_cast<FrameRoot*> (child))
    {
      if (frame->getFrameNo() != 0)
        delete frame;
    }
    child = nextChild;
  }
}

// **************************************************************************

QListViewItem* VarTree::lastChild() const
{
  QListViewItem* child = firstChild();
  if (child)
    while (QListViewItem* nextChild = child->nextSibling())
      child = nextChild;

  return child;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

TrimmableItem::TrimmableItem(VarTree* parent) :
  QListViewItem (parent, parent->lastChild()),
  activeFlag_(0)
{
  setActive();
}

// **************************************************************************

TrimmableItem::TrimmableItem(TrimmableItem* parent) :
  QListViewItem (parent, parent->lastChild()),
  activeFlag_(0)
{
  setActive();
}

// **************************************************************************

TrimmableItem::~TrimmableItem()
{
}

// **************************************************************************

int TrimmableItem::rootActiveFlag() const
{
  return ((VarTree*)listView())->activeFlag();
}

// **************************************************************************

QListViewItem* TrimmableItem::lastChild() const
{
  QListViewItem* child = firstChild();
  if (child)
    while (QListViewItem* nextChild = child->nextSibling())
      child = nextChild;

  return child;
}

// **************************************************************************

TrimmableItem* TrimmableItem::findMatch
                (const QString& match, DataType type) const
{
  QListViewItem* child = firstChild();

  // Check the siblings on this branch
  while (child)
  {
    if (child->text(VarNameCol) == match)
    {
      if (TrimmableItem* item = dynamic_cast<TrimmableItem*> (child))
        if (item->getDataType() == type)
          return item;
    }

    child = child->nextSibling();
  }

  return 0;
}

// **************************************************************************

void TrimmableItem::trim()
{
  QListViewItem* child = firstChild();
  while (child)
  {
    QListViewItem* nextChild = child->nextSibling();
    if (TrimmableItem* item = dynamic_cast<TrimmableItem*>(child))
    {
      // Never trim a branch if we are waiting on data to arrive.
      if (!isOpen() || getDataType() != typePointer)
      {
        if (item->isActive())
          item->trim();      // recurse
        else
          delete item;
      }
    }
    child = nextChild;
  }
}

// **************************************************************************

DataType TrimmableItem::getDataType() const
{
  return typeUnknown;
}

// **************************************************************************

void TrimmableItem::setCache(const QString&)
{
  ASSERT(false);
}

// **************************************************************************

QString TrimmableItem::getCache()
{
  ASSERT(false);
  return QString();
}

// **************************************************************************

void TrimmableItem::updateValue(char* /* buf */)
{
}

// **************************************************************************

QString TrimmableItem::key (int, bool) const
{
  return QString::null;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarItem::VarItem( TrimmableItem* parent, const QString& varName, DataType dataType) :
  TrimmableItem (parent),
  cache_(QString()),
  dataType_(dataType),
  highlight_(false)
{
  setText (VarNameCol, varName);
}

// **************************************************************************

VarItem::~VarItem()
{
}

// **************************************************************************

QString VarItem::varPath() const
{
  QString varPath("");
  const VarItem* item = this;

  // This stops at the root item (FrameRoot or WatchRoot)
  while ((item = dynamic_cast<const VarItem*> (item->parent())))
  {
    if (item->getDataType() != typeArray)
    {
      if (*(item->text(VarNameCol)) != '<')
      {
        QString itemName(item->text(VarNameCol));
        varPath = itemName.replace(QRegExp("^static "), "") + "." + varPath;
      }
    }
  }

  return varPath;
}

// **************************************************************************

QString VarItem::fullName() const
{
  QString itemName(getName());
  ASSERT (itemName);
  if (itemName[0] == '<')
    return varPath();

  return varPath() + itemName.replace(QRegExp("^static "), "");
}

// **************************************************************************

void VarItem::setText ( int column, const QString& data )
{
  if (!isActive() && isOpen() && dataType_ == typePointer)
    ((VarTree*)listView())->emitExpandItem(this);

  setActive();
  if (column == ValueCol)
  {
    QString oldValue(text(column));
    if (oldValue)                   // Don't highlight new items
      highlight_ = (oldValue != QString(data));
  }

  QListViewItem::setText(column, data);
}

// **************************************************************************

void VarItem::updateValue(char* buf)
{
  if (*buf == '$')
  {
    if (char* end = strchr(buf, '='))
        buf = end+2;
  }

  if (dataType_ == typeUnknown)
  {
    dataType_ = getParser()->determineType(buf);
    if (dataType_ == typeArray)
      buf++;

    // Try fixing a format string here by overriding the dataType calculated
    // from this data
    QString varName=getName();
    if (dataType_ == typePointer && varName[0] == '/')
      dataType_ = typeValue;
  }

  getParser()->parseData(this, buf, true, false);
  setActive();
}

// **************************************************************************

void VarItem::setCache(const QString& value)
{
  cache_ = value;
  setExpandable(true);
  checkForRequests();
  if (isOpen())
    setOpen(true);
  setActive();
}

// **************************************************************************

void VarItem::setOpen(bool open)
{
  if (open)
  {
    if (cache_)
    {
      QCString value = cache_;
      cache_ = QCString();
      getParser()->parseData(this, value.data(), false, false);
      trim();
    }
    else
      if (dataType_ == typePointer || dataType_ == typeReference)
        ((VarTree*)listView())->emitExpandItem(this);
  }

  QListViewItem::setOpen(open);
}

// **************************************************************************

QString VarItem::getCache()
{
  return cache_;
}

// **************************************************************************

void VarItem::checkForRequests()
{
  // TODO - hardcoded for now - these should get read from config

  // Signature for a QT1.44 QString
  if (strncmp(cache_, "<QArrayT<char>> = {<QGArray> = {shd = ", 38) == 0)
    ((VarTree*)listView())->emitExpandUserItem(this,
                                          fullName()+QString(".shd.data"));

  // Signature for a QT1.44 QDir
  if (strncmp(cache_, "dPath = {<QArrayT<char>> = {<QGArray> = {shd", 44) == 0)
    ((VarTree*)listView())->emitExpandUserItem(this,
                                          fullName()+QString(".dPath.shd.data"));

  // Signature for a QT2.0.x QT2.1 QString
  // TODO - This handling is not that good - but it works sufficiently well
  // at the moment to leave it here, and it won't cause bad things to happen.
  if (strncmp(cache_, "d = 0x", 6) == 0)      // Eeeek - too small
    ((VarTree*)listView())->emitExpandUserItem(this,
           QString().sprintf("(($len=($data=%s.d).len)?$data.unicode.rw@($len>100?200:$len*2):\"\")",
           fullName().data()));
}

// **************************************************************************

DataType VarItem::getDataType() const
{
  return dataType_;
}

// **************************************************************************

// Overridden to highlight the changed items
void VarItem::paintCell( QPainter * p, const QColorGroup & cg,
                                int column, int width, int align )
{
  if ( !p )
    return;

  if (column == ValueCol && highlight_)
  {
    QColorGroup hl_cg( cg.foreground(), cg.background(), cg.light(),
                        cg.dark(), cg.mid(), red, cg.base());
    QListViewItem::paintCell( p, hl_cg, column, width, align );
  }
  else
    QListViewItem::paintCell( p, cg, column, width, align );
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

FrameRoot::FrameRoot(VarTree* parent, int frameNo) :
  TrimmableItem (parent),
  needLocals_(true),
  frameNo_(frameNo),
  params_(QCString()),
  locals_(QCString())
{
  setExpandable(true);
}

// **************************************************************************

FrameRoot::~FrameRoot()
{
}

// **************************************************************************

void FrameRoot::setLocals(char* locals)
{

  ASSERT(isActive());

  // "No symbol table info available" or "No locals."
  bool noLocals = (locals &&  (strncmp(locals, "No ", 3) == 0));
  setExpandable(!params_.isEmpty() || !noLocals);

  if (noLocals)
  {
    locals_ = "";
    if (locals)
      if (char* end = strchr(locals, '\n'))
        *end = 0;      // clobber the new line
  }
  else
    locals_ = locals;

  if (!isExpandable() && noLocals)
    setText ( ValueCol, locals );

  needLocals_ = false;
  if (isOpen())
    setOpen(true);
}

// **************************************************************************

void FrameRoot::setParams(const QString& params)
{
  setActive();
  params_ = params;
  needLocals_ = true;
}

// **************************************************************************

// Override setOpen so that we can decide what to do when we do change
// state. This
void FrameRoot::setOpen(bool open)
{
  bool localStateChange = (isOpen() != open);
  QListViewItem::setOpen(open);

  if (localStateChange)
    emit ((VarTree*)listView())->emitSetLocalViewState(open, frameNo_);

  if (!open)
    return;

  getParser()->parseData(this, params_.data(), false, true);
  getParser()->parseData(this, locals_.data(), false, false);

  locals_ = QCString();
  params_ = QCString();
}

// **************************************************************************
// **************************************************************************
// **************************************************************************
// **************************************************************************

WatchRoot::WatchRoot(VarTree* parent) :
  TrimmableItem(parent)
{
  setText(0, i18n("Watch"));
  setOpen(true);
}

// **************************************************************************

WatchRoot::~WatchRoot()
{
}

// **************************************************************************

void WatchRoot::requestWatchVars()
{
  for (QListViewItem* child = firstChild(); child; child = child->nextSibling())
    if (VarItem* varItem = dynamic_cast<VarItem*>(child))
      ((VarTree*)listView())->emitExpandItem(varItem);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

