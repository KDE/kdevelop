/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEGLOBALCONFIGWIDGET_H_
#define _DOCTREEGLOBALCONFIGWIDGET_H_

#include "doctreeglobalconfigwidgetbase.h"

class DocTreeViewWidget;
class DocTreeViewPart;

class DocTreeGlobalConfigWidget : public DocTreeGlobalConfigWidgetBase
{
    Q_OBJECT

public:
    DocTreeGlobalConfigWidget( DocTreeViewPart *part, DocTreeViewWidget *widget, QWidget *parent, const char *name=0 );
    ~DocTreeGlobalConfigWidget();

public slots:
    void accept();

private slots:
    void updateConfigClicked();
    void updateIndexClicked();
    void runHtdig( QString arg );
    void removeBookmarkClicked();
    void addBookmarkClicked();
    void extEdit();
    void extEnable();
    void extDisable();
    void doxygenadd_button_clicked();
    void doxygenedit_button_clicked();
    void doxygenremove_button_clicked();
    void qtdocsadd_button_clicked();
    void qtdocsedit_button_clicked();
    void qtdocsremove_button_clicked();
    void kdocadd_button_clicked();
    void kdocedit_button_clicked();
    void kdocremove_button_clicked();
    void pushEdit_clicked();
    void extAddButton_clicked();
    void extRemoveButton_clicked();
    void dhAddButton_clicked();
    void dhDisableButton_clicked();
    void dhEditButton_clicked();
    void dhEnableButton_clicked();
    void dhRemoveButton_clicked();
    void dhScanButton_clicked();

private:
    void readConfig();
    void storeConfig();
    void readTocConfigs();
    void readDevHelpConfig();
    
    DocTreeViewWidget *m_widget;
    QStringList m_ignoreTocs;
    QStringList m_ignoreDevHelp;
    DocTreeViewPart *m_part;
};

#endif
