#include "svn_revisionwidget.h"
#include <QRadioButton>
#include <knuminput.h>
#include <kdatetimewidget.h>
#include <kcombobox.h>

SvnRevisionWidget::SvnRevisionWidget( QWidget *parent )
    : QWidget(parent)
{
    setupUi( this );

    QList<SvnUtils::SvnRevision::RevKeyword> keylist;
    keylist << SvnRevision::HEAD << SvnRevision::BASE << SvnRevision::WORKING;
    keylist << SvnRevision::PREV << SvnRevision::COMMITTED;
    installKeys( keylist );

    datetimeWidget->setDateTime( QDateTime::currentDateTime() );
}

SvnRevisionWidget::~SvnRevisionWidget()
{}

SvnUtils::SvnRevision SvnRevisionWidget::revision()
{
    SvnUtils::SvnRevision rev;

    if( revnumRadio->isChecked() ){
        rev.setNumber( numInput->value() );
    }
    else if( revkindRadio->isChecked() ){
        QVariant data = kindCombo->itemData( kindCombo->currentIndex() );
        SvnUtils::SvnRevision::RevKeyword key = (SvnRevision::RevKeyword)(data.toInt());
        rev.setKey( key );
    }
    else if( revdateRadio->isChecked() ){
        rev.setDate( datetimeWidget->dateTime() );
    }

    return rev;
}

void SvnRevisionWidget::installKeys( const QList<SvnUtils::SvnRevision::RevKeyword> &keylist )
{
    kindCombo->clear();
    foreach( SvnUtils::SvnRevision::RevKeyword _key, keylist )
    {
        switch( _key ){
            case SvnUtils::SvnRevision::HEAD:
                kindCombo->addItem( "HEAD", QVariant( (int)SvnUtils::SvnRevision::HEAD ) );
            break;
            case SvnUtils::SvnRevision::BASE:
                kindCombo->addItem( "BASE", QVariant( (int)SvnUtils::SvnRevision::BASE ) );
            break;
            case SvnUtils::SvnRevision::WORKING:
                kindCombo->addItem( "WORKING", QVariant( (int)SvnUtils::SvnRevision::WORKING ) );
            break;
            case SvnUtils::SvnRevision::PREV:
                kindCombo->addItem( "PREV", QVariant( (int)SvnUtils::SvnRevision::PREV ) );
            break;
            case SvnUtils::SvnRevision::COMMITTED:
                kindCombo->addItem( "COMMITTED", QVariant( (int)SvnUtils::SvnRevision::COMMITTED ) );
            break;
        }
    }
}

void SvnRevisionWidget::setNumber( long revnum )
{
    numInput->setValue( revnum );
}

void SvnRevisionWidget::setKey( SvnUtils::SvnRevision::RevKeyword keyword )
{
    int idx = kindCombo->findData( QVariant((int)keyword) );
    if( idx == -1 ) return;
    kindCombo->setCurrentIndex( idx );
}

// only enables specified type. disable all other type
void SvnRevisionWidget::enableType( SvnUtils::SvnRevision::RevType type )
{
    switch( type ){
        case SvnUtils::SvnRevision::number:
            revnumRadio->setChecked(true);
        break;
        case SvnUtils::SvnRevision::kind:
            revkindRadio->setChecked(true);
        break;
        case SvnUtils::SvnRevision::date:
            revdateRadio->setChecked(true);
        break;
    }
}

#include "svn_revisionwidget.moc"
