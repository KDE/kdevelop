/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens@krypton.supernet                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __REPLACE_WIDGET_H__
#define __REPLACE_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;

class ReplacePart;
class ReplaceDlg;

class QPushButton;
//class QProgressBar;
//class QListView;
class QListViewItem;
class KListView;

class ReplaceItem;

class ReplaceWidget : public QWidget
{
    Q_OBJECT

public:

    ReplaceWidget(ReplacePart *part);

    QPushButton * _cancel;
    QPushButton * _replace;
    // QProgressBar * _progress;
    KListView * _listview;

public slots:
    void showDialog();
    void find();
    void replace();
    void clear();

    void clicked( QListViewItem * );

private:
    void makeReplacements( QString const & pattern, QString const & replacement );
    void showReplacements( QStringList files, QString pattern, QString replacement );
    void reloadOpenFiles();

    QString relativeProjectPath( QString );
    QString fullProjectPath( QString );
    QStringList workFiles();
    QStringList openEditorPaths();
    QStringList modifiedEditorPaths();
    QStringList getEditorPaths( bool );

    ReplacePart * m_part;
    ReplaceDlg * m_dialog;
};


#endif
