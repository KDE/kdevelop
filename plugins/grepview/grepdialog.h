/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann and the KDevelop Team           *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GREPDIALOG_H
#define GREPDIALOG_H

#include <kdialog.h>
#include "ui_grepwidget.h"

class KConfig;
class KUrlRequester;
class QPushButton;
class GrepViewPart;
class KLineEdit;

class GrepDialog : public KDialog, private Ui::GrepWidget
{
    Q_OBJECT

public:
    explicit GrepDialog( GrepViewPart * part, QWidget *parent=0 );
    ~GrepDialog();

    void setPattern(const QString &pattern);
    void setDirectory(const QString &dir);
    void setEnableProjectBox(bool enable);

    QString patternString() const;
    QString templateString() const;
    QString filesString() const;
    QString excludeString() const;
    KUrl directory() const;

    bool useProjectFilesFlag() const;
    bool regexpFlag() const;
    bool recursiveFlag() const;
    bool noFindErrorsFlag() const;
    bool caseSensitiveFlag() const;

    void showEvent(QShowEvent*);

Q_SIGNALS:
    void search();
private Q_SLOTS:
    void templateTypeComboActivated(int);
    void syncButtonClicked();
    void patternComboEditTextChanged( const QString& );
    void slotHidden();

private:
    GrepViewPart * m_part;
};


#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
