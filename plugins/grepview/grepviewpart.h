/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWPART_H_
#define _GREPVIEWPART_H_

#include <iplugin.h>

#include <QFile>
class QStringList;
class GrepDialog;

namespace KDevelop
{
    class IOutputView;
}

// class KDialogBase;
// class QPopupMenu;
// class Context;
// class GrepViewWidget;


class GrepViewPart : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    GrepViewPart( QObject *parent, const QStringList & );
    ~GrepViewPart();

private slots:
//     void stopButtonClicked(KDevPlugin *which);
//     void projectOpened();
//     void projectClosed();
//     void contextMenu(QPopupMenu *popup, const Context *context);

    void slotGrep();
    void showDialogWithPattern(QString pattern);
    void searchActivated();
//     void slotContextGrep();

private:
    static QString escape(const QString &str);
    static QString quote( const QString &arg );
//     QPointer<GrepViewWidget> m_widget;
//     GrepViewWidget *m_widget;

    // vars from grepviewwidget
    GrepDialog *m_grepdlg;
    QString m_lastPattern;
    QFile m_tempFile;

    // vars from part
//     QString m_popupstr;

    KDevelop::IOutputView *m_view;
};

#endif
