#include "listadddel.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"

ListAddDel::ListAddDel()
{
	//ctor
}

ListAddDel::~ListAddDel()
{
	//dtor
}

void ListAddDel::Init(wxWindow* parent, wxString title, std::vector<std::string>& strVec, int x,int y, int width,int height)
{
	// list
	text = new wxStaticText(parent, -1, title, wxPoint(x,y));
	wxArrayString strArray;
	for ( int i = 0; i < strVec.size(); i++ )
		strArray.Add( std2wx(strVec[i]) );
	list = new wxListBox(parent, -1, wxPoint(x,y+20), wxSize(width,height), strArray);
		// Add item
		add = new wxButton(parent, -1,L"Add",wxPoint(x,y+height+25),wxSize(50,30));
		// Delete item
		del = new wxButton(parent, -1,L"Delete",wxPoint(x+55,y+height+25),wxSize(55,30));

	add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ListAddDel::OnAdd, NULL, (wxEvtHandler*)this);
	del->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ListAddDel::OnDel, NULL, (wxEvtHandler*)this);
}

void ListAddDel::OnAdd()
{
	if ( !list->IsEnabled() )
		return;

	wxString str;
	if ( GetTextDialog(L"Insert text to add", L"Add to list", str) )
	{
		list->Insert( str, list->GetCount() );
	}
}
void ListAddDel::OnDel()
{
	list->Delete( list->GetSelection() );
}

void ListAddDel::Lock(bool lock)
{
	wxColour c(255,255,255);
	if ( lock )
		c.Set(240,240,240);
	list->SetOwnBackgroundColour(c);
	list->Enable(!lock);
}
void ListAddDel::Show(bool show)
{
	list->Show( show );
	text->Show( show );
	add->Show( show );
	del->Show( show );
}

void ListAddDel::Get( std::vector<std::string>& strVec )
{
	list->Clear();
	for ( int i = 0; i < strVec.size(); i++ )
		list->Insert( std2wx(strVec[i]), i );
}
void ListAddDel::Set( std::vector<std::string>& strVec )
{
	strVec.clear();
	for ( int i = 0; i < list->GetCount(); i++ )
		strVec.push_back( wx2std( list->GetString(i) ) );
}
