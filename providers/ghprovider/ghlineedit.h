/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef GH_LINEEDIT_H
#define GH_LINEEDIT_H


#include <KLineEdit>


namespace gh
{

/**
 * @class LineEdit
 *
 * This class is the Line Edit used in the gh::ProviderWidget. It's basically
 * the same as the KLineEdit class but it emits the returnPressed() signal
 * when the return key has been pressed. Moreover, it also implements an
 * internal timer that emits the returnPressed signal when 0.5 seconds have
 * passed since the user pressed a key.
 */
class LineEdit : public KLineEdit
{
    Q_OBJECT

public:
    /// Constructor.
    LineEdit(QWidget *parent = NULL);

    /// Destructor.
    virtual ~LineEdit();

    /// Overridden from KLineEdit.
    void keyPressEvent(QKeyEvent *e);

signals:
    /// Emitted when the return key has been pressed.
    void returnPressed();

private slots:
    /// The timer has timed out: stop it and emit the returnPressed signal.
    void timeOut();

private:
    QTimer *m_timer;
};

} // End of namespace gh


#endif // GH_LINEEDIT_H
