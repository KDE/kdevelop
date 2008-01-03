/* This file is part of KDevelop
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "valgrindmodel.h"

#include <QApplication>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

//#include "modeltest.h"

ValgrindError::~ ValgrindError( )
{
  delete stack;
  delete auxStack;
}

ValgrindError::ValgrindError(ValgrindModel* parent)
  : uniqueId(-1)
  , threadId(-1)
  , kind(Unknown)
  , leakedBytes(0L)
  , leakedBlocks(0L)
  , stack(0L)
  , auxStack(0L)
  , m_parent(parent)
{
}

void ValgrindError::setKind(const QString& s)
{
  if (s == "Unknown")
    kind = Unknown;
  else if (s == "InvalidFree")
    kind = InvalidFree;
  else if (s == "MismatchedFree")
    kind = MismatchedFree;
  else if (s == "InvalidRead")
    kind = InvalidRead;
  else if (s == "InvalidWrite")
    kind = InvalidWrite;
  else if (s == "InvalidJump")
    kind = InvalidJump;
  else if (s == "Overlap")
    kind = Overlap;
  else if (s == "InvalidMemPool")
    kind = InvalidMemPool;
  else if (s == "UninitCondition")
    kind = UninitCondition;
  else if (s == "UninitValue")
    kind = UninitValue;
  else if (s == "SyscallParam")
    kind = SyscallParam;
  else if (s == "ClientCheck")
    kind = ClientCheck;
  else if (s == "Leak_DefinitelyLost")
    kind = Leak_DefinitelyLost;
  else if (s == "Leak_IndirectlyLost")
    kind = Leak_IndirectlyLost;
  else if (s == "Leak_PossiblyLost")
    kind = Leak_PossiblyLost;
  else if (s == "Leak_StillReachable")
    kind = Leak_StillReachable;
  else
    kind = Unknown;
}

ValgrindFrame::ValgrindFrame(ValgrindStack* parent)
  : m_parent(parent)
{
}

ValgrindStack::ValgrindStack(ValgrindError* parent)
  : m_parent(parent)
{
}

ValgrindStack::~ ValgrindStack( )
{
  qDeleteAll(frames);
}

ValgrindModel::~ ValgrindModel( )
{
  qDeleteAll(errors);
}

bool ValgrindModel::startElement()
{
  m_buffer.clear();

  State newState = Unknown;

  switch (m_state) {
    case Root:
      if (name() == "valgrindoutput")
        newState = Session;
      break;

    case Session:
      if (name() == "status")
        newState = Status;
      else if (name() == "preamble")
        newState = Preamble;
      else if (name() == "error") {
        newState = Error;
        m_currentError = new ValgrindError(this);

        beginInsertRows(QModelIndex(), errors.count(), errors.count());
        errors.append(m_currentError);
        endInsertRows();
      }
      break;

    case Error:
      if (name() == "stack") {
        newState = Stack;
        m_currentStack = new ValgrindStack(m_currentError);
      }
      break;

    case Stack:
      if (name() == "frame") {
        newState = Frame;
        m_currentFrame = new ValgrindFrame(m_currentStack);
      }
      break;

    default:
      break;
  }

  //kDebug() << name() << newState;

  m_stateStack.push(m_state);
  m_state = newState;
  ++m_depth;
  return true;
}

bool ValgrindModel::endElement()
{
  m_state = m_stateStack.pop();

  //kDebug() << name() << m_state;

  switch (m_state) {
    case Root:
      if (name() == "m_protocolVersion")
        m_protocolVersion = m_buffer.toInt();
      else if (name() == "pid")
        pid = m_buffer.toInt();
      else if (name() == "ppid")
        ppid = m_buffer.toInt();
      else if (name() == "tool")
        tool = m_buffer;
      else if (name() == "usercomment")
        userComment = m_buffer;
      else if (name() == "error")
        m_currentError = 0L;
      break;

    case Preamble:
      if (name() == "line")
        preamble.append(m_buffer);
      break;

    case Error:
      if (name() == "unique")
        m_currentError->uniqueId = m_buffer.toInt(0L, 16);
      else if (name() == "tid")
        m_currentError->threadId = m_buffer.toInt();
      else if (name() == "kind")
        m_currentError->setKind(m_buffer);
      else if (name() == "what")
        m_currentError->what = m_buffer;
      else if (name() == "leakedbytes")
        m_currentError->leakedBytes = m_buffer.toInt();
      else if (name() == "leakedblocks")
        m_currentError->leakedBlocks = m_buffer.toInt();
      else if (name() == "auxwhat")
        m_currentError->auxWhat = m_buffer;
      else if (name() == "stack") {
        if (!m_currentError->stack) {
            beginInsertRows(indexForItem(m_currentError), 0, 0);
            m_currentError->stack = m_currentStack;
            endInsertRows();
        } else if (!m_currentError->auxStack) {
            beginInsertRows(indexForItem(m_currentError), 1, 1);
            m_currentError->auxStack = m_currentStack;
            endInsertRows();
        } else {
            delete m_currentStack;
            kWarning() << "Unexpected stack received";
        }
        m_currentStack = 0L;
      }
      break;

    case Stack:
      if (name() == "frame") {
        m_currentStack->frames.append(m_currentFrame);
        m_currentFrame = 0L;
      }
      break;

    case Frame:
      if (name() == "ip")
        m_currentFrame->instructionPointer = m_buffer.toInt(0L, 16);
      else if (name() == "obj")
        m_currentFrame->obj = m_buffer;
      else if (name() == "fn")
        m_currentFrame->fn = m_buffer;
      else if (name() == "dir")
        m_currentFrame->dir = m_buffer;
      else if (name() == "file")
        m_currentFrame->file = m_buffer;
      else if (name() == "line")
        m_currentFrame->line = m_buffer.toInt();
      break;

    default:
      break;
  }

  --m_depth;
  return true;
}

int ValgrindModel::columnCount ( const QModelIndex & parent ) const
{
  Q_UNUSED(parent)
  return numColumns;
}

QVariant ValgrindModel::data ( const QModelIndex & index, int role ) const
{
  ValgrindItem* item = itemForIndex(index);

  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        /*case Index:
          if (ValgrindError* e = dynamic_cast<ValgrindError*>(item))
            return e->uniqueId;
          else if (ValgrindFrame* f = dynamic_cast<ValgrindFrame*>(item))
            return f->line;
          break;*/
        case Function:
          if (ValgrindError* e = dynamic_cast<ValgrindError*>(item))
            return e->what;
          else if (ValgrindStack* s = dynamic_cast<ValgrindStack*>(item))
            return s->what();
          else if (ValgrindFrame* f = dynamic_cast<ValgrindFrame*>(item))
            if (!f->fn.isEmpty())
              return f->fn;
            else
              return QString("0x%1").arg(QString::number(f->instructionPointer, 16));
          break;
        case Source:
          if (ValgrindFrame* f = dynamic_cast<ValgrindFrame*>(item))
            return f->url();
          break;
        case Object:
          if (ValgrindFrame* f = dynamic_cast<ValgrindFrame*>(item))
            return f->obj;
          break;
      }
      break;
  }

  return QVariant();
}

