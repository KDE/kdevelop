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

#ifndef _PARTSELECTWIDGET_H_
#define _PARTSELECTWIDGET_H_

#include <qdialog.h>
#include <qdom.h>
#include <qstringlist.h>

class Q3ListView;
class QComboView;
class KURLLabel;
class QLabel;

/**
Plugin selection widget.
*/
class PartSelectWidget : public QWidget
{
    Q_OBJECT

public:
    PartSelectWidget( QDomDocument &projectDom, QWidget *parent=0, const char *name=0 );
    ~PartSelectWidget();

public slots:
   void accept();

signals:
   void accepted();

private slots:
    void itemSelected( Q3ListViewItem * );
    void openURL( const QString & );
    void selectProfile(Q3ListViewItem* item);
    void setAsDefault();

private:
    void init();
    void readProjectConfig();
    void saveProjectConfig();
//    void fillProfilesList();

    QDomDocument m_projectDom;

    Q3ListView * _pluginList;
//    QComboView *_pluginProfile;
    QLabel * _pluginDescription;
    KURLLabel * _urlLabel;

//    QString _profile;
};

#endif
