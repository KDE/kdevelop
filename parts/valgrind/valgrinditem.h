#ifndef _VALGRINDITEM_H_
#define _VALGRINDITEM_H_

#include <q3valuelist.h>
#include <qstring.h>

class ValgrindBacktraceItem
{
public:
  enum Type { Unknown, SourceCode, Library };
  ValgrindBacktraceItem( const QString& rawOutput = QString::null );
  ~ValgrindBacktraceItem();

  QString message() const { return _message; }
  QString address() const { return _address; }
  QString url() const { return _url; }
  QString function() const { return _function; }
  int line() const { return _line; }
  int pid() const { return _pid; }
  Type type() const { return _type; }
  bool isHighlighted() const { return _highlight; }
  void setHighlighted( bool h ) { _highlight = h; }
  
private:
  QString _rawOutput, _address, _url, _function, _message;
  int _line;
  int _pid;
  Type _type;
  bool _highlight;
};

class ValgrindItem
{
public:
  ValgrindItem( const QString& message );
  ~ValgrindItem();

  typedef Q3ValueList<ValgrindBacktraceItem> BacktraceList;  
  BacktraceList& backtrace() { return _backtrace; }
  const BacktraceList& backtrace() const { return _backtrace; }
  QString message() const { return _message; }
  int pid() const { return _pid; }
  
private:
  BacktraceList _backtrace;
  QString _message;
  int _pid;
};

#endif
