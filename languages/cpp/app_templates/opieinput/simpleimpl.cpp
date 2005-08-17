#include <qwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qsignalmapper.h>
#include <qpushbutton.h>
#include <qpe/resource.h>
//Added by qt3to4:
#include <QPixmap>

#include "%{APPNAMELC}.h"

%{APPNAME}::%{APPNAME}(QWidget* par, Qt::WFlags fl )
	: Q3HBox(par, "name", fl )
{
	QCheckBox *box1 = new QCheckBox(tr("Alt"),this);
	connect(box1,SIGNAL(toggled(bool)),
			this,SLOT(slotAlt(bool)));
	m_alt = box1;
	box1 = new QCheckBox(tr("Shift"),this );
	connect(box1,SIGNAL(toggled(bool)),
			this,SLOT(slotShift(bool)));
	m_shi = box1;
	box1 = new QCheckBox(tr("Ctrl","Control Shortcut on keyboard"),this );
	connect(box1,SIGNAL(toggled(bool)),
			this,SLOT(slotCtrl(bool)));
	m_ctrl = box1;
			
	QSignalMapper *map = new QSignalMapper(this);
	QPushButton *btn = new QPushButton("a",this);
	map->setMapping(btn,0);
	connect(btn,SIGNAL(clicked()),map,SLOT(map()));
	
	btn = new QPushButton("b",this);
	map->setMapping(btn,1);
	connect(btn,SIGNAL(clicked()),map,SLOT(map()));
	
	btn = new QPushButton("c",this);
	map->setMapping(btn,2);
	connect(btn,SIGNAL(clicked()),map,SLOT(map()));
	
	connect(map,SIGNAL(mapped(int)),
			this,SLOT(slotKey(int)));
	resetState();
}

%{APPNAME}::~%{APPNAME}(){
}

void %{APPNAME}::resetState(){
	m_state = 0;
	m_shi->setChecked(false);
	m_ctrl->setChecked(false);
	m_alt->setChecked(false);
}

void %{APPNAME}::slotKey(int _ke){	
	int ke = _ke + 0x61; // 0 + 65 = 0x41 == A
	if(m_state & ShiftButton )	
		ke -= 0x20;
	
	/*
	 *  Send the key
	 * ke is the unicode
	 * _ke + 0x41 is the keycode
	 *  m_state Normally the state 
	 * down/up
	 * auto repeat
	 */
	emit key(ke, _ke +0x41,m_state,true,false);
	emit key(ke, _ke + 0x41,m_state,false,false);
}

void %{APPNAME}::slotShift(bool b){
	if(b)
		m_state |= ShiftButton;
	else
		m_state &= ~ShiftButton;	
}

void %{APPNAME}::slotAlt(bool b){
	if(b)
		m_state |= AltButton;
	else
		m_state &=  ~AltButton;
}

void %{APPNAME}::slotCtrl(bool b){
	if(b)
		m_state |= ControlButton;
	else
		m_state &= ~ControlButton;
}



%{APPNAME}Impl::%{APPNAME}Impl()
    : m_pickboard(0), m_icn(0)
{
}

%{APPNAME}Impl::~%{APPNAME}Impl()
{
    delete m_pickboard;
    delete m_icn;
}

QWidget *%{APPNAME}Impl::inputMethod( QWidget *parent, Qt::WFlags f )
{
    if ( !m_pickboard )
	m_pickboard = new %{APPNAME}( parent, f );
    return m_pickboard;
}

void %{APPNAME}Impl::resetState()
{
    if ( m_pickboard )
	m_pickboard->resetState();
}

QPixmap *%{APPNAME}Impl::icon()
{
    if ( !m_icn )
	m_icn = new QPixmap(Resource::loadPixmap("Tux"));
    return m_icn;
}

QString %{APPNAME}Impl::name()
{
    return QObject::tr("Example Input");
}

void %{APPNAME}Impl::onKeyPress( QObject *receiver, const char *slot )
{
    if ( m_pickboard )
	QObject::connect( m_pickboard, SIGNAL(key(ushort,ushort,ushort,bool,bool)), receiver, slot );
}

#ifndef QT_NO_COMPONENT
QRESULT %{APPNAME}Impl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_InputMethod )
	*iface = this;
    else
	return QS_FALSE;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( %{APPNAME}Impl )
}
#endif

