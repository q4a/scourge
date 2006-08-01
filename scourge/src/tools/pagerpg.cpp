#include "pagerpg.h"
#include "subpageskills.h"
#include "subpageitemtags.h"
#include "subpagenames.h"
#include "dfrpg.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"
#include "listadddel.h"

PageRpg::PageRpg()
{
	//ctor
	pageSkills = new subPageSkills;
	pageItemTags = new subPageItemTags;
	pageNames = new subPageNames;

	currentSubPage = pageSkills;
}

PageRpg::~PageRpg()
{
	//dtor
	delete pageSkills;
	delete pageItemTags;
	delete pageNames;
}

void PageRpg::Init(wxNotebook *notebook, DF *dataFile)
{
	dfRpg = (DFRpg*)dataFile;
	this->dataFile = dataFile;
	page = new wxPanel(notebook, ID_RpgPage);

//	Group *group = dfRpg->GetCurrent();

// Notebook
	subNotebook = new wxNotebook(page, ID_RpgSubNotebook);
	subNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&PageRpg::OnSubPageChange, NULL, (wxEvtHandler*)this);

	pageSkills->Init(subNotebook, dfRpg, this);
	pageItemTags->Init(subNotebook, dfRpg, this);
	pageNames->Init(subNotebook, dfRpg, this);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(subNotebook, 1,wxEXPAND|wxALIGN_CENTER);
	page->SetSizerAndFit(sizer);

	notebook->AddPage(page, _("RPG"));
}

void PageRpg::UpdatePage()
{
}

void PageRpg::Prev(int n)
{
//	currentSubPage->Prev(n);
	pageSkills->Prev(n);
}
void PageRpg::Next(int n)
{
//	currentSubPage->Next(n);
	pageSkills->Next(n);
}
void PageRpg::New()
{
//	currentSubPage->New();
	pageSkills->New();
}
void PageRpg::Del()
{
//	currentSubPage->Del();
	pageSkills->Del();
}

void PageRpg::GetCurrent()
{
}
void PageRpg::SetCurrent()
{
}

void PageRpg::ClearCurrent()
{
}

void PageRpg::OnSubPageChange()
{
	currentSubPage->SetCurrent();	// Store current data item held

	wxString str = subNotebook->GetPageText( subNotebook->GetSelection() );

	if ( str == L"Skills" )
		currentSubPage = pageSkills;
	else if ( str == L"Item Tags" )
		currentSubPage = pageItemTags;
	else
		;

	currentSubPage->UpdatePage();
	currentSubPage->UpdatePageNumber();
}

void PageRpg::OnPageHelp()
{
	currentSubPage->OnPageHelp();
}
