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

#ifndef DOCUMENTATIONVIEW_H
#define DOCUMENTATIONVIEW_H

#include <QWidget>
#include <KToolBar>
#include <interfaces/idocumentation.h>

class DocumentationView : public QWidget
{
	Q_OBJECT
	public:
		DocumentationView(QWidget* parent);
		
		void showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc);
    public slots:
        void browseForward();
        void browseBack();
    private:
        void replaceView(QWidget*);
        
        KToolBar* mActions;
        QAction* mForward;
        QAction* mBack;
        QList< KSharedPtr< KDevelop::IDocumentation > > mHistory;
        QList< KSharedPtr< KDevelop::IDocumentation > >::iterator mCurrent;
};

#endif // DOCUMENTATIONVIEW_H
