
#ifndef _CVSOPTIONSWIDGET_H_
#define _CVSOPTIONSWIDGET_H_

#include <qwidget.h>

#include "cvsoptionswidgetbase.h"

/**
 *
 * KDevelop Authors
 **/

class CvsPart;
class ServerConfigurationWidget;
class QLabel;
class QLineEdit;
class QVBoxLayout;

class CvsOptionsWidget : public CvsOptionsWidgetBase// QWidget
{
	Q_OBJECT
public:
	CvsOptionsWidget( QWidget *parent, const char *name=0 );
	~CvsOptionsWidget();

	QString cvs() const;
	QString commit() const;
	QString update() const;
	QString add() const;
	QString remove() const;
	QString diff() const;
	QString log() const;
	QString rsh() const;

public slots:
	void accept();
/*
protected:
    QLabel* m_cvsLabel;
    QLineEdit* m_cvsEdit;
    QLabel* m_commitLabel;
    QLineEdit* m_commitEdit;
    QLabel* m_updateLabel;
    QLineEdit* m_updateEdit;
    QLabel* m_addLabel;
    QLineEdit* m_addEdit;
    QLabel* m_removeLabel;
    QLineEdit* m_removeEdit;
    QLabel* m_diffLabel;
    QLineEdit* m_diffEdit;
    QLabel* m_logLabel;
    QLineEdit* m_logEdit;
    QLabel* m_rshLabel;
    QLineEdit* m_rshEdit;

	QVBoxLayout* m_cvsOptionsWidgetLayout;
*/
private:
	ServerConfigurationWidget *m_cfgWidget;

	void readConfig();
	void storeConfig();
};

#endif
