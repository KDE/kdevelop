/*
    SPDX-FileCopyrightText: 2003, 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEW_H
#define KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEW_H

#include <QWidget>
#include <QUrl>

class KDevKonsoleViewPlugin;


class KDevKonsoleView : public QWidget
{
    Q_OBJECT
public:
    explicit KDevKonsoleView( KDevKonsoleViewPlugin* plugin, QWidget *parent = nullptr );
    ~KDevKonsoleView() override;

public Q_SLOTS:
    void setDirectory( const QUrl &dirUrl );

protected:
    bool eventFilter( QObject *obj, QEvent *e ) override;

private :
    class KDevKonsoleViewPrivate* const d;
};

#endif

