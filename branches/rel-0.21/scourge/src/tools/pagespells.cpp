#include "pagespells.h"
#include "subpageschools.h"
#include "subpagespells.h"
#include "dfspells.h"
#include <wx/wx.h>
#include "common.h"

PageSpells::PageSpells()
{
	pageSchools = new subPageSchools;
	pageSpells = new subPageSpells;

	currentSubPage = pageSchools;
}

PageSpells::~PageSpells()
{
	delete pageSchools;
	delete pageSpells;
}

void PageSpells::Init(wxNotebook *notebook, DF* dataFile)
{
	dfSpells = (DFSpells*)dataFile;
	this->dataFile = dataFile;
	page = new wxPanel(notebook, ID_SpellsPage);

// Notebook
	subNotebook = new wxNotebook(page, ID_SpellsSubNotebook);
	subNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&PageSpells::OnSubPageChange, NULL, (wxEvtHandler*)this);

	pageSchools->Init(subNotebook, dfSpells, this);
	pageSpells->Init(subNotebook, dfSpells, this);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(subNotebook, 1,wxEXPAND|wxALIGN_CENTER);
	page->SetSizerAndFit(sizer);

	notebook->AddPage(page, _("Spells"));
}

void PageSpells::UpdatePage()
{
}

void PageSpells::Prev(int n)
{
	wxString str = subNotebook->GetPageText( subNotebook->GetSelection() );
	if ( str == L"Schools" )
	{
		currentSubPage = pageSchools;
		pageSchools->Prev(n);			// Need to do this, for some reason caling currentSubPage->Next(n) does not work!
										// I have no idea what's wrong, need to look into it.
	}
	else
	{
		currentSubPage = pageSpells;
		pageSpells->Prev(n);
	}
//	currentSubPage->Prev(n);
}
void PageSpells::Next(int n)
{
	wxString str = subNotebook->GetPageText( subNotebook->GetSelection() );
	if ( str == L"Schools" )
	{
		currentSubPage = pageSchools;
		pageSchools->Next(n);			// Need to do this, for some reason caling currentSubPage->Next(n) does not work!
										// I have no idea what's wrong, need to look into it.
	}
	else
	{
		currentSubPage = pageSpells;
		pageSpells->Next(n);
	}

//	currentSubPage->Next(n);
}
void PageSpells::New()
{
	currentSubPage->New();
}
void PageSpells::Del()
{
	currentSubPage->Del();
}

void PageSpells::UpdatePageNumber()
{
	currentSubPage->UpdatePageNumber();
}

void PageSpells::GetCurrent()
{
//	School *school = dfSpells->GetCurrent();
}

void PageSpells::SetCurrent()
{
}

void PageSpells::ClearCurrent()
{
}

void PageSpells::OnSubPageChange(wxCommandEvent& WXUNUSED(event))
{
	currentSubPage->SetCurrent();	// Store current data item held

	wxString str = subNotebook->GetPageText( subNotebook->GetSelection() );

	if ( str == L"Schools" )
		currentSubPage = pageSchools;
	else
		currentSubPage = pageSpells;

	currentSubPage->UpdatePage();
	currentSubPage->UpdatePageNumber();
}
