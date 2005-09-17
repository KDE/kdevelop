%{CPP_TEMPLATE}

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>

class %{APPNAME}Application : public CL_ClanApplication
{
public:
	virtual int main(int argc, char** argv)
	{
		CL_SetupCore::init();
		CL_SetupDisplay::init();
		CL_SetupGL::init();
		CL_SetupSound::init();
	
		// Code here
	
		CL_SetupSound::deinit();
		CL_SetupGL::deinit();
		CL_SetupDisplay::deinit();
		CL_SetupCore::deinit();

		return 0;
	}
} app;

