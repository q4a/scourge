#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

/** Forward Declarations **/
class wxString;
class wxStaticText;
class Page;

extern wxStaticText *g_pageNumText;
extern Page *g_currentPage;

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

/*
	Splits a string into strings of up to length lineSize (cutting off at nearest space)
	and stores the resulting strings in lines
*/
void SplitLine(std::string &line, std::vector<std::string> &lines, int lineSize = 70);

#endif // COMMON_H
