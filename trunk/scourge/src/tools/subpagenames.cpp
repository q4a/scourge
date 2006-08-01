#include "subpagenames.h"
#include "pagerpg.h"
#include "dfrpg.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"
#include "listadddel.h"

subPageNames::subPageNames()
{
	//ctor
}

subPageNames::~subPageNames()
{
	//dtor
}

void subPageNames::Init(wxNotebook *notebook, DF *dataFile, PageRpg *parent)
{
	dfRpg = (DFRpg*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, -1);
	this->parent = parent;

	// list
	list = new wxListView(page, -1, wxPoint(10,10),wxSize(350,200), wxLC_REPORT|wxLC_EDIT_LABELS);
		list->InsertColumn(0,L"Tag name");
		list->InsertColumn(1,L"Description");

	int i = 0;
	std::vector<SyllableLine>::iterator lineItr;
	std::vector<std::string>::iterator itr;
	for ( lineItr = dfRpg->syllables.first.begin(); lineItr != dfRpg->syllables.first.end(); lineItr++ )
	{
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++, i++ )
		{
			list->InsertItem(i,L"");
			list->SetItem(i,0, std2wx((*itr)));
		}
	}
	for ( lineItr = dfRpg->syllables.mid.begin(); lineItr != dfRpg->syllables.mid.end(); lineItr++ )
	{
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++, i++ )
		{
			list->InsertItem(i,L"");
			list->SetItem(i,0, std2wx((*itr)));
		}
	}
	for ( lineItr = dfRpg->syllables.end.begin(); lineItr != dfRpg->syllables.end.end(); lineItr++ )
	{
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++, i++ )
		{
			list->InsertItem(i,L"");
			list->SetItem(i,0, std2wx((*itr)));
		}
	}

	list->SetColumnWidth(0,-1);
	list->SetColumnWidth(1,-1);

	wxButton *addSkill = new wxButton(page, -1, L"Add/Edit", wxPoint(10,215), wxSize(70,-1));
//	addSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageNames::OnAddTag, NULL, (wxEvtHandler*)this);
	wxButton *delSkill = new wxButton(page, -1, L"Del", wxPoint(85,215), wxSize(50,-1));
//	delSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageNames::OnDelTag, NULL, (wxEvtHandler*)this);


ListAddDel *listAddDel = new ListAddDel;
listAddDel->Init(page, L"First", dfRpg->syllables.first[0], 400,100);

	notebook->AddPage(page, L"Names");
}

void subPageNames::UpdatePage()
{
}

void subPageNames::GetCurrent()
{
}
void subPageNames::SetCurrent()
{
}
void subPageNames::ClearCurrent()
{
}
