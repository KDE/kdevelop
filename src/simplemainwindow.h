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
#ifndef SIMPLEMAINWINDOW_H
#define SIMPLEMAINWINDOW_H

#include <dmainwindow.h>
#include <kdevplugin.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>

class KAction;
class MainWindowShare;

namespace KParts {
    class ReadOnlyPart;
}

class SimpleMainWindow: public DMainWindow, public KDevMainWindow {
    Q_OBJECT
public:
    SimpleMainWindow(QWidget* parent = 0, const char *name = 0);
    virtual ~SimpleMainWindow();

    virtual void embedPartView(QWidget *view, const QString &title, const QString& toolTip = QString::null);
    virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip);
    
    virtual void removeView(QWidget *view);
    virtual void setViewAvailable(QWidget *pView, bool bEnabled);
    virtual void raiseView(QWidget *view);
    virtual void lowerView(QWidget *view);

    virtual void loadSettings();
    virtual void saveSettings();
    
    virtual KMainWindow *main();
    
    void init();

protected slots:
    virtual void closeTab();
    virtual void tabContext(QWidget *w, const QPoint &p);
    
private slots:
    void gotoNextWindow();
    void gotoPreviousWindow();
    void gotoFirstWindow();
    void gotoLastWindow();
    void slotCoreInitialized();
    void projectOpened();
    void slotPartURLChanged(KParts::ReadOnlyPart *part);
    void documentChangedState(const KURL &url, DocumentState state);
    void tabContextActivated(int);
    void configureToolbars();
    void slotNewToolbarConfig();

private:
    void createFramework();
    void createActions();
    
    MainWindowShare *m_mainWindowShare;
    
    KURL m_currentTabURL;
    QMap<QWidget*, DDockWindow::Position> m_docks;
    KAction *m_raiseEditor;
};

#endif
