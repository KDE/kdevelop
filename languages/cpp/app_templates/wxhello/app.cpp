
#include <wx/wx.h>
#include "$APPNAMELC$.h"

BEGIN_EVENT_TABLE( $APPNAME$Frame, wxFrame )
	EVT_MENU( Menu_File_Quit, $APPNAME$Frame::OnQuit )
	EVT_MENU( Menu_File_About, $APPNAME$Frame::OnAbout )
END_EVENT_TABLE()

IMPLEMENT_APP($APPNAME$app)
	

bool 
$APPNAME$app::OnInit()
{
	$APPNAME$Frame *frame = new $APPNAME$Frame( "Hello World", wxPoint(50,50), wxSize(450,340) );

	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
} 

$APPNAME$Frame::$APPNAME$Frame( const wxString& title, const wxPoint& pos, const wxSize& size )
	: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	
	menuFile->Append( Menu_File_About, "&About..." );
	menuFile->AppendSeparator();
	menuFile->Append( Menu_File_Quit, "E&xit" );
	
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, "&File" );
	
	SetMenuBar( menuBar );
	
	CreateStatusBar();
	SetStatusText( "Welcome to Kdevelop wxWindows app!" );
}

void 
$APPNAME$Frame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	Close(TRUE);
}

void 
$APPNAME$Frame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	wxMessageBox( "This is a wxWindows Hello world sample",
			"About Hello World", wxOK | wxICON_INFORMATION, this );
}

