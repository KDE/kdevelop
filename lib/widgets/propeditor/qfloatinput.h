/**
 * qfloatinput.h
 *
 * Copyright (C)  2004  David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#ifndef QFLOATINPUT_H
#define QFLOATINPUT_H

#include <qapplication.h>
#include <qspinbox.h>
#include <qvalidator.h>

class QFloatInput : public QSpinBox
{
public:
    QFloatInput( int min, int max, float step, int digits,
                  QWidget *parent, const char *name = 0 );

    virtual QString  mapValueToText( int value );

    virtual int mapTextToValue( bool* ok );

    int digits() const
    {
        return m_digits;
    }

private:
    int m_digits;
};


#endif
