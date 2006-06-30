#include "pagespells.h"
#include "dfspells.h"
#include <wx/wx.h>
#include "common.h"

PageSpells::PageSpells()
{
	//ctor
}

PageSpells::~PageSpells()
{
	//dtor
}

void PageSpells::Init(wxNotebook *notebook, DF* dataFile)
{
	dfSpells = (DFSpells*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_SpellsPage);

	School *school = dfSpells->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	wxTextCtrl *nameEdit = new wxTextCtrl(page, -1, std2wx(school->name), wxPoint(10,30), wxSize(200,25));

	notebook->AddPage(page, _("Spells"));
}

void PageSpells::UpdatePage()
{
}

void PageSpells::GetCurrent()
{
}

void PageSpells::SetCurrent()
{
}

void PageSpells::ClearCurrent()
{
}