QVariant ValgrindModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation)

  switch (role) {
    case Qt::DisplayRole:
      switch (section) {
        //case Index:
          //return i18n("Item");
        case Function:
          return i18n("Function");
        case Source:
          return i18n("Source");
        case Object:
          return i18n("Object");
      }
      break;
  }

  return QVariant();
}

QModelIndex ValgrindModel::index ( int row, int column, const QModelIndex & p ) const
{
  if (row < 0 || column < 0 || column >= numColumns)
    return QModelIndex();

  if (p.isValid() && p.column() != 0)
    return QModelIndex();

  ValgrindItem* parent = itemForIndex(p);

  if (!parent) {
    if (row < errors.count())
        return createIndex(row, column, errors.at(row));

  } else if (ValgrindError* e = dynamic_cast<ValgrindError*>(parent)) {
    if (row < 2)
      return createIndex(row, column, row ? e->auxStack : e->stack);

  } else if (ValgrindStack* s = dynamic_cast<ValgrindStack*>(parent)) {
    if (row < s->frames.count())
      return createIndex(row, column, s->frames[row]);
  }

  return QModelIndex();
}

QModelIndex ValgrindModel::parent ( const QModelIndex & index ) const
{
  ValgrindItem* item = itemForIndex(index);
  if (!item)
    return QModelIndex();

  return indexForItem(item->parent(), 0);
}

