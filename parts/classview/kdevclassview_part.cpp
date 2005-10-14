/* This file is part of KDevelop
    Copyright (C) 2004, 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "kdevclassview_part.h"
#include "kdevclassviewdelegate.h"
#include "kdevcodemodel.h"
#include "kdevclassview.h"

#include <kfiltermodel.h>
#include <kdevcore.h>
#include <kdevdocumentcontroller.h>
#include <kdevmainwindow.h>
#include <urlutil.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <ktrader.h>
#include <kaboutdata.h>

#include <kparts/componentfactory.h>

#include <QtGui/QVBoxLayout>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qlineedit.h>

#include <kdevplugininfo.h>

typedef KDevGenericFactory<KDevClassViewPart> KDevClassViewFactory;
static const KDevPluginInfo data("kdevclassview");
K_EXPORT_COMPONENT_FACTORY(libkdevclassview, KDevClassViewFactory(data));

KDevClassViewPart::KDevClassViewPart(QObject *parent, const char *name, const QStringList&)
    : KDevPlugin(&data, parent)
{
  setObjectName(QString::fromUtf8(name));

  m_workspace = 0;
  m_codeModel = new KDevCodeModel(this);

  setInstance(KDevClassViewFactory::instance());

  m_widget = new QWidget(0);
  new QVBoxLayout(m_widget);

  QLineEdit *editor = new QLineEdit(m_widget);
  m_widget->layout()->addWidget(editor);

  editor->hide();

  KDevClassViewDelegate *delegate = new KDevClassViewDelegate(this);

  m_classView = new KDevClassView(this, m_widget);
  m_classView->setModel(m_codeModel);
  m_classView->setItemDelegate(delegate);
  m_classView->setWhatsThis(i18n("Class View"));
  m_widget->layout()->add(m_classView);

  //KFilterModel *filterModel = new KFilterModel(m_codeModel, m_codeModel);
  //connect(editor, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilter(QString)));
  //m_classView->setModel(filterModel);

  connect(m_classView, SIGNAL(activateURL(KURL)), this, SLOT(openURL(KURL)));

  mainWindow()->embedSelectView(m_widget, tr("Class View"), tr("Class View"));

  setXMLFile("kdevclassview.rc");

  for (int i=0; i<1000; ++i)
    {
      codeModel()->appendItem(new KDevCodeNamespaceItem(QLatin1String("Class_") + QString::number(i)));
    }
  //Q_ASSERT(0);

  //codeModel()->refresh();
}

KDevClassViewPart::~KDevClassViewPart()
{
  if (m_classView) {
    mainWindow()->removeView(m_widget);
    delete m_widget;
  }
}

KDevCodeNamespaceItem *KDevClassViewPart::currentNamespaceItem() const
{
  return m_classView->currentNamespaceItem();
}

KDevCodeClassItem *KDevClassViewPart::currentClassItem() const
{
  return m_classView->currentClassItem();
}

KDevCodeFunctionItem *KDevClassViewPart::currentFunctionItem() const
{
  return m_classView->currentFunctionItem();
}

KDevCodeVariableItem *KDevClassViewPart::currentVariableItem() const
{
  return m_classView->currentVariableItem();
}

void KDevClassViewPart::import(RefreshPolicy policy)
{
}

#include "kdevclassview_part.moc"
