/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MAKEWIDGET_H_
#define _MAKEWIDGET_H_

#include <qstack.h>
#include <processview.h>


class MakeView;
class KDevEditorManager;


class MakeWidget : public ProcessView
{
    Q_OBJECT
public:
    MakeWidget(MakeView *part);
    ~MakeWidget();

    void setEditorManager(KDevEditorManager *em);
    void startJob(const QString &dir, const QString &command);

public slots:
    void nextError();
    void prevError();
    
private slots:
    void lineHighlighted(int line);

private:
    virtual void childFinished(bool normal, int status);
    virtual void insertStdoutLine(const QString &line);
    virtual void insertStderrLine(const QString &line);
    
private:
    MakeView *m_part;
    KDevEditorManager *m_editormanager;
    QStack<QString> dirstack;
};

#endif
