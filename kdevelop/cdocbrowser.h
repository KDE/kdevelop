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

#include <khtml_part.h>

#include <qstringlist.h>
#include <qtabdialog.h>

class QPopupMenu;

/**
  *the documentation browser, attention!: only a prototype
  *@author Sandy Meier
  */
class CDocBrowser : public KHTMLPart
{
  Q_OBJECT 
public: 
  /** construtor */
  CDocBrowser(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CDocBrowser();
  /**show a html in a htmlview,if reload true it reload a file even it is in memory*/
  void showURL(const QString& url,bool reload=false);
  void setDocBrowserOptions();
  void doSearchDialog();

  inline const QString& currentURL() { return complete_url; }
  QString currentTitle();
  QPopupMenu* popup(){ return doc_pop; };	
	
protected:	
#if (QT_VERSION < 300)
  virtual void urlSelected( const QString &url, int button = 0, int state = 0,
                            const QString &_target = QString::null );
#else
  virtual void urlSelected( const QString &url, int button, int state,
                            const QString &_target, KParts::URLArgs args = KParts::URLArgs());
#endif

public slots:
  void slotDocBrowserOptions( void );
  void slotPopupMenu( const QString&, const QPoint & );
  void slotCopyText();
  void slotURLBack();
  void slotURLForward();
  void slotSearchText();
  void slotGrepText();
  void slotManpage();
  void slotSetFileTitle(const QString& title);
  void slotViewInKFM();

  void slotFindTextNext(QString);

signals:
  void signalURLBack();
  void signalURLForward();
  void signalSearchText();
  void signalGrepText(QString text);
  void signalManpage(const QString& text);
  void signalBookmarkToggle();
  void enableStop(int);

protected:
  QString old_url, complete_url;
  QString m_title, m_refTitle;
//  void  mousePressEvent(QMouseEvent* event);
  QPopupMenu* doc_pop;


private:
  // html view preferences
  static int  fSize;
  static QFont standardFont;
  static QFont fixedFont;

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
	void	slotStandardFont( const QString& n );
	void	slotFixedFont( const QString& n );

signals:
	void  configChanged( void );

private:
	void	readOptions();
	void	getFontList( QStringList &list, const char *pattern );
  void  getFontList( QStringList &list, bool fixed );
	void	addFont( QStringList &list, const char *xfont );

private:
	int	fSize;
	QString stdName;
	QString fixedName;
	QStringList standardFonts;
	QStringList fixedFonts;
};

//-----------------------------------------------------------------------------

class CDocBrowserColor : public QWidget
{
	Q_OBJECT
public:
	CDocBrowserColor( QWidget *parent = NULL, const char *name = NULL );

signals:
	void  configChanged( void );

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
//FB	QColor bgColor;
	QColor textColor;
	QColor linkColor;
	QColor vLinkColor;
	bool   underlineLinks;
//FB  bool   forceDefault;
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













