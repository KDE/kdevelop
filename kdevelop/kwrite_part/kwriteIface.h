#ifndef _KWRITE_IFACE_H_
#define _KWRITE_IFACE_H_

#include <dcopobject.h>

class KWriteIface : virtual public DCOPObject
{
   K_DCOP

k_dcop:
   virtual void cursorLeft()=0;
   virtual void shiftCursorLeft()=0;
   virtual void cursorRight()=0;
   virtual void shiftCursorRight()=0;
   virtual void wordLeft()=0;
   virtual void shiftWordLeft()=0;
   virtual void wordRight()=0;
   virtual void shiftWordRight()=0;
   virtual void home()=0;
   virtual void shiftHome()=0;
   virtual void end()=0;
   virtual void shiftEnd()=0;
   virtual void up()=0;
   virtual void shiftUp()=0;
   virtual void down()=0;
   virtual void shiftDown()=0;
   virtual void scrollUp()=0;
   virtual void scrollDown()=0;
   virtual void topOfView()=0;
   virtual void bottomOfView()=0;
   virtual void pageUp()=0;
   virtual void shiftPageUp()=0;
   virtual void pageDown()=0;
   virtual void shiftPageDown()=0;
   virtual void top()=0;
   virtual void shiftTop()=0;
   virtual void bottom()=0;
   virtual void shiftBottom()=0;

   virtual int numLines()=0;
   virtual QString text()=0;
   virtual QString currentTextLine()=0;
   virtual QString textLine(int num)=0;
   virtual QString currentWord()=0;
   virtual QString word(int x, int y)=0;
   virtual void insertText( const QString &txt, bool mark )=0;


   virtual void setCursorPosition( int line, int col, bool mark )=0;
   virtual bool isOverwriteMode() const =0;
   virtual void setOverwriteMode( bool b )=0;
 
   virtual int currentLine()=0;
   virtual int currentColumn()=0;

   virtual bool loadFile(const QString &name, int flags = 0)=0;
   virtual bool writeFile(const QString &name)=0;
};
#endif
