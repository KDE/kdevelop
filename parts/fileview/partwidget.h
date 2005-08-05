/***************************************************************************
 *   Copyright (C) 2003 by KDevelop authors                                *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEVIEWPARTWIDGET_H_
#define _FILEVIEWPARTWIDGET_H_

#include <qvbox.h>

class FileViewPart;

class PartWidget : public QVBox
{
    Q_OBJECT
public:
    PartWidget( FileViewPart *part, QWidget *parent = 0 );
    virtual ~PartWidget();

    class FileTreeWidget *m_filetree;
    // Support for filters
    class KHistoryCombo *m_filter;
    class QToolButton *m_btnFilter;

    void showProjectFiles();

protected:
    virtual void focusInEvent( QFocusEvent * );

private slots:
    void slotFilterChange( const QString & nf );
    void slotBtnFilterClick();

private:
    FileViewPart *m_part;
    QString m_lastFilter;
};

#endif
