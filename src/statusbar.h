/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _STATUSBAR_H_
#define _STATUSBAR_H_

#include <kstatusbar.h>

class QLabel;


/**
 * A special status bar adjusted to our needs. Its main
 * uses:
 * @li Use message() to show a message in the status bar.
 *     It will be shown until message() is called again,
 *     or for 1.5 seconds if a widget is inserted.
 * @li Create a widget with this object as a parent.
 *     The widget will then be embedded. Embedded widgets
 *     have priority compared to messages. That means,
 *     as long as a widget is embedded, messages will
 *     be shown only for a short time.
 *
 * Embedded widgets are responsible for deleting themselves.
 */
class StatusBar : public KStatusBar
{
    Q_OBJECT
    
public:
    StatusBar( QWidget *parent=0, const char *name=0 );
    ~StatusBar();

public slots:
    void message(const QString &str);


    void setEditorStatusVisible(bool visible);
    void setStatus(const QString &str);
	void setCursorPosition(int line, int col);
	void setModified(bool isModified);


protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual bool event(QEvent *e);

private slots:
    void timeout();
    
private:
    QString msg;
    QWidget *widget;
    QTimer *timer;

	QLabel *_modified;
	QLabel *_cursorPosition;
	QLabel *_status;

};

#endif
