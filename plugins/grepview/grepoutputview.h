/**************************************************************************
*   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GREPOUTPUTVIEW_H
#define GREPOUTPUTVIEW_H

#include <qwidget.h>
#include <qtreeview.h>
#include <qaction.h>

#include <interfaces/iuicontroller.h>

#include "ui_grepoutputview.h"

namespace KDevelop
{
    class IStatus;
}

class GrepViewPlugin;
class GrepJob;
class GrepOutputModel;
class GrepOutputDelegate;

class GrepOutputViewFactory: public KDevelop::IToolViewFactory
{
public:
    GrepOutputViewFactory();
    virtual QWidget* create(QWidget *parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
};

class GrepOutputView : public QWidget, Ui::GrepOutputView
{
  Q_OBJECT

public:
    GrepOutputView(QWidget* parent);

    GrepOutputModel* model();
    void setMessage(const QString& msg);
    void enableReplace(bool enable);
    void setPlugin(GrepViewPlugin *plugin);

public Q_SLOTS:
    void showErrorMessage( const QString& errorMessage );
    void showMessage( KDevelop::IStatus*, const QString& message );


private:
    GrepOutputModel* m_model;
    QAction* m_apply;
    GrepViewPlugin *m_plugin;
    
private slots:
    void selectPreviousItem();
    void selectNextItem();
    void onApply();
    void showDialog();
    void expandRootElement( const QModelIndex & parent );
};

#endif // GREPOUTPUTVIEW_H
