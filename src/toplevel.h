/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qglobal.h>
#include <qptrlist.h>
#include <kparts/mainwindow.h>
#include "statusbar.h"
#include "kdevcore.h"

class Splitter;
class QSplitter;
class QTabWidget;


class TopLevel : public KParts::MainWindow
{
    Q_OBJECT
    
public:
    TopLevel(QWidget* parent=0, const char *name=0);
    ~TopLevel();

    // Updated to public, so it is accessible from the core class
    void createGUI(KParts::Part *part);
    void closeReal();

    StatusBar *statusBar() const
    { return static_cast<StatusBar*>(QMainWindow::statusBar()); }
    
signals:
    void wantsToQuit();
    
public slots:
    void splitDocumentWidget(QWidget *w, QWidget *old, Orientation orient);
    void embedDocumentWidget(QWidget *w, QWidget *old);
    void embedToolWidget(QWidget *w, KDevCore::Role role, const QString &shortCaption);
    void raiseWidget(QWidget *w);

private slots:
void splitterCollapsed(Splitter *splitter);
    void slotToggleSelectViews();
    void slotToggleOutputViews();
    void slotOptionsEditToolbars();

private:
    virtual bool queryClose();

    bool closing;
    QSplitter *vertSplitter, *horzSplitter;
    QTabWidget *leftTabGroup, *lowerTabGroup;
    Splitter *mainSplitter;
    QPtrList<QWidget> leftWidgets, lowerWidgets;
};

#endif
