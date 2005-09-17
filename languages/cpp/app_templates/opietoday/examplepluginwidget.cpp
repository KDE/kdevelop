#include <qpe/config.h>
#include <qpe/qcopenvelope_qws.h>

#include "%{APPNAMELC}widget.h"

%{APPNAME}Widget::%{APPNAME}Widget( QWidget *parent,  const char* name)
    : QWidget(parent,  name ) {

    m_exampleLabel = 0l;
    m_layout = 0l;

    if ( m_exampleLabel )  {
        delete m_exampleLabel;
    }

   // since here a OClickableLabel is used, the plugin part will be clickable, and the actions
   // that should be triggered when clicked are defined in  slotClicked()
   // of course also normal widgets can be used.
    m_exampleLabel = new OClickableLabel( this );
    connect( m_exampleLabel, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );

    if ( m_layout )  {
        delete m_layout;
    }
    m_layout = new QHBoxLayout( this );
    m_layout->setAutoAdd( true );

    readConfig();
    getInfo();
}


%{APPNAME}Widget::~%{APPNAME}Widget() {
    delete m_exampleLabel;
    delete m_layout;
}


void %{APPNAME}Widget::readConfig() {
// we dont have any config entries in this plugin
// normally this method is used after today config was used
}


void %{APPNAME}Widget::refresh()  {

}

void %{APPNAME}Widget::getInfo() {
    m_exampleLabel->setText(  tr("%{APPNAME} text")  );
}

void %{APPNAME}Widget::slotClicked() {
   getInfo();
}
