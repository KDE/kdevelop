/**************************************************************************
*                        kfiledndiconview.h  -  description
*                          -------------------
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

#ifndef KFILEDNDICONVIEW_H
#define KFILEDNDICONVIEW_H

#include <qwidget.h>
#include <qtimer.h> 
#include <kfileiconview.h>
#include <kurldrag.h>

/**
 * This is a fileview inherited from @ref KFileIconView.
 * It adds "drag and drop" suitable for a @ref KFileIconView
 * No special setup is needed, just connect to the @ref dropped signals
 * and the view will take care of the rest.
 * @short Drag and drop" aware @ref KFileIconView	
 * @author Björn Sahlström
*/

class KFileDnDIconView : public KFileIconView
{
	Q_OBJECT
public:  // Public methods

	KFileDnDIconView( QWidget *parent = 0, const char *name = 0 );
	virtual ~KFileDnDIconView();
	
	/**
	 * Set if Drag'n drop should be enabled or not, default is enabled.
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
	 * Sets the auto open time, which means the time that will
	 * elapse before a directory is automatically opened after entered by DnD.
	 * Only need to call this if you want to change the predefined time that is 750 ms.
	 * This also calls @ref #useAutoOpenTimer so no need to call this to.
	*/
	void setAutoOpenTime( const int& time );
	
	/**
	 * Set this to true if you want the view to use it's auto open functionallity otherwhise set it to false.
	 * By default this is turned ON.
	 */
	void useAutoOpenTimer( bool on = true );
	
	/**
	 * @returns true if auto open functionallity is turned ON (default), otherwhise false
	 * @see #useAutoOpenTimer
	 * @see #setAutoOpenTime
	 */
	bool isAutoOpening() const
	{
		return m_useAutoOpenTimer;
	}

	virtual void readConfig( KConfig*, const QString& group = QString::null );
	virtual void writeConfig( KConfig*, const QString& group = QString::null );
signals:
	
	/**
	 * Emitted whenever an decodable item is dropped in the view.
	 * Note: The @ref QDropEvent contains a @ref KURLDrag object.
	 */
	void dropped( QDropEvent* );
	
protected slots:
	/**
	 * Called when the auto timer times out. Open the current folder.
	 */
	void slotOpenFolder();
	
protected:
	virtual void contentsDragEnterEvent( QDragEnterEvent *e );
	virtual void contentsDragMoveEvent( QDragMoveEvent *e );
	virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
	virtual void contentsDropEvent( QDropEvent* e );
	
	/**
	 * Creates a @ref QDragObject containing all urls of the selected @ref KFileItem of the view,
	 * @returns the @ref QDragObject
	 */
	virtual QDragObject* dragObject();
	
	/**
	 *Creates the drag item and starts the drag
	 */
	virtual void startDrag();
	
	/**
	 *@returns true if we can decode the drag and support the action
	 */
	virtual bool acceptDrag( QDropEvent* event ) const;
	
protected:
	
	QTimer m_autoOpenTimer;
	int m_autoOpenTime;
	bool m_useAutoOpenTimer;
	QIconViewItem* m_dropItem;
	bool m_dndEnabled;
};
#endif 
// kate: indent-mode csands; tab-width 4; auto-insert-doxygen on;

