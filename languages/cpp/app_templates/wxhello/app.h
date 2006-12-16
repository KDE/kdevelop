
#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

/**
 * @short Application Main Window
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */

class 
%{APPNAME}app : public wxApp
{
	public:
		virtual bool OnInit();
};

class 
%{APPNAME}Frame : public wxFrame
{
	public:
		%{APPNAME}Frame( const wxString& title, const wxPoint& pos, const wxSize& size );
		void OnQuit( wxCommandEvent& event );
		void OnAbout( wxCommandEvent& event );

	private:
		DECLARE_EVENT_TABLE()
};

enum
{
	Menu_File_Quit = 100,
	Menu_File_About
};

#endif // _%{APPNAMEUC}_H_
