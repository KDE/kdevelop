/*
  kswallow.cpp - a widget to swallow a program

  written 1997 by Matthias Hoelzer

  Modified 1998 by Thomas Tanghus for KProject.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */

#include <kapp.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <qtimer.h>

#include "kswallow.h"


KSwallowWidget::KSwallowWidget(QString exestring, QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  app = exestring;
  init();
}

KSwallowWidget::KSwallowWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  init();
}

void KSwallowWidget::setExeString(QString exestring)
{
  app = exestring;
}

void KSwallowWidget::init()
{
  ksw_save = XInternAtom(qt_xdisplay(),"KSW_SAVE",False);
  ksw_exit = XInternAtom(qt_xdisplay(),"KSW_EXIT",False);
  window = 0;
  setFocusPolicy(StrongFocus);
  //  resize(480,480);
  process = 0;
  show();
}

void KSwallowWidget::resizeEvent(QResizeEvent*)
{
  if (window != 0)
    XResizeWindow(qt_xdisplay(), window, width(), height());
}

void KSwallowWidget::closeEvent(QCloseEvent *e)
{
  sWClose(false);
  if(window == 0 && process == 0)
    e->accept();
  else
    e->ignore();
}

void KSwallowWidget::focusInEvent( QFocusEvent * ){
  // workarund: put the focus onto the swallowed widget (ettrich)
  // TODO: When we switch to a newer qt than qt-1.33 this hack should
  // be replaced with my new kswallow widget!
  if (isActiveWindow() && isVisible()){ // isActiveWindow is important here!
    // verify wether the window still belongs to us
    unsigned int nwins;
    Window dw1, dw2, *wins;
    XQueryTree(qt_xdisplay(), winId(), 
	       &dw1, &dw2, &wins, &nwins);
    if (nwins)
      XSetInputFocus(qt_xdisplay(), window, RevertToParent, CurrentTime);
  }
}

void KSwallowWidget::swallowWindow(Window w)
{
  debug("\nKSwallowWidget::swallowWindow\n");
  window = w;

#warning FIXME Swallowing apps??
//  KWM::prepareForSwallowing(w);

  XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
  XMapRaised(qt_xdisplay(), w);
  XResizeWindow(qt_xdisplay(), window, width(), height());

  debug("\nKSwallowWidget::swallowWindow\n");
  //show();
}

void KSwallowWidget::sWExecute()
{
  debug("slotExecute");
  if(!app.length()) return;
  if (!process)
  {
    // Create process object
    process = new KProcess();
    if (!process)
      return;

    // split Exec entry 
    QString executable(app.data());
    QString params;
    int pos = executable.find(' ');
      
    if (pos > 0)
    {
      params = executable.right(executable.length()-pos-1);
      executable.truncate(pos);
    }

    // set executable
    process->setExecutable(executable.data());
    swallowTitle = executable + "_swallow";

//    debug("Args: %s", arg.data());

    *process << "-caption" << swallowTitle;


    QString par;
    while (params.length()>0)
    {
      if (params[0]==' ')
      {
        if (!par.isEmpty())  
          *process << par;
        par = "";
      } 
      else
      {
        if (params[0]=='"')
        {
          params.remove(0,1);
          while (!params.isEmpty() && params[0] != '"')
          {
            par += params[0];
            params.remove(0,1);
          } 
        }
        else  par += params[0];
      }
      params.remove(0,1);
    }
    if (!par.isEmpty())
      *process << par;
       


#warning FIXME Swallowing apps??
    // Avoid flickering a la kwm! (ettrich)
//    KWM::doNotManage(swallowTitle);

    // connect to KWM events to get notification if window appears
    connect(kapp, SIGNAL(windowAdd(Window)), this, SLOT(addWindow(Window)));
  }

  QObject::connect(process, SIGNAL(processExited(KProcess *)), this,
            SLOT(processExit(KProcess *)));

  // start process
  process->start();
}

void KSwallowWidget::processExit(KProcess *proc)
{
  debug("\nKSwallowWidget::processExit\n");
  if (proc == process)
    {
      delete process;
      process = 0;
      window = 0;
      close();
/*
      if (visibleWidget && visibleWidget == swallowWidget){
	//Workaround /see kswallow.cpp (ettrich)
	XSetInputFocus(qt_xdisplay(), visibleWidget->topLevelWidget()->winId(), 
		       RevertToPointerRoot, CurrentTime);
	visibleWidget = 0;
      }
*/
    }
}

void KSwallowWidget::addWindow(Window w)
{
  debug("\nKSwallowWidget::addWindow\n");
  XTextProperty titleProperty;

  XGetWMName(qt_xdisplay(), w, &titleProperty);

  if (strcmp(swallowTitle.data(), (char *) titleProperty.value) == 0)
  {
    swallowWindow(w);
    //setFocus(); //workaround (ettrich)

    // disconnect from KWM events
    disconnect(kapp, SIGNAL(windowAdd(Window)), this, SLOT(addWindow(Window)));
    //QTimer *t = new QTimer(this);
    //connect(t, SIGNAL(timeout()), SLOT(slotSave()));
    //t->start(5000);
  }  
  debug("\nKSwallowWidget::addWindow - done\n");
}

void KSwallowWidget::sWSave()
{
  if(window)
    sendClientMessage(window, ksw_save, 0);
}

void KSwallowWidget::sWClose(bool save)
{
  if(window)
  {
    if(save)
      sendClientMessage(window, ksw_save, 0);
    sendClientMessage(window, ksw_exit, 0);
#warning FIXME Swallowing apps??
//    KWM::close(window);
  }
}

void KSwallowWidget::sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = 0; //timeStamp();
  mask = 0L;
  //if (w == qt_xrootwin())
  //  mask = SubstructureRedirectMask;        /* magic! */
  XSendEvent(qt_xdisplay(), w, False, mask, &ev);
}


