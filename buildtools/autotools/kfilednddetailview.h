/***************************************************************************
*                        kfilednddetailview.h  -  description
*                           -------------------
*  begin                : Wed Nov 1 2000
*  copyright            : (C) 2000 by Björn Sahlström
*  email                : kbjorn@users.sourceforge.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/

#ifndef KFILEDNDDETAILVIEW_H
#define KFILEDNDDETAILVIEW_H

//////////////////////////////////////////////////////////////////////
// Qt specific include files
#include <qwidget.h>
#include <qtimer.h> 
//////////////////////////////////////////////////////////////////////
// KDE specific include files
#include <kfiledetailview.h>
#include <kurldrag.h>


/**
	*This is a fileview inherited from @ref KFileDetailView.
	*It adds "drag and drop" handling to the view suitable for a @ref KFileDetailView.
	* No special setup is needed, just connect to the @ref dropped signals
	* and the view will take care of the rest.
	*@short "Drag and drop" aware @ref KFileDetailView	
	*@author Björn Sahlström
  */

class KFileDnDDetailView : public KFileDetailView
{
	Q_OBJECT
public:
	/**  */
	KFileDnDDetailView( QWidget *parent = 0, const char *name = 0 );
	/**  */
	virtual ~KFileDnDDetailView();
	/**
	* Set this to true if Drag'n drop should be enabled or not, default is enabled
	*/
	void setDnDEnabled( bool );
	/**
	* @returns wether DnD is enabled or not.
	*/
	bool isDnDEnabled() const
	{
		return m_dndEnabled;
	}
	/**
	*Sets the auto open time, which means the time that will
	*elapse before a directory is automatically opened after entered by DnD.
	*Only need to call this if you want to change the predefined time that is 750 ms.
	*This also calls @ref #useAutoOpenTimer so no need to call this to.
	*/
	void setAutoOpenTime( const int& time );
	/**
	*Set this to true if you want the view to use it's auto open functionallity otherwhise set it to false.
	*By default this is turned ON.
	*/
	void useAutoOpenTimer( bool on = true );
	/**
	*@returns true if auto open functionallity is turned ON (default), otherwhise false
	*@see #useAutoOpenTimer
	*@see #setAutoOpenTime
	*/
	bool isAutoOpening() const
	{
		return m_useAutoOpenTimer;
	}
	/** */
	virtual void readConfig( KConfig*, const QString& group = QString::null );
	/** */
	virtual void writeConfig( KConfig*, const QString& group = QString::null );
signals:  // Signals
	/**
	* Emitted whenever an decodable item is dropped in the view.
	* Note: The @ref QDropEvent contains a @ref KURLDrag object.
	*/
	void dropped( QDropEvent* );
	/**
	* Emitted whenever an decodable item is dropped in the view
	* Note: The @ref QDropEvent contains a @ref KURLDrag object.
	*/
	void dropped( KFileView*, QDropEvent* );
	/**
	* Emitted whenever an decodable item is dropped in the view.
	*@param urls contains a list of all dropped @ref KURL
	*/
	void dropped( KFileView*, KURL::List& urls );
protected slots:  // Protected slots
	/**
	* Called when the auto timer times out. Open the current folder.
	*/
	void slotOpenFolder();
protected:  //Protected Methods
	/**  */
	virtual void contentsDragEnterEvent( QDragEnterEvent *e );
	/**  */
	virtual void contentsDragMoveEvent( QDragMoveEvent *e );
	/**  */
	virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
	/**  */
	virtual void contentsDropEvent( QDropEvent* e );
	/**
	* Create dragobject encoding the current selection and starts the drag
	*/
	virtual void startDrag();
	/**
	* @returns the dragObject
	*/
	virtual QDragObject* dragObject() const;
	/**
	* @returns true if we can decode the drag and support the action
	*/
	virtual bool acceptDrag( QDropEvent* event ) const;
protected:  // Private attributes
	QTimer m_autoOpenTimer;
	int m_autoOpenTime;
	bool m_useAutoOpenTimer;
	QListViewItem* m_dropItem;
	KURLDrag* m_dragObject;
	bool m_dndEnabled;
};

#endif 
// kate: indent-mode csands; tab-width 4;
