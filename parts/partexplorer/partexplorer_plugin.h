/*
 * Copyright (C) 2003 Mario Scalas <mario@gicomsrl.it>
 */

#ifndef _PARTEXPLORERPLUGIN_H_
#define _PARTEXPLORERPLUGIN_H_

#include <ktrader.h>
#include "kdevplugin.h"

class QWidget;
class QPainter;
class KURL;
class PartExplorerForm;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Mario Scalas <mario@gicomsrl.it>
 * @version 0.1
 */
class PartExplorerPlugin : public KDevPlugin
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    PartExplorerPlugin(  QObject *parent, const char *name, const QStringList & );

    /**
     * Destructor
     */
    virtual ~PartExplorerPlugin();

signals:
	/**
	* This signal is connected to the m_widget, so the form will be the only encharged
	* for displaying error messages.
	*/
	void displayError( QString );

public slots:
	/**
	* Query KDE for registered services matching the query costraints.
	*/
	void slotSearchServices( QString, QString );
	/**
	* Show PartEplorer form.
	*/
	void slotShowWidget();

private:
	/**
	* Fills the widget with data gathered for system's query.
	*/
	void fillWidget( const KTrader::OfferList &services );
	/**
	* The dialog for user interaction, so the plugin is usable too ;)
	*/
	PartExplorerForm *m_widget;
};

#endif // _PARTEXPLORERPLUGIN_H_