int ValgrindModel::rowCount ( const QModelIndex & p ) const
{
  if (!p.isValid())
    return errors.count();

  if (p.column() != 0)
    return 0;

  ValgrindItem* parent = itemForIndex(p);

  if (ValgrindError* e = dynamic_cast<ValgrindError*>(parent))
    if (e->stack && e->auxStack)
      return 2;
    else if (e->stack)
      return 1;
    else
      return 0;

  else if (ValgrindStack* s = dynamic_cast<ValgrindStack*>(parent))
    return s->frames.count();

  return 0;
}

void ValgrindModel::clear( )
{
  m_state = Root;
  m_depth = 0;
  m_currentError = 0L;
  m_currentStack = 0L;
  m_currentFrame = 0L;
  m_stateStack.clear();
  m_buffer.clear();
  m_protocolVersion = pid = ppid = -1;
  tool.clear();
  userComment.clear();
  preamble.clear();
  valgrindArgs.clear();
  programArgs.clear();

  qDeleteAll(errors);
  errors.clear();

  reset();
}

QModelIndex ValgrindModel::indexForItem( ValgrindItem* item, int column ) const
{
  int index = -1;

  if (ValgrindError* e = dynamic_cast<ValgrindError*>(item))
    index = e->parent()->errors.indexOf(e);
  else if (ValgrindStack* s = dynamic_cast<ValgrindStack*>(item))
    index = (s == s->parent()->stack) ? 0 : 1;
  else if (ValgrindFrame* f = dynamic_cast<ValgrindFrame*>(item))
    index = f->parent()->frames.indexOf(f);

  if (index != -1)
    return createIndex(index, column, item);

  return QModelIndex();
}

ValgrindItem* ValgrindModel::itemForIndex(const QModelIndex& index) const
{
  if (index.internalPointer())
    return static_cast<ValgrindItem*>(index.internalPointer());

  return 0L;
}

KUrl ValgrindFrame::url() const
{
  KUrl url(KUrl::fromPath(dir), file);
  url.cleanPath();
  return url;
}

ValgrindModel::ValgrindModel(QObject * parent)
    : QAbstractItemModel(parent)
{
  //new ModelTest(this);
}

QString ValgrindError::whatForStack(const ValgrindStack * s) const
{
  if (s == stack)
    return what;
  if (s == auxStack)
    return auxWhat;

  return "<INTERNAL ERROR>";
}

QString ValgrindStack::what() const
{
  return parent()->whatForStack(this);
}

void ValgrindModel::parse()
{
    while (!atEnd()) {
        switch (readNext()) {
            case StartDocument:
                clear();
                break;

            case StartElement:
                startElement();
                break;

            case EndElement:
                endElement();
                break;

            case Characters:
                m_buffer += text().toString();
                break;

            default:
                break;
        }
    }

    if (hasError()) {
        switch (error()) {
            case CustomError:
            case UnexpectedElementError:
            case NotWellFormedError:
                KMessageBox::error(qApp->activeWindow(), i18n("Valgrind XML Parsing: error at line %1, column %2: %3", lineNumber(), columnNumber(), errorString()), i18n("Valgrind Error"));
                break;

            case NoError:
            case PrematureEndOfDocumentError:
                break;
        }
    }
}

#include "valgrindmodel.moc"

