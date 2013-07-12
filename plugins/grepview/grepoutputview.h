/**************************************************************************
*   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H

#include <interfaces/iuicontroller.h>

#include "ui_grepoutputview.h"

namespace KDevelop
{
    class IStatus;
}

class QModelIndex;

class GrepViewPlugin;
class GrepOutputModel;
class GrepOutputDelegate;

class GrepOutputViewFactory: public KDevelop::IToolViewFactory
{
public:
    GrepOutputViewFactory(GrepViewPlugin* plugin);
    virtual QWidget* create(QWidget* parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
private:
    GrepViewPlugin* m_plugin;
};

class GrepOutputView : public QWidget, Ui::GrepOutputView
{
  Q_OBJECT

public:
    GrepOutputView(QWidget* parent, GrepViewPlugin* plugin);
    ~GrepOutputView();
    GrepOutputModel* model();
    
    /**
     * This causes the creation of a new model, the old one is kept in model history.
     * Oldest models are deleted if needed.
     * @return pointer to the new model
     */
    GrepOutputModel* renewModel(QString name, QString descriptionOrUrl);
    
    void setMessage(const QString& msg);

public Q_SLOTS:
    void showErrorMessage( const QString& errorMessage );
    void showMessage( KDevelop::IStatus*, const QString& message );
    void updateApplyState(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void changeModel(int index);
    void replacementTextChanged(QString);

Q_SIGNALS:
    void outputViewIsClosed();
    
private:
    static const int HISTORY_SIZE;
    QAction* m_next;
    QAction* m_prev;
    QAction* m_collapseAll;
    QAction* m_expandAll;
    QAction* m_clearSearchHistory;
    QLabel*  m_statusLabel;
    GrepViewPlugin *m_plugin;
    
private slots:
    void selectPreviousItem();
    void selectNextItem();
    void collapseAllItems();
    void expandAllItems();
    void onApply();
    void showDialog();
    void expandElements( const QModelIndex & parent );
    void rowsRemoved();
    void clearSearchHistory();
    void modelSelectorContextMenu(const QPoint& pos);
    void updateScrollArea( const QModelIndex &index );
    void updateCheckable();
};

#endif // KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H
