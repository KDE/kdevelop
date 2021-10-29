/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXPANDABLE_LINE_EDIT_H
#define EXPANDABLE_LINE_EDIT_H

#include "utilexport.h"
#include <QLineEdit>

/** This class implements a line edit widget which tries to expand its
 * width to fit typed text. When no text typed or text is too short
 * the widget's width is defined by the preferredWidth() value
 * (default is 200 px).
 */

class KDEVPLATFORMUTIL_EXPORT KExpandableLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit KExpandableLineEdit(QWidget* parent = nullptr);

    ~KExpandableLineEdit() override;

    int preferredWidth() const;
    void setPreferredWidth(int width);

    QSize sizeHint() const override;

protected:
    int m_preferredWidth = 200;
};

#endif
