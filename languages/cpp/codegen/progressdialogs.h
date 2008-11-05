/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef PROGRESSDIALOGS_H
#define PROGRESSDIALOGS_H

#include <QProgressBar>
#include <QObject>
#include <QDialog>
#include <language/duchain/indexedstring.h>
#include <language/duchain/topducontext.h>

namespace KDevelop {
  class UsesCollector;
}

class StringProgressBar : public QProgressBar {
  public:
    virtual QString text() const;
    
    void setText(KDevelop::IndexedString text);
    
  private:
    KDevelop::IndexedString m_text;
};

class RefactoringProgressDialog : public QDialog {
  Q_OBJECT
  public:
    RefactoringProgressDialog(QString action);
    
  public slots:

    void progress(uint done, uint max);
    
    void maximumProgress(uint max);
    
    void setProcessing(KDevelop::IndexedString processing);
  private:
    KDevelop::IndexedString m_processing;
    QPushButton* m_cancelButton;
    StringProgressBar* m_progressBar;
};

class CollectorProgressDialog : public RefactoringProgressDialog {
  Q_OBJECT
  public:
    CollectorProgressDialog(QString action, KDevelop::UsesCollector& collector);
    
  private slots:
    void processUses(KDevelop::ReferencedTopDUContext context);
    
  private:
    KDevelop::UsesCollector& m_collector;
};

#endif
