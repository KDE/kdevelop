/***************************************************************************
                          wizardbasepage.h  -  description
                             -------------------
    begin                : Sat Jun 9 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIZARDBASEPAGE_H
#define WIZARDBASEPAGE_H

#include <qhbox.h>

class QWidget;
class QString;
class CKDevInstallState;
class QLabel;
class QVBox;

class WizardBasePage : public QHBox
{
	Q_OBJECT
public:
	WizardBasePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState = 0L);

  virtual QString pageTitle() const { return m_pageTitle; };
  virtual void setPageTitle( const QString& pageTitle ) { m_pageTitle = pageTitle; };
  virtual bool valid() const { return m_valid; }
  virtual void setValid( bool state );
  virtual void setBackButtonEnabled( bool state);
  virtual void setHelpTextBackgroundColor( const QColor& bgcolor);

signals:
  void validData(QWidget* page, bool valid);
  void enableBackButton(QWidget* page, bool enable);

protected:
  QString m_pageTitle;
  bool    m_valid;
  bool    m_backButtonEnabled;
  QLabel* m_labelText;
	QVBox * m_vbox;
	CKDevInstallState* m_pInstallState;
};

#endif  // WIZARDBASEPAGE_H
