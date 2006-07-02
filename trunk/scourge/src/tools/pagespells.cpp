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
	page = new wxNotebookPage(notebook, ID_SpellsPage);

	School *school = dfSpells->GetCurrent();
	Spell *spell = *(school->spells.begin());

// Notebook
	subNotebook = new wxNotebook(page, ID_SpellsSubNotebook, wxDefaultPosition, wxSize(800,350));

	pageSchools->Init(subNotebook, dfSpells, this);
	pageSpells->Init(subNotebook, dfSpells, this);


	notebook->AddPage(page, _("Spells"));
}

void PageSpells::UpdatePage()
{
}

void PageSpells::Prev()
{
	currentSubPage->Prev();
}
void PageSpells::Next()
{
	currentSubPage->Next();
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
	School *school = dfSpells->GetCurrent();
}

void PageSpells::SetCurrent()
{
}

void PageSpells::ClearCurrent()
{
}

void PageSpells::OnSubPageChange(wxCommandEvent& WXUNUSED(event))
{
	PageSpells *pPage = ((PageSpells*)currentPage);
	wxString str = pPage->subNotebook->GetPageText( pPage->subNotebook->GetSelection() );

	if ( str == L"Schools" )
		pPage->currentSubPage = pPage->pageSchools;//->SetAsCurrent();
	else
		pPage->currentSubPage = pPage->pageSpells;

	pPage->currentSubPage->UpdatePage();
	pPage->currentSubPage->UpdatePageNumber();
}
