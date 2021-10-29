/*
    KDevelop Class viewer

    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CLASSWIDGET_H
#define KDEVPLATFORM_PLUGIN_CLASSWIDGET_H

#include <QWidget>

class ClassBrowserPlugin;
class ClassTree;
class ClassModel;
class QLineEdit;
class QTimer;

/// The class browser widget
class ClassWidget
    : public QWidget
{
    Q_OBJECT

public:
    ClassWidget(QWidget* parent, ClassBrowserPlugin* plugin);
    ~ClassWidget() override;

private:
    ClassBrowserPlugin* m_plugin;
    ClassModel* m_model;
    ClassTree* m_tree;
    QLineEdit* m_searchLine;
    QTimer* m_filterTimer;
    QString m_filterText;
};

#endif // KDEVPLATFORM_PLUGIN_CLASSWIDGET_H
