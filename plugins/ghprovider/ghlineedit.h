/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GH_LINEEDIT_H
#define GH_LINEEDIT_H


#include <QLineEdit>


namespace gh
{

/**
 * @class LineEdit
 *
 * This class is the Line Edit used in the gh::ProviderWidget. It's basically
 * the same as the QLineEdit class but it emits the returnPressed() signal
 * when the return key has been pressed. Moreover, it also implements an
 * internal timer that emits the returnPressed signal when 0.5 seconds have
 * passed since the user pressed a key.
 */
class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /// Constructor.
    explicit LineEdit(QWidget *parent = nullptr);

    /// Destructor.
    ~LineEdit() override;

protected:
    /// Overridden from QLineEdit.
    void keyPressEvent(QKeyEvent *e) override;

private Q_SLOTS:
    /// The timer has timed out: stop it and emit the returnPressed signal.
    void timeOut();

private:
    QTimer *m_timer;
};

} // End of namespace gh


#endif // GH_LINEEDIT_H
