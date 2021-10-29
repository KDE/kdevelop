/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_DEFINESWIDGET_H
#define KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_DEFINESWIDGET_H

#include <QWidget>

#include "idefinesandincludesmanager.h"

namespace Ui
{
class DefinesWidget;
}

namespace KDevelop
{
    class IProject;
}

class DefinesModel;

class DefinesWidget : public QWidget
{
Q_OBJECT
public:
    explicit DefinesWidget( QWidget* parent = nullptr );
    ~DefinesWidget() override;

    void setDefines( const KDevelop::Defines& defines );
    void clear();
Q_SIGNALS:
    void definesChanged( const KDevelop::Defines& defines );
private Q_SLOTS:
    // Forward defines model changes
    void definesChanged();

    // Handle Del key in defines list
    void deleteDefine();
private:
    QScopedPointer<Ui::DefinesWidget> ui;
    DefinesModel* definesModel;
};

#endif
