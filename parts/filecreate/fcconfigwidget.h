/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef FCCONFIGWIDGET_H
#define FCCONFIGWIDGET_H

#include <qwidget.h>
#include <qvaluelist.h>
#include <kurl.h>

#include "fcconfigwidgetbase.h"

class FileCreatePart;
class QListView;
class QDomDocument;
class QDomElement;

namespace FileCreate{
class FileType;
}

class FCConfigWidget : public FCConfigWidgetBase
{
Q_OBJECT
public:
    FCConfigWidget(FileCreatePart * part, bool global, QWidget *parent, const char *name);

    ~FCConfigWidget();

public slots:
    void accept();
    void removetemplate_button_clicked();
    void copyToProject_button_clicked();
    void newtype_button_clicked();
    void newsubtype_button_clicked();
    void remove_button_clicked();
    void moveup_button_clicked();
    void movedown_button_clicked();
    void edittype_button_clicked();
    void newtemplate_button_clicked();
    void edittemplate_button_clicked();
    void edit_template_content_button_clicked();
    void edit_type_content_button_clicked();

protected:
    void loadGlobalConfig(QListView *view, bool checkmarks = false);
    void loadProjectConfig(QListView *view);
    void loadProjectTemplates(QListView *view);
    void saveGlobalConfig();
    void saveProjectConfig();
    void loadFileTypes(QPtrList<FileCreate::FileType> list, QListView *view, bool checkmarks = false);
    void copyTemplate(QString templateUrl, QString dest, QString destName);
    void saveConfiguration(QDomDocument &dom, QDomElement &element, bool global);

private:
    FileCreatePart * m_part;
    QPtrList<FileCreate::FileType> m_globalfiletypes;
    QPtrList<FileCreate::FileType> m_projectfiletypes;
    QPtrList<FileCreate::FileType> m_projectfiletemplates;
    bool m_global;

    QValueList<KURL> urlsToEdit;
};

#endif
