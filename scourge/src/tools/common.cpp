#include "common.h"
#include <wx/wx.h>

wxString std2wx(std::string s){
 wxString wx;
 const char* my_string=s.c_str();
 wxMBConvUTF8 *wxconv= new wxMBConvUTF8();
 wx=wxString(wxconv->cMB2WC(my_string),wxConvUTF8);
 delete wxconv;
 // test if conversion works of not. In case it fails convert from Ascii
 if(wx.length()==0)
 wx=wxString(wxString::FromAscii(s.c_str()));
 return wx;
}

std::string wx2std(wxString s){
  std::string s2;
  if(s.wxString::IsAscii()) {
    s2=s.wxString::ToAscii();
  } else {
    const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(s);
    const char *tmp_str = (const char*) tmp_buf;
    s2=std::string(tmp_str, strlen(tmp_str));
  }
  return s2;
}


bool GetTextDialog(wxString message, wxString title, wxString &text)
{
	wxTextEntryDialog dialog(0, message, title);
	if ( dialog.ShowModal() == wxID_CANCEL )
		return false;

	text = dialog.GetValue();
	return ( text != L"" );
}

void SplitLine(std::string &line, std::vector<std::string> &lines, uint lineSize)
{
	// Make sure the vector is clear
	lines.clear();

	std::string::size_type strLength;
	std::string str;
	while ( true )
	{
		if ( line.size() <= lineSize )
		{
			lines.push_back(line);
			break;
		}
		str = line.substr(0,lineSize);
		strLength = str.find_last_of(' ');
		str = str.substr(0, strLength);
		lines.push_back(str);

		line = line.substr(strLength+1);		// Skip the space
	}
}
