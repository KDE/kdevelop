/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "documentationview.h"
#include <QAction>
#include <QVBoxLayout>
#include <KDebug>
#include <KIcon>
#include <KLocale>

DocumentationView::DocumentationView(QWidget* parent)
 : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    mActions=new KToolBar(this);
    layout()->addWidget(mActions);
    mBack=mActions->addAction(KIcon("go-previous"), i18n("Back"));
    mForward=mActions->addAction(KIcon("go-next"), i18n("Forward"));
    
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    connect(mBack, SIGNAL(triggered()), this, SLOT(browseBack()));
    connect(mForward, SIGNAL(triggered()), this, SLOT(browseForward()));
    mCurrent=mHistory.begin();
}

void DocumentationView::browseBack()
{
    mCurrent--;
    mBack->setEnabled(mCurrent!=mHistory.begin());
    mForward->setEnabled(true);
    
    replaceView((*mCurrent)->documentationWidget());
}

void DocumentationView::browseForward()
{
    mCurrent++;
    mForward->setEnabled(mCurrent+1!=mHistory.end());
    mBack->setEnabled(true);
    
    replaceView((*mCurrent)->documentationWidget());
}

void DocumentationView::replaceView(QWidget* newView)
{
    if(layout()->count()!=1)
        layout()->takeAt(1);
    layout()->addWidget(newView);
}

void DocumentationView::showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)
{
	kDebug(9529) << "showing" << doc;
	replaceView(doc->documentationWidget(this));
    
    mBack->setEnabled( !mHistory.isEmpty() );
    mForward->setEnabled(false);
    mHistory.append(doc);
    mCurrent=mHistory.end()-1;
}
