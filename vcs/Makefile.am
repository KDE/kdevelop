##
## Do NOT remove the comments that start with "kdevelop:"
## They are actually directives to the kdevelop plugin system
##
## The include_xxxx variables are controlled by configure.in.in
##

#kdevelop: CERVISIA_SUBDIR = cervisia
#if include_cervisia
#CERVISIA_SUBDIR = cervisia
#endif

#kdevelop: CLEARCASE_SUBDIR = clearcase
if include_clearcase
CLEARCASE_SUBDIR = clearcase
endif

#kdevelop: CVSCLIENT_SUBDIR = cvsclient
#if include_cvsclient
#CVSCLIENT_SUBDIR = cvsclient
#endif

#kdevelop: CVSSERVICE_SUBDIR = cvsservice
if include_cvsservice
CVSSERVICE_SUBDIR = cvsservice
endif

#kdevelop: PERFORCE_SUBDIR = perforce
if include_perforce
PERFORCE_SUBDIR = perforce
endif

#kdevelop: SUBVERSION_SUBDIR = subversion
#if include_subversion
#SUBVERSION_SUBDIR = subversion
#endif

#SUBDIRS = $(CERVISIA_SUBDIR) $(CLEARCASE_SUBDIR) $(CVSSERVICE_SUBDIR) $(PERFORCE_SUBDIR) $(SUBVERSION_SUBDIR)
SUBDIRS = $(CLEARCASE_SUBDIR) $(CVSSERVICE_SUBDIR) $(PERFORCE_SUBDIR) 
