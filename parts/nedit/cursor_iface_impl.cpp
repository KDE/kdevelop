#include <stdlib.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include "cursor_iface_impl.h"


using namespace KEditor;


CursorIfaceImpl::CursorIfaceImpl(DocumentImpl *parent, Editor *editor)
  : CursorDocumentIface(parent, editor)
{
    mDocument = parent;
}


bool CursorIfaceImpl::setCursorPosition(int line, int col)
{
    kdDebug() << "NeditPart: goto line " << line << endl;
    system("nc -noask -line " + QString::number(line + 1) + " " + mDocument->filename());

  return true;
}


int CursorIfaceImpl::numberOfLines() const
{
    return 0;
}


int CursorIfaceImpl::lengthOfLine(int line) const
{
    return 0;
}


void CursorIfaceImpl::getCursorPosition(int &line, int &col)
{
    line = 0;
    col = 0;
}


#include "cursor_iface_impl.moc"
