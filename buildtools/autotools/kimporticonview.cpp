/***************************************************************************
                             -------------------
    begin                : 19.01.2003
    copyright            : (C) 2002 by Victor Röder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfontmetrics.h>
#include <qpainter.h>
#include <qrect.h>
#include <qwidget.h>

#include <kglobalsettings.h>
#include <klocale.h>
#include <kwordwrap.h>

#include "kimporticonview.h"


KImportIconView::KImportIconView(const QString& strIntro, QWidget *parent, const char *name)
  : KFileDnDIconView(parent, name)
{
	m_strIntro = strIntro;
	m_bDropped = false;

	setAcceptDrops ( true );
}


KImportIconView::~KImportIconView()
{
}

void KImportIconView::drawContents ( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !m_bDropped)
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );

        p->save();
        QFont font ( p->font() );
        font.setBold ( true );
        font.setFamily ( "Helvetica [Adobe]" );
        font.setPointSize ( 10 );
        p->setFont ( font );
        p->setPen ( QPen ( KGlobalSettings::highlightColor() ) );

        QRect rect = frameRect();
        QFontMetrics fm ( p->font() );
        rect.setLeft ( rect.left() + 30 );
        rect.setRight ( rect.right() - 30 );

        resizeContents ( contentsWidth(), contentsHeight() );

        // word-wrap the string
        KWordWrap* wordWrap1 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, m_strIntro );
        KWordWrap* wordWrap2 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, i18n("Or just use the buttons!") );

        QRect introRect1 = wordWrap1->boundingRect();
		QRect introRect2 = wordWrap2->boundingRect();

        wordWrap1->drawText ( p, ( ( frameRect().right() - introRect1.right() ) / 2 ), ( ( frameRect().bottom() - introRect1.bottom() ) / 2 ) - 20, AlignHCenter | AlignVCenter );
        wordWrap2->drawText ( p, ( ( frameRect().right() - introRect2.right() ) / 2 ), ( ( frameRect().bottom() - introRect2.bottom() ) / 2 ) + introRect1.bottom(), AlignHCenter | AlignVCenter );

        p->restore();
    }
    else
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );
    }
}

void KImportIconView::somethingDropped ( bool dropped )
{
    m_bDropped = dropped;
}

#include "kimporticonview.moc"
