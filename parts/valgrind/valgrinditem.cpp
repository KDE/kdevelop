#include "valgrinditem.h"

#include <qregexp.h>
#include <qstringlist.h>

#include <kdebug.h>

ValgrindBacktraceItem::ValgrindBacktraceItem( const QString& rawOutput ): _rawOutput( rawOutput ), _highlight( false )
{
  QRegExp re1( "^==(\\d+)==\\s+(by|at) (0x[\\dABCDEF]+): (.*) \\((.*):(\\d+)\\)$" );
  QRegExp re2( "^==(\\d+)==\\s+(by|at) (0x[\\dABCDEF]+): (.*) \\(in (.*)\\)$" );
  QRegExp valRe( "==(\\d+)== (.*)" );
  if ( valRe.search( _rawOutput ) >= 0 )
    _message = valRe.cap( 2 );
  if ( re1.search( _rawOutput ) >= 0 ) {
    _type = SourceCode;
    _pid = re1.cap( 1 ).toInt();
    _address = re1.cap( 3 );
    _function = re1.cap( 4 );
    _url = re1.cap( 5 );
    _line = re1.cap( 6 ).toInt();
  } else if ( re2.search( _rawOutput ) >= 0 ) {
    _type = Library;
    _pid = re2.cap( 1 ).toInt();
    _address = re2.cap( 3 );
    _function = re2.cap( 4 );
    _url = re2.cap( 5 );
    _line = -1;
  } else {
    _type = Unknown;
    _line = -1;
    _pid = -1;
  }
}

ValgrindBacktraceItem::~ValgrindBacktraceItem()
{
}


ValgrindItem::ValgrindItem( const QString& message ): _pid(-1)
{
  QRegExp valRe( "==(\\d+)== (.*)" );
  QStringList lines = QStringList::split( "\n", message );
  QString curMsg;
  
  for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it ) {
    if ( valRe.search( *it ) < 0 ) {
      kdDebug() << "ValgrindItem: got unrecognizable line '" << *it << "'" << endl;
      continue; // not of interest
    }
    if ( _pid == -1 )
      _pid = valRe.cap( 1 ).toInt();
    curMsg = valRe.cap( 2 );

    if ( curMsg.startsWith( " " ) ) {
      _backtrace.append( ValgrindBacktraceItem( *it ) );
    } else {
      if ( !_message.isEmpty() )
        _message += "\n";
      _message += curMsg;
    }
  }
//  static int i = 0;
//  kdDebug() << "got: " << ++i << ": " << _message << endl << message << endl;
}


ValgrindItem::~ValgrindItem()
{
}
