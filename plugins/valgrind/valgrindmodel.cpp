/* This file is part of KDevelop
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

ValgrindError::~ ValgrindError( )
{
  delete stack;
  delete auxStack;
}

ValgrindError::ValgrindError(ValgrindModel* parent)
  : ValgrindItem(parent)
  , uniqueId(-1)
  , threadId(-1)
  , kind(Unknown)
  , leakedBytes(0L)
  , leakedBlocks(0L)
  , stack(0L)
  , auxStack(0L)
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
  : ValgrindItem(parent)
{
}

ValgrindStack::ValgrindStack(ValgrindError* parent)
  : ValgrindItem(parent)
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

bool ValgrindModel::startElement( const QString &, const QString & localName, const QString &, const QXmlAttributes &)
{
  m_buffer.clear();

  State newState = Unknown;

  switch (m_state) {
    case Root:
      if (localName == "valgrindoutput")
        newState = Session;
      break;

    case Session:
      if (localName == "status")
        newState = Status;
      else if (localName == "preamble")
        newState = Preamble;
      else if (localName == "error") {
        newState = Error;
        m_currentError = new ValgrindError(this);
      }
      break;

    case Error:
      if (localName == "stack") {
        newState = Stack;
        m_currentStack = new ValgrindStack(m_currentError);
      }
      break;

    case Stack:
      if (localName == "frame") {
        newState = Frame;
        m_currentFrame = new ValgrindFrame(m_currentStack);
      }
      break;

    default:
      break;
  }

  m_stateStack.push(newState);
  m_state = newState;
  ++m_depth;
}

bool ValgrindModel::startDocument( )
{
  m_state = Root;
  m_depth = 0;
  m_currentError = 0L;
  m_currentStack = 0L;
  m_currentFrame = 0L;

  reset();
}

bool ValgrindModel::endElement( const QString & namespaceURI, const QString & localName, const QString & qName )
{
  m_state = m_stateStack.pop();

  switch (m_state) {
    case Root:
      if (localName == "m_protocolVersion")
        m_protocolVersion = m_buffer.toInt();
      else if (localName == "pid")
        pid = m_buffer.toInt();
      else if (localName == "ppid")
        ppid = m_buffer.toInt();
      else if (localName == "tool")
        tool = m_buffer;
      else if (localName == "usercomment")
        userComment = m_buffer;
      else if (localName == "error") {
        errors.append(m_currentError);
        m_currentError = 0L;
      }
      break;

    case Preamble:
      if (localName == "line")
        preamble.append(m_buffer);
      break;

    case Error:
      if (localName == "unique")
        m_currentError->uniqueId = m_buffer.toInt(0L, 16);
      else if (localName == "tid")
        m_currentError->threadId = m_buffer.toInt();
      else if (localName == "kind")
        m_currentError->setKind(m_buffer);
      else if (localName == "what")
        m_currentError->what = m_buffer;
      else if (localName == "leakedbytes")
        m_currentError->leakedBytes = m_buffer.toInt();
      else if (localName == "leakedblocks")
        m_currentError->leakedBlocks = m_buffer.toInt();
      else if (localName == "auxwhat")
        m_currentError->auxWhat = m_buffer;
      else if (localName == "stack") {
        m_currentError->stack = m_currentStack;
        m_currentStack = 0L;
      } else if (localName == "auxstack") {
        m_currentError->auxStack = m_currentStack;
        m_currentStack = 0L;
      }
      break;

    case Stack:
      if (localName == "frame") {
        m_currentStack->frames.append(m_currentFrame);
        m_currentFrame = 0L;
      }
      break;

    case Frame:
      if (localName == "ip")
        m_currentFrame->instructionPointer = m_buffer.toInt(0L, 16);
      else if (localName == "obj")
        m_currentFrame->obj = m_buffer;
      else if (localName == "fn")
        m_currentFrame->fn = m_buffer;
      else if (localName == "dir")
        m_currentFrame->dir = m_buffer;
      else if (localName == "file")
        m_currentFrame->file = m_buffer;
      else if (localName == "line")
        m_currentFrame->line = m_buffer.toInt();
      break;

    default:
      break;
  }

  --m_depth;
}

bool ValgrindModel::characters( const QString & ch )
{
  m_buffer += ch;
  return true;
}

int ValgrindModel::columnCount ( const QModelIndex & parent ) const
{
}

QVariant ValgrindModel::data ( const QModelIndex & index, int role ) const
{
}

QModelIndex ValgrindModel::index ( int row, int column, const QModelIndex & parent ) const
{
}

QModelIndex ValgrindModel::parent ( const QModelIndex & index ) const
{
}

int ValgrindModel::rowCount ( const QModelIndex & parent ) const
{
}

ValgrindItem::ValgrindItem(ValgrindItem* parent)
  : m_parent(parent)
{
}

ValgrindItem* ValgrindItem::parent() const
{
  return m_parent;
}

#include "valgrindmodel.moc"
