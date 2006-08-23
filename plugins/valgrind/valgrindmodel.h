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

#ifndef KDEV_VALGRIND_MODEL_H
#define KDEV_VALGRIND_MODEL_H

#include <QHash>
#include <QStack>
#include <QXmlDefaultHandler>
#include <QAbstractItemModel>

#include <kurl.h>

class ValgrindError;
class ValgrindFrame;
class ValgrindStack;

class ValgrindItem
{
public:
  virtual ~ValgrindItem() {}
  virtual ValgrindItem* parent() const = 0;
};

/**
 * A class which parses valgrind's XML output and presents it as an
 * item model.
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
class ValgrindModel : public QAbstractItemModel, public QXmlDefaultHandler, public ValgrindItem
{
  Q_OBJECT

public:
  virtual ~ValgrindModel();

  enum Columns {
    Index = 0,
    Function,
    Source,
    Object
  };
  static const int numColumns = 4;

  // Item
  virtual ValgrindItem* parent() const { return 0L; }

  // Model
  QModelIndex indexForItem(ValgrindItem* item, int column = 0) const;
  ValgrindItem* itemForIndex(const QModelIndex& index) const;

  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex parent ( const QModelIndex & index ) const;
  virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  // XML parsing
  virtual bool characters( const QString & ch );
  virtual bool endElement( const QString & namespaceURI, const QString & localName, const QString & qName );
  virtual bool startDocument();
  virtual bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );

  // Manipulation
  void clear();

private:
  enum State {
    Unknown,
    Root,
    Session,
    Status,
    Preamble,
    Error,
    Stack,
    Frame
  } m_state;

  QStack<State> m_stateStack;

  QString m_buffer;
  int m_depth;
  int m_protocolVersion;
  int pid;
  int ppid;
  QString tool, userComment;
  QStringList preamble;
  QHash<QString, QString> valgrindArgs, programArgs;

  enum {
    NotRunning,
    Running,
    Paused
  } state;

  QList<ValgrindError*> errors;

  ValgrindError* m_currentError;
  ValgrindStack* m_currentStack;
  ValgrindFrame* m_currentFrame;
};

class ValgrindError : public ValgrindItem
{
public:
  ValgrindError(ValgrindModel* parent);
  virtual ~ValgrindError();

  virtual ValgrindModel* parent() const { return m_parent; }

  void setKind(const QString& s);

  int uniqueId;
  int threadId;

  enum {
    Unknown,
    InvalidFree,
    MismatchedFree,
    InvalidRead,
    InvalidWrite,
    InvalidJump,
    Overlap,
    InvalidMemPool,
    UninitCondition,
    UninitValue,
    SyscallParam,
    ClientCheck,
    Leak_DefinitelyLost,
    Leak_IndirectlyLost,
    Leak_PossiblyLost,
    Leak_StillReachable
  } kind;

  QString what, auxWhat;
  int leakedBytes, leakedBlocks;

  ValgrindStack* stack;
  ValgrindStack* auxStack;
  ValgrindModel* m_parent;
};

class ValgrindStack : public ValgrindItem
{
public:
  ValgrindStack(ValgrindError* parent);
  virtual ~ValgrindStack();

  virtual ValgrindError* parent() const { return m_parent; }

  QList<ValgrindFrame*> frames;
  ValgrindError* m_parent;
};

class ValgrindFrame : public ValgrindItem
{
public:
  ValgrindFrame(ValgrindStack* parent);

  virtual ValgrindStack* parent() const { return m_parent; }

  KUrl url() const;

  int instructionPointer, line;
  QString obj, fn, dir, file;
  ValgrindStack* m_parent;
};

#endif
