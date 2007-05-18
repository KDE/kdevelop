/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVSUBVERSIONVIEW_H
#define KDEVSUBVERSIONVIEW_H

#include "subversion_part.h"
#include <kdialog.h>
// #include "ui_uiprogress_dlg.h"
class KJob;
class QWidget;
struct KDevSubversionViewPrivate;
class QModelIndex;
class KTabWidget;
class SvnLogHolder;
class SvnBlameHolder;
class QProgressBar;
class KUrl;

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
    void printNotification( QString msg );
    void printLog( SvnKJobBase *job );
    void printBlame( SvnKJobBase *job );
    void printDiff( SvnKJobBase *job );
    void slotJobFinished( SvnKJobBase *job );
    void closeCurrentTab();
private:
    KTabWidget *tab();

    KDevSubversionViewPrivate* const d;

};

///////////////////////////////////////////////////////////////////////////

/** @short Progress display dialog for subversion ASync jobs
 */
class SvnProgressDialog : public KDialog /*public QDialog, public Ui::SvnProgressDialog*/
{
public:
    explicit SvnProgressDialog( QWidget *parent, const QString &caption = QString() );
    virtual ~SvnProgressDialog();

    QProgressBar* progressBar();
    void setSource( const QString &src );
    void setDestination( const QString &dest );
private:
	class SvnProgressDialogPrivate;
	SvnProgressDialogPrivate* const d;

};

#endif
