/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _GENERALINFOWIDGET_H_
#define _GENERALINFOWIDGET_H_

#include <QWidget>
#include <QDomDocument>

#include "shellexport.h"

class QDomDocument;
namespace Ui { class GeneralInfoWidgetBase; }

/**
General project information widget.
*/
class KDEVSHELL_EXPORT GeneralInfoWidget : public QWidget {
    Q_OBJECT

public:

    GeneralInfoWidget(QDomDocument &projectDom, QWidget *parent=0);
    ~GeneralInfoWidget();

public slots:
    void accept();

private slots:
    void slotProjectDirectoryChanged( const QString& text );
    void slotProjectDirectoryComboChanged();

private:

    QDomDocument m_projectDom;
    Ui::GeneralInfoWidgetBase* m_baseUi;

    void readConfig();
    void writeConfig();

    bool isProjectDirectoryAbsolute();
    QString projectDirectory();
    void setProjectDirectoryError( const QString& error );
    void setProjectDirectorySuccess();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
