/***************************************************************************
                    cdocbrowser.h - a htmlview for kdevelop
                             -------------------                                         

    version              :                                   
    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef CDOCBROWSER_H
#define CDOCBROWSER_H


#include <qwidget.h>
#include <qstring.h>
#include <qtabdialog.h>
#include <qcombobox.h>
#include <qstrlist.h>
#include <qpopupmenu.h>
#include <qframe.h>

#include <htmlview.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kconfig.h>
#include "structdef.h"


/** 
  *the documentation browser, attention!: only a prototype
  *@author Sandy Meier
  */
class CDocBrowser : public KHTMLView {
  Q_OBJECT 
public: 
  /** construtor */
  CDocBrowser(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CDocBrowser();
  /**show a html in a htmlview,if reload true it reload a file even it is in memory*/
  void showURL(QString url,bool reload=false);
  void setDocBrowserOptions();
  QString currentURL();
	QString currentTitle();
	QPopupMenu* popup(){ return doc_pop; };	
public slots:
  void slotDocFontSize(int);
  void slotDocStandardFont(const char *);
  void slotDocFixedFont(const char *);
  void slotDocColorsChanged(const QColor&, const QColor&,
            		const QColor&, const QColor&, const bool, const bool);
  void slotPopupMenu( KHTMLView *, const char *, const QPoint & );
  void slotCopyText();
  void slotURLBack();
  void slotURLForward();
  void slotSearchText();
	void slotGrepText();
	void slotSetFileTitle(const char* title);
	void slotViewInKFM();
		
 signals:
  void signalURLBack();
  void signalURLForward();
  void signalSearchText();
  void signalGrepText(QString text);
	void signalBookmarkAdd();
	void enableStop(int);	
protected:
  QString old_url;
  QString m_title;
//  void  mousePressEvent(QMouseEvent* event);
  QPopupMenu* doc_pop;


private:
	// html view preferences
	static int  fSize;
	static QString standardFont;
	static QString fixedFont;

	static QColor bgColor;
	static QColor textColor;
	static QColor linkColor;
	static QColor vLinkColor;
	static bool   underlineLinks;
	static bool   forceDefaults;

};


//-----------------------------------------------------------------------------
// Adapted from options.h & options.cpp of
//
// KDE Help Options
//
// (c) Martin R. Jones 1996
//



//-----------------------------------------------------------------------------

class CDocBrowserFont : public QWidget
{
	Q_OBJECT

public:
	CDocBrowserFont( QWidget *parent = NULL, const char *name = NULL);

public slots:
	void	slotApplyPressed();
	void	slotFontSize( int );
	void	slotStandardFont( const char *n );
	void	slotFixedFont( const char *n );

signals:
	void	fontSize( int );
	void	standardFont( const char * );
	void	fixedFont( const char * );

private:
	void	readOptions();
	void	getFontList( QStrList &list, const char *pattern );
	void	addFont( QStrList &list, const char *xfont );

private:
	int	fSize;
	QString	stdName;
	QString	fixedName;
	QStrList standardFonts;
	QStrList fixedFonts;
};

//-----------------------------------------------------------------------------

class CDocBrowserColor : public QWidget
{
	Q_OBJECT
public:
	CDocBrowserColor( QWidget *parent = NULL, const char *name = NULL );

signals:
	void	colorsChanged( const QColor &bg, const QColor &text,
                const QColor &link, const QColor &vlink, const bool underline,
                const bool forceDefaults );

protected slots:
	void	slotApplyPressed();
	void	slotBgColorChanged( const QColor &col );
	void	slotTextColorChanged( const QColor &col );
	void	slotLinkColorChanged( const QColor &col );
	void	slotVLinkColorChanged( const QColor &col );
	void    slotUnderlineLinksChanged( bool uline );
	void    slotForceDefaultChanged( bool force );

private:
	void	readOptions();

private:
	QColor bgColor;
	QColor textColor;
	QColor linkColor;
	QColor vLinkColor;
	bool   underlineLinks;
        bool   forceDefault;
	bool   changed;
};


//-----------------------------------------------------------------------------

class CDocBrowserOptionsDlg : public QTabDialog
{
	Q_OBJECT
public:
	CDocBrowserOptionsDlg( QWidget *parent = NULL, const char *name=NULL);
//	~CDocBrowserOptionsDlg();

	CDocBrowserFont *fontOptions;
	CDocBrowserColor *colorOptions;

};

#endif












