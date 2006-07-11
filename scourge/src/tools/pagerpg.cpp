#include "pagerpg.h"
#include "subpageskills.h"
#include "dfrpg.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"
#include "listadddel.h"

PageRpg::PageRpg()
{
	//ctor
//	currentSkillName = new wxString;
	pageSkills = new subPageSkills;

	currentSubPage = pageSkills;
}

PageRpg::~PageRpg()
{
	//dtor
//	delete currentSkillName;
	delete pageSkills;
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
	currentSubPage->Prev(n);
}
void PageRpg::Next(int n)
{
	currentSubPage->Next(n);
}
void PageRpg::New()
{
	currentSubPage->New();
}
void PageRpg::Del()
{
	currentSubPage->Del();
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

	currentSubPage->UpdatePage();
	currentSubPage->UpdatePageNumber();
}
