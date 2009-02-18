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
#include <QVBoxLayout>
#include <KDebug>

DocumentationView::DocumentationView(QWidget* parent)
 : QWidget(parent)
{
	setLayout(new QVBoxLayout(this));
}

void DocumentationView::showWidget(QWidget* toAdd) const
{
	kDebug(9529) << "showing" << toAdd;
	if(layout()->count()!=0)
		layout()->takeAt(0);
	layout()->addWidget(toAdd);
}
