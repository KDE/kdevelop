/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2008 Hamish Rodda <rodda@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "savedialog.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <interfaces/idocument.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>

using namespace KDevelop;

class DocumentItem : public QListWidgetItem
{
public:
    DocumentItem( IDocument* doc, QListWidget* parent )
        : QListWidgetItem(parent)
        , m_doc( doc )
    {
        setFlags(Qt::ItemIsUserCheckable | flags());
        setData(Qt::CheckStateRole, Qt::Checked);
        setText(m_doc->url().toDisplayString(QUrl::PreferLocalFile));
    }

    IDocument* doc() const { return m_doc; }

private:
    IDocument* m_doc;
};

KSaveSelectDialog::KSaveSelectDialog( const QList<IDocument*>& files, QWidget * parent )
    : QDialog( parent )
{
    setWindowTitle( i18n("Save Modified Files?") );

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel( i18n("The following files have been modified. Save them?"), this ));

    m_listWidget = new QListWidget(this);
    mainLayout->addWidget(m_listWidget);
//     m_listWidget->addColumn( "" );
//     m_listWidget->header()->hide();
//     m_listWidget->setSectionResizeMode( QListView::LastColumn );

    foreach (IDocument* doc, files)
        new DocumentItem( doc, m_listWidget );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Save);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    auto user1Button = buttonBox->addButton(i18n("Save &None" ), QDialogButtonBox::ActionRole);
    user1Button->setToolTip(i18n("Discard all modifications" ));
    connect(user1Button, SIGNAL(clicked()), this, SLOT(accept()));
    mainLayout->addWidget(buttonBox);
}

KSaveSelectDialog::~KSaveSelectDialog()
{
}

void KSaveSelectDialog::save( )
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        DocumentItem* item = static_cast<DocumentItem*>(m_listWidget->item(i));
        if (item->data(Qt::CheckStateRole).toBool())
            item->doc()->save(IDocument::Silent);
    }

    accept();
}

