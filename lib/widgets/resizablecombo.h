/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef RESIZABLECOMBO_H
#define RESIZABLECOMBO_H

#include <qpushbutton.h>
#include <qpoint.h>

class KComboView;
class QMouseEvent;
class MyPushButton;

class ResizableCombo: public QWidget{
    Q_OBJECT
public:
    ResizableCombo(KComboView *view, QWidget *parent = 0, const char *name = 0);

public slots:
    void startResize();
    void stopResize();

private:
    bool m_resizing;
    MyPushButton *m_sizer;
    KComboView *m_combo;

friend class MyPushButton;
};

class MyPushButton: public QPushButton
{
public:
    MyPushButton(ResizableCombo *parent = 0, const char *name = 0 );

    QPoint pressedPos()
    {
        return m_pressedPos;
    }

protected:
    virtual void mousePressEvent ( QMouseEvent * e );
    virtual void mouseMoveEvent ( QMouseEvent * e );

private:
    QPoint m_pressedPos;
    int m_width;
    ResizableCombo *m_combo;
};

#endif
