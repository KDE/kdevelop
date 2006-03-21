/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWWIDGET_H_
#define _GREPVIEWWIDGET_H_

#include "processwidget.h"
#include <qwidget.h>
//Added by qt3to4:
#include <QHBoxLayout>

class GrepDialog;
class GrepViewPart;
class KDevProject;
class KTabWidget;
class QHBoxLayout;
class QToolButton;
class GrepViewProcessWidget : public ProcessWidget
{
	Q_OBJECT
public:
	GrepViewProcessWidget(QWidget* parent) : ProcessWidget(parent) {};
	~GrepViewProcessWidget(){};
	void setMatchCount(int newCount) 
	{
		m_matchCount = newCount;
	}
	
	void incrementMatchCount(uint amount = 1)
	{
		m_matchCount += amount;
	}
	
	void setLastFileName(const QString& lastFileName)
	{
		_lastfilename = lastFileName;
	}
	
public slots:
	virtual void insertStdoutLine(const QString &line);

protected:
	virtual void childFinished(bool normal, int status);

private:
	int m_matchCount;
	QString _lastfilename;
};

class GrepViewWidget : public QWidget
{
	Q_OBJECT

public:
	GrepViewWidget(GrepViewPart *part);
	~GrepViewWidget();

	void projectChanged(KDevProject *project);
	void killJob( int signo = SIGTERM );
	bool isRunning() const;
	
public slots:
	void showDialog();
	void showDialogWithPattern(QString pattern);

private slots:
	void searchActivated();
	/**
	 * If item is a valid result of a search run, it opens the file at the position, where the stuff was found.
	 * @param item item containing filename and linenumber of the file to open.
	 */
	void slotExecuted(Q3ListBoxItem *item);
	void popupMenu(Q3ListBoxItem*, const QPoint& p);
	/**
	 * Creates a new tab containing the current output in the main tab and clears the main tab.
	 */
	void slotKeepOutput();
	/**
	 * Closes the currently active tab, if it is not the main output.
	 */
	void slotCloseCurrentOutput();
	/**
	 * Slot reacting on changes of the active tab, to activate/deactivate the close button,
	 * as the main output tab must not be closed.
	 */
	void slotOutputTabChanged();
	
	void slotSearchProcessExited();

private:

	QHBoxLayout* m_layout;
	KTabWidget* m_tabWidget;
	GrepViewProcessWidget* m_curOutput;
	GrepDialog *grepdlg;
	GrepViewPart *m_part;
	QToolButton* m_closeButton;
	QString m_lastPattern;
};

#endif
