/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PARTEXPLORERFORM_H
#define PARTEXPLORERFORM_H

#include <ktrader.h>
#include <kdialogbase.h>

class ResultsList;
class PartExplorerFormBase;

/**
* The GUI for the PartExplorer.
*
* @author Mario Scalas
*/
class PartExplorerForm : public KDialogBase
{
    Q_OBJECT
public:
    PartExplorerForm( QWidget *parent = 0 );
    virtual ~PartExplorerForm();

    /**
    * Fills the widget with data gathered for system's query.
    */
    void fillServiceList( const KTrader::OfferList &services );

public slots:
    void slotDisplayError( QString );

protected slots:
    void slotSearchRequested();

private:
    ResultsList *m_resultsList;
    PartExplorerFormBase *m_base;
};

#endif
