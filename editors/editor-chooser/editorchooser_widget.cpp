#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kdeversion.h>
#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>
#include <kconfig.h>


#include "editorchooser_widget.h"


EditorChooserWidget::EditorChooserWidget(QWidget *parent, const char *name)
        : EditChooser(parent, name)
{
    load();
}


void EditorChooserWidget::load()
{
    EditorPart->clear();

    // ask the trader which editors he has to offer
    KTrader::OfferList offers = KTrader::self()->query("text/plain", "'KTextEditor/Document' in ServiceTypes");

    // find the editor to use
    KConfig *config = kapp->config();
    config->setGroup("Editor");
    QString editor = config->readPathEntry("EmbeddedKTextEditor");

    // add the entries to the listview
    KTrader::OfferList::Iterator it;
    int index=-1, current=0;
    for (it = offers.begin(); it != offers.end(); ++it)
    {
        EditorPart->insertItem((*it)->name());
        if ( (*it)->name() == editor )
            index = current;
        ++current;
    }

    if (index >=0)
        EditorPart->setCurrentItem(index);

	QString dirtyAction = config->readEntry( "DirtyAction" );
	
	if ( dirtyAction == "reload" )
	{
		reload->setChecked( true );
	}
	else if ( dirtyAction == "alert" )
	{
		alert->setChecked( true );
	}
	else
	{
		nothing->setChecked( true );
	}
}


void EditorChooserWidget::save()
{
    KConfig *config = kapp->config();
    config->setGroup("Editor");

    KTrader::OfferList offers = KTrader::self()->query("text/plain", "'KTextEditor/Document' in ServiceTypes");

    KTrader::OfferList::Iterator it;
    for (it = offers.begin(); it != offers.end(); ++it)
        if ( EditorPart->currentText() == (*it)->name() )
        {
            config->writePathEntry("EmbeddedKTextEditor", (*it)->name());
        }

	if ( reload->isChecked() )
	{
		config->writeEntry( "DirtyAction", "reload" );
	}
	else if ( alert->isChecked() )
	{
		config->writeEntry( "DirtyAction", "alert" );
	}
	else 
	{
		config->writeEntry( "DirtyAction", "nothing" );
	}

    config->sync();
}


void EditorChooserWidget::accept()
{
    save();
}

void EditorChooserWidget::slotEditPartChanged( const QString & )
{
	KTrader::OfferList offers = KTrader::self()->query("text/plain", "'KTextEditor/Document' in ServiceTypes");

	KTrader::OfferList::Iterator it;
	for (it = offers.begin(); it != offers.end(); ++it)
	{
		if ( EditorPart->currentText() == (*it)->name() )
		{
			external_changes_group->setEnabled( (*it)->desktopEntryName() == "katepart" );
			return;
		}
	}
}


#include "editorchooser_widget.moc"




