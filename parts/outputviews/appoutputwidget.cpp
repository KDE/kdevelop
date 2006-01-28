/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (C) 2003 by Hamish Rodda                                    *
 *   meddie@yoyo.its.monash.edu.au                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appoutputwidget.h"

#include <qregexp.h>
#include <qmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kconfig.h>
#include <q3buttongroup.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <kglobal.h>

#include "appoutputviewpart.h"
#include "filterdlg.h"
#include "kdevdocumentcontroller.h"
#include "kdevmainwindow.h"

AppOutputWidget::AppOutputWidget(AppOutputViewPart* part)
  : ProcessWidget(0), m_part(part),
    assertMatch(QLatin1String("ASSERT: \\\"([^\\\"]+)\\\" in ([^\\( ]+) \\(([\\d]+)\\)")),
    lineInfoMatch(QLatin1String("\\[([^:]+):([\\d]+)\\]"))
{
  setObjectName(QString::fromUtf8("app output widget"));
  setContextMenuPolicy(Qt::CustomContextMenu);

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  setFont(config->readFontEntry("OutputViewFont"));

  connect(this, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotRowSelected(QListWidgetItem*)));
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)));
}


AppOutputWidget::~AppOutputWidget()
{
}

void AppOutputWidget::childFinished(bool normal, int status)
{
  ProcessWidget::childFinished(normal, status);
}

void AppOutputWidget::slotRowSelected(QListWidgetItem* row)
{
  if (row)
    {
      if (assertMatch.exactMatch(row->text()))
        {
          m_part->documentController()->editDocument(KUrl( assertMatch.cap(2) ), assertMatch.cap(3).toInt() - 1);
          m_part->mainWindow()->statusBar()->message(i18n("Assertion failed: %1").arg(assertMatch.cap(1)), 10000);
          m_part->mainWindow()->lowerView(this);
        }
      else if (lineInfoMatch.search(row->text()) != -1)
        {
          m_part->documentController()->editDocument(KUrl( lineInfoMatch.cap(1) ), lineInfoMatch.cap(2).toInt() - 1);
          m_part->mainWindow()->statusBar()->message(row->text(), 10000);
          m_part->mainWindow()->lowerView(this);
        }
    }
}

void AppOutputWidget::insertStdoutLine(const QString &line)
{
  kdDebug(9004) << k_funcinfo << line << endl;
  strList.append(QString("o-")+line);
  ProcessWidget::insertStdoutLine(line);
}


void AppOutputWidget::insertStderrLine(const QString &line)
{
  kdDebug(9004) << k_funcinfo << line << endl;
  strList.append(QString("e-")+line);
  ProcessWidget::insertStderrLine(line);
}


void AppOutputWidget::popupContextMenu(const QPoint &pos)
{
  //generate the popupmenu first
  QMenu popup(this);

  QAction *idNoFilter = popup.addAction( i18n("Do Not Filter Output") );
  idNoFilter->setCheckable(true);
  idNoFilter->setChecked(iFilterType == eNoFilter);

  QAction *idFilter = popup.addAction( i18n("Filter Output") );
  idFilter->setCheckable(true);
  idFilter->setChecked(iFilterType == eFilterStr || iFilterType == eFilterRegExp);

  //pop it up
  QAction *res = popup.exec(mapToGlobal(pos));

  //init the query dialog with current data
  FilterDlg dlg(this);
  dlg.filtergroup->setButton((int)iFilterType);
  dlg.cbCase->setChecked(bCS);
  dlg.leFilterStr->setText(strFilterStr);

  //did user select the filter item from the popup
  //and did he accept the filter-dialog
  if (res == idFilter || res == idNoFilter)
    {
      if (res == idFilter)
        {
          if ( dlg.exec() != QDialog::Accepted )
            return;
          //get back data from the dialog
          else if (dlg.rNoFilter->isChecked())
            iFilterType = eNoFilter;
          else if (dlg.rFilterStr->isChecked())
            iFilterType = eFilterStr;
          else if (dlg.rFilterRegExp->isChecked())
            iFilterType = eFilterRegExp;

          strFilterStr = dlg.leFilterStr->text();
          bCS = dlg.cbCase->isChecked();
        }
      else
        {
          iFilterType = eNoFilter;
        }

      //copy the first item from the listbox
      //if a programm was started, this contains the issued command
      QString strFirst;
#warning "port me"
#if 0
      if (count())
        {
          setTopItem(0);
          strFirst = item(topItem())->text();
        }
#endif

      //clear the listbox and write back the issued command
      clear();
      if (!strFirst.isEmpty())
        addItem(new ProcessListBoxItem(strFirst, ProcessListBoxItem::Diagnostic));

      //grep through the QList for items matching the filter...
      QStringList strListFound;
      if (iFilterType == eFilterStr)
        strListFound = strList.grep(strFilterStr, bCS);
      else if (iFilterType == eFilterRegExp)
        strListFound = strList.grep(QRegExp(strFilterStr, bCS, false));
      else if (iFilterType == eNoFilter)
        strListFound = strList;

    //... and reinsert the found items into the listbox
      for ( QStringList::Iterator it = strListFound.begin(); it != strListFound.end(); ++it )
        {
          if ((*it).startsWith(QLatin1String("o-")))
            {
              (*it).remove(0,2);
            addItem(new ProcessListBoxItem(*it, ProcessListBoxItem::Normal));
            }
          else if ((*it).startsWith(QLatin1String("e")))
            {
              (*it).remove(0,2);
              addItem(new ProcessListBoxItem(*it, ProcessListBoxItem::Error));
            }
        }
    }
  else if (res == idNoFilter)
    {
      iFilterType = eNoFilter;
    }
}


#include "appoutputwidget.moc"
