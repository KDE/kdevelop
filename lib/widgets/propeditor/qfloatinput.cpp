/**
 * qfloatinput.cpp
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
#include "qfloatinput.h"

#include <math.h>

QFloatInput::QFloatInput( int min, int max, float step, int digits,
                            QWidget *parent, const char *name )
    : QSpinBox( (int) (min*pow(digits,10)),
                (int) (max*pow(digits,10)),
                (int) (step*pow(digits,10)), parent, name ),
      m_digits( digits )
{
    setValue( (int) (min*pow(digits,10)) );
    delete validator();
    QDoubleValidator* validator =
        new QDoubleValidator( min,  max, m_digits, this );
    setValidator( validator );
}

QString QFloatInput::mapValueToText( int value )
{
    QString format = QString("%.%1f").arg( m_digits );
    return QString().sprintf(format.latin1(),
                             (value/(float)pow(m_digits,10)) );
}

int QFloatInput::mapTextToValue( bool* ok )
{
    return int(cleanText().toFloat(ok)*pow(m_digits,10));
}


