/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <qtextedit.h>
#include <qlistview.h>

#include "dmainwindow.h"

#include "button.h"
#include "buttonbar.h"

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

class T: public QObject{
    Q_OBJECT
public slots:
    void widgetChanged(QWidget*w) {
        kdDebug() << "Widget is now: " << w << endl;
    }
};

int main(int argc, char **argv)
{
    KAboutData about("MyApp", I18N_NOOP("MyApp"), "1", "",
                     KAboutData::License_GPL, "(C) 2004 by me", 0, 0, "");
    about.addAuthor( "Me", 0, "" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app(argc, argv);

    DMainWindow *mainWin = new DMainWindow();
    
//    mainWin->setCentralWidget(new QTextEdit(mainWin));
    
    QTextEdit *te1 = new QTextEdit(mainWin);
    mainWin->addWidget(te1, "First");
    mainWin->addWidget(new QTextEdit(mainWin), "Second");
    mainWin->addWidget(new QTextEdit(mainWin), "Third");
    
    mainWin->splitVertical();
    mainWin->addWidget(new QTextEdit(mainWin), "Fourth");

    mainWin->splitVertical();
    QTextEdit *te5 = new QTextEdit(mainWin);
    mainWin->addWidget(te5, "Fifth");
    
    mainWin->splitHorizontal();
    QTextEdit *te = new QTextEdit(mainWin);
    mainWin->addWidget(te, "Sixth");

    mainWin->splitHorizontal();
    mainWin->addWidget(new QTextEdit(mainWin), "Seventh");

    te->setFocus();
    DTabWidget *tw = mainWin->splitVertical();
    mainWin->addWidget(new QTextEdit(mainWin), "Eighth");
    mainWin->addWidget(tw, new QTextEdit(mainWin), "Nineth");
    
    //mainWin->splitHorizontal();
    
    te1->setFocus();
    mainWin->addWidget(new QTextEdit(mainWin), "Tenth");    

    mainWin->removeWidget(te5);
    
    te1->setFocus();
    tw = mainWin->splitVertical();
    mainWin->addWidget(tw, new QTextEdit(mainWin), "11");    

    DDockWindow *dock = mainWin->toolWindow(DDockWindow::Bottom);
    dock->addWidget("Foo kjsdg lkgj sldkgj sdlgkj sdlkgj", new QListView(0));
    dock->addWidget("Boo", new QListView(0));
    dock->addWidget("Goo", new QTextEdit(0));

    dock = mainWin->toolWindow(DDockWindow::Left);
    dock->addWidget("Foo", new QListView(0));
    dock->addWidget("Boo sdkfjaskjh asfjkh as", new QListView(0));
    dock->addWidget("Goo", new QTextEdit(0));

    dock = mainWin->toolWindow(DDockWindow::Right);
    dock->addWidget("Foo", new QListView(0));
    dock->addWidget("Boo", new QListView(0));
    dock->addWidget("Goo", new QTextEdit(0));
            
/*    QWidget *w = new QWidget(0);
    w->show();
    QHBoxLayout *l = new QHBoxLayout(w, 0, 0);
    l->setAutoAdd( true);
    Ideal::ButtonBar *bar = new Ideal::ButtonBar(Ideal::Left, Ideal::Text, w);
    Ideal::Button *button = new Ideal::Button(bar, "fdfhdfh");
    bar->addButton(button);
    bar->show();
    QTextEdit *te = new QTextEdit(w);
    te->show();*/
    
    app.setMainWidget( mainWin );
    mainWin->resize(800, 600);
    mainWin->show();

    T *t = new T();
    
    mainWin->connect(mainWin, SIGNAL(widgetChanged(QWidget*)), t, SLOT(widgetChanged(QWidget*)));
    
    return app.exec();
}

#include "main.moc"
