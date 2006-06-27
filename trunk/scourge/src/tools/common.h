#ifndef COMMON_H
#define COMMON_H

#include <string>

/** Forward Declarations **/
class wxString;

/*
	This code obtained from
	machard <at> gmail.com <machard <at> gmail.com>
	http://article.gmane.org/gmane.comp.lib.wxwidgets.general/44223
*/
wxString std2wx(std::string s);
std::string wx2std(wxString s);

/*
	Returns false if nothing is entered, or cancel is pressed
*/
bool GetTextDialog(wxString,wxString,wxString&);

#endif // COMMON_H
