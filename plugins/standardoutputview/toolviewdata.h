/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TOOLVIEWDATA_H
#define KDEVPLATFORM_PLUGIN_TOOLVIEWDATA_H

#include <outputview/ioutputview.h>

#include <QObject>
#include <QMap>

namespace Sublime
{
class View;
}

class QAbstractItemModel;
class StandardOutputView;
class ToolViewData;

class OutputData : public QObject
{
Q_OBJECT
public:
    explicit OutputData( ToolViewData* tv );
    QAbstractItemDelegate* delegate;
    QAbstractItemModel* model;
    KDevelop::IOutputView::Behaviours behaviour;
    QString title;
    int id;
    void setModel( QAbstractItemModel* model );
    void setDelegate( QAbstractItemDelegate* delegate );
Q_SIGNALS:
    void modelChanged( int );
    void delegateChanged( int );
};

class ToolViewData : public QObject
{
Q_OBJECT
public:
    explicit ToolViewData( QObject* parent );
    ~ToolViewData() override;
    OutputData* addOutput( int id, const QString& title, KDevelop::IOutputView::Behaviours behave );
    // If we would adhere to model-view-separation strictly, then this member would move into standardoutputview, but it is more convenient this way.
    // TODO: move into standardoutputview
    mutable QList<Sublime::View*> views;
    StandardOutputView* plugin;
    QMap<int, OutputData*> outputdata;
    QString id;
    QString title;
    QIcon icon;
    KDevelop::IOutputView::ViewType type;
    KDevelop::IOutputView::Options option;
    QList<QAction*> actionList;
Q_SIGNALS:
    void outputAdded( int );
};

#endif
