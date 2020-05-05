/* This file is part of KDevelop

   Copyright 2020 Friedrich W. H. Kossebau <kossebaukde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CLANGTIDY_CUSTOMCHECKSETCONFIGPROXYWIDGET_H
#define CLANGTIDY_CUSTOMCHECKSETCONFIGPROXYWIDGET_H

#include <QWidget>

namespace ClangTidy
{

/**
 * Dummy widget to integrate processing of the custom checks with KConfigDialogManager
 * It's only purpose is to be part of the config form widget hierarchy and by that being picked up
 * by KConfigDialogManager scanning widgets for those with kcfg_* object names, to use them for
 * reading & writing values of the matching items of the ConfigSkeleton.
 */
class CustomCheckSetConfigProxyWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(
        QString checks
        READ checks
        WRITE setChecks
        NOTIFY checksChanged
        USER true)

public:
    explicit CustomCheckSetConfigProxyWidget(QWidget* parent = nullptr);
    ~CustomCheckSetConfigProxyWidget() override;

public:
    QString checks() const;

    void setChecks(const QString& checks);

Q_SIGNALS:
    void checksChanged(const QString& checks);

private:
    QString m_checks;
};

}

#endif
