/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __REPLACE_WIDGET_H__
#define __REPLACE_WIDGET_H__

#include <ktexteditor/editinterface.h>

#include <qwidget.h>
#include <qstring.h>

class QPushButton;
class QListViewItem;
class QDialog;

class ReplacePart;
class ReplaceDlg;
class ReplaceItem;
class ReplaceView;

class ReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    ReplaceWidget(ReplacePart *part);

public slots:
    void showDialog();
    void find();
    void replace();
    void clear();

    void clicked( QListViewItem * );
    void showRegExpEditor();

private:
    void showReplacements();
    void makeReplacements();

    QString relativeProjectPath( QString );
    QString fullProjectPath( QString );

    QStringList const & workFiles();
    QStringList const & allProjectFiles();
    QStringList const & subProjectFiles( QString const & );
    QStringList const & openProjectFiles();

    KTextEditor::EditInterface * getEditInterfaceForFile( QString const & file );

    ReplacePart * m_part;
    ReplaceDlg * m_dialog;
    ReplaceView * _listview;
    QDialog * _regexp_dialog;
    QPushButton * _cancel;
    QPushButton * _replace;

    QStringList _list;

};


#endif
