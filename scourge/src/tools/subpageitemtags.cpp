#include "subpageitemtags.h"
#include "pagerpg.h"
#include "dfrpg.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"

subPageItemTags::subPageItemTags()
{
	//ctor
	pageHelp = "In the description $$ will be substituted with 'weapons' or 'armor'.";
}

subPageItemTags::~subPageItemTags()
{
	//dtor
}

void subPageItemTags::Init(wxNotebook *notebook, DF *dataFile, PageRpg *parent)
{
	dfRpg = (DFRpg*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, /*ID_ItemTags_subPage*/-1);
	this->parent = parent;

	// list
	list = new wxListView(page, -1, wxPoint(10,10),wxSize(350,200), wxLC_REPORT|wxLC_EDIT_LABELS);
		list->InsertColumn(0,L"Tag name");
		list->InsertColumn(1,L"Description");

	std::vector<ItemTag*>::iterator itr = dfRpg->itemTags.begin();
	for ( int i = 0; itr != dfRpg->itemTags.end(); itr++, i++ )
	{
		list->InsertItem(i,L"");
		list->SetItem(i,0, std2wx((*itr)->name));
		list->SetItem(i,1, std2wx((*itr)->description));
	}
	list->SetColumnWidth(0,-1);
	list->SetColumnWidth(1,-1);

	wxButton *addSkill = new wxButton(page, -1, L"Add/Edit", wxPoint(10,215), wxSize(70,-1));
	addSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageItemTags::OnAddTag, NULL, (wxEvtHandler*)this);
	wxButton *delSkill = new wxButton(page, -1, L"Del", wxPoint(85,215), wxSize(50,-1));
	delSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageItemTags::OnDelTag, NULL, (wxEvtHandler*)this);

	notebook->AddPage(page, _("Item Tags"));
}

void subPageItemTags::UpdatePage()
{
}

void subPageItemTags::GetCurrent()
{
}
void subPageItemTags::SetCurrent()
{
}
void subPageItemTags::ClearCurrent()
{
}

class ItemTagEntryDialog : public wxTextEntryDialog
{
public:
	ItemTagEntryDialog(wxListView *list) : wxTextEntryDialog(0,L"Enter item tag",L"Item Tag Entry")
	{
		itemEdit = new wxTextCtrl(this, -1, L"", wxPoint(205,10),wxSize(75,-1) );

		wxString item, tag;
		if ( list->GetSelectedItemCount() == 1 )
		{
			item = list->GetItemText ( list->GetFirstSelected() );
			wxListItem l; l.SetColumn(1); l.SetId( list->GetFirstSelected() );// l.SetMask(wxLIST_MASK_TEXT);
			list->GetItem( l );
			tag = l.GetText();
			itemEdit->SetValue( item );
			wxTextEntryDialog::SetValue( tag );
		}

		if ( wxTextEntryDialog::ShowModal() == wxID_CANCEL )
			return;


		tag = wxTextEntryDialog::GetValue();
		item = itemEdit->GetValue();
		if ( item == L"" || tag == L"")
			return;

		long itemPos = list->FindItem(-1, item);
		if ( itemPos == -1)
		{
			itemPos = list->InsertItem( list->GetItemCount(), item );
			list->SetItem( itemPos, 1, tag);
		}
		else
			list->SetItem( itemPos, 1, tag);
	}
protected:
	wxTextCtrl *itemEdit;
};
void subPageItemTags::OnAddTag()
{
	ItemTagEntryDialog skillDialog( list );
}
void subPageItemTags::OnDelTag()
{
	long selected = list->GetFirstSelected();

	while ( selected != -1 )
	{
		list->DeleteItem( selected );

		selected = list->GetFirstSelected();
	}
}
