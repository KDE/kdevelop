/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVSUBVERSIONVIEW_H
#define KDEVSUBVERSIONVIEW_H

#include "svnpart.h"
#include <kdialog.h>
// #include "ui_progress_dlg.h"
class KJob;
class QWidget;
struct KDevSubversionViewPrivate;
class KTabWidget;
class QProgressBar;

/** @class KDevSubversionView
 *  @short Displays any subversion results in a bottom-docked tabwidget.
 *  Upon any new job results are available, add a tab or print the simple notification messages.
 */
class KDevSubversionView : public QWidget/*, public Ui::SvnLogviewWidget*/ {
    Q_OBJECT
public:
    KDevSubversionView(KDevSubversionPart *part, QWidget* parent);
    virtual ~KDevSubversionView();

public Q_SLOTS:
    void printNotification( const QString &path, const QString& msg );
//     void printLog( SvnJobBase *job );
//     void printBlame( SvnJobBase *job );
//     void printDiff( SvnJobBase *job );
//     void openDiff( SvnJobBase *job );
//     void printInfo( SvnJobBase* job );
//     void printStatus( SvnJobBase* job );
//     void openCat( SvnJobBase *job );
//     void slotJobFinished( SvnJobBase *job );
    void closeCurrentTab();
private:
    KTabWidget *tab();

    KDevSubversionViewPrivate* const d;

};

#endif
