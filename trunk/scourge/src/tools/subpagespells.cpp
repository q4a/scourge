#include "subpagespells.h"
#include "pagespells.h"
#include "dfspells.h"
#include <wx/wx.h>
#include "common.h"

subPageSpells::subPageSpells()
{
	schoolStrArray = new wxArrayString;
	spellNumber = 1;
}

subPageSpells::~subPageSpells()
{
	delete schoolStrArray;
}

void subPageSpells::Init(wxNotebook *notebook, DF* dataFile, PageSpells *parent)
{
	dfSpells = (DFSpells*)dataFile;		// Make a copy of the datafile
	this->dataFile = dataFile;
	spellItr = dfSpells->GetCurrent()->spells.begin();
	currentSpell = *spellItr;
	currentSchool = *(dfSpells->data.begin());

	page = new wxNotebookPage(notebook, ID_Spells_subPage);
	this->parent = parent;

	School *school = dfSpells->GetCurrent();
	Spell *spell = *(school->spells.begin());

/**
	Schools
**/
	wxStaticText *schoolsText = new wxStaticText(page, -1, L"Schools", wxPoint(10,10));
	for ( std::vector<School*>::iterator itr = dfSpells->data.begin(); itr != dfSpells->data.end(); itr++ )
		schoolStrArray->Add( std2wx( (*itr)->name ) );
	schoolList = new wxListBox(page, ID_subSpellsSchoolList, wxPoint(10,30), wxSize(200,90), *schoolStrArray);


	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(220,60));
	nameEdit = new wxTextCtrl(page, -1, std2wx(spell->name), wxPoint(220,80), wxSize(200,25));

	// symbol
	wxStaticText *symbolText = new wxStaticText(page, -1, _("Symbol"), wxPoint(430,60));
	symbolEdit = new wxTextCtrl(page, -1, std2wx(spell->symbol), wxPoint(430,80), wxSize(150,25));

	// level
	wxStaticText *levelText = new wxStaticText(page, -1, _("Level"), wxPoint(590,60));
	levelEdit = new wxTextCtrl(page, -1, std2wx(spell->level), wxPoint(590,80), wxSize(50,25));

	// mana
	wxStaticText *manaText = new wxStaticText(page, -1, _("Mana"), wxPoint(650,60));
	manaEdit = new wxTextCtrl(page, -1, std2wx(spell->mana), wxPoint(650,80), wxSize(50,25));

	// exp
	wxStaticText *expText = new wxStaticText(page, -1, _("Exp"), wxPoint(10,130));
	expEdit = new wxTextCtrl(page, -1, std2wx(spell->exp), wxPoint(10,150), wxSize(50,-1));

	// failure rate
	wxStaticText *failureRateText = new wxStaticText(page, -1, _("Failure Rate"), wxPoint(70,130));
	failureRateEdit = new wxTextCtrl(page, -1, std2wx(spell->failureRate), wxPoint(70,150), wxSize(50,25));

	// action
	wxStaticText *actionText = new wxStaticText(page, -1, _("Action"), wxPoint(160,130));
	actionEdit = new wxTextCtrl(page, -1, std2wx(spell->action), wxPoint(160,150), wxSize(100,25));

	// distance
	wxStaticText *distanceText = new wxStaticText(page, -1, _("Distance"), wxPoint(270,130));
	distanceEdit = new wxTextCtrl(page, -1, std2wx(spell->distance), wxPoint(270,150), wxSize(50,25));

	// area
	wxStaticText *areaText = new wxStaticText(page, -1, _("Area"), wxPoint(330,130));
	wxString choices[2] = { L"single", L"group" };
	areaCombo = new wxComboBox(page, -1, std2wx(spell->area), wxPoint(330,150),wxSize(80,25),
			2,choices, wxCB_READONLY);

	notebook->AddPage(page, _("Spells"));
}

void subPageSpells::UpdatePage()
{
/**
	Schools
**/
	wxStaticText *schoolsText = new wxStaticText(page, -1, L"Schools", wxPoint(10,10));
	schoolStrArray->Clear();
	for ( std::vector<School*>::iterator itr = dfSpells->data.begin(); itr != dfSpells->data.end(); itr++ )
		schoolStrArray->Add( std2wx( (*itr)->name ) );
	schoolList->Set(*schoolStrArray);
}

void subPageSpells::Prev()
{
	std::vector<Spell*> *pSpells = &currentSchool->spells;
	spellNumber--;

	if ( spellItr == pSpells->begin() )
	{
		spellItr = pSpells->end();
		spellNumber = currentSchool->spells.size();
	}
	spellItr--;
	currentSpell = *spellItr;

	GetCurrent();
}
void subPageSpells::Next()
{
	std::vector<Spell*> *pSpells = &currentSchool->spells;

	spellItr++;
	spellNumber++;
	if ( spellItr == pSpells->end() )
	{
		spellItr = pSpells->begin();
		spellNumber = 1;
	}
	currentSpell = *spellItr;

	GetCurrent();
}
void subPageSpells::New()
{
}
void subPageSpells::Del()
{
}

void subPageSpells::UpdatePageNumber()
{
	char buffer[64];
	sprintf(buffer, "Page %i/%i", spellNumber, currentSchool->spells.size());
	g_pageNumText->SetLabel(std2wx(buffer));
}

void subPageSpells::GetCurrent()
{
	Spell *spell = currentSpell;

	nameEdit->SetValue(std2wx(spell->name));
	symbolEdit->SetValue(std2wx(spell->symbol));
	levelEdit->SetValue(std2wx(spell->level));
	manaEdit->SetValue(std2wx(spell->mana));
	expEdit->SetValue(std2wx(spell->exp));
	failureRateEdit->SetValue(std2wx(spell->failureRate));
	actionEdit->SetValue(std2wx(spell->action));
	distanceEdit->SetValue(std2wx(spell->distance));
	areaCombo->SetValue(std2wx(spell->area));
}

void subPageSpells::SetCurrent()
{
	Spell *spell = currentSpell;

	spell->name = wx2std( nameEdit->GetValue() );
	spell->symbol = wx2std( symbolEdit->GetValue() );
	spell->level = wx2std( levelEdit->GetValue() );
	spell->mana = wx2std( manaEdit->GetValue() );
}

void subPageSpells::ClearCurrent()
{
}

School* subPageSpells::GetSelectedSchool()
{
	wxArrayInt selected;
	if ( schoolList->GetSelections(selected) == 0 )
		return 0;

	wxArrayString *pSchoolStrArray = schoolStrArray;
	wxString *schoolName = &(*pSchoolStrArray)[ (selected[0]) ];

	for ( int i = 0; i < dfSpells->data.size(); i++ )
		if ( dfSpells->data[i]->name == wx2std(*schoolName) )
			currentSchool = dfSpells->data[i];

	spellItr = currentSchool->spells.begin();
	if ( !(*spellItr) )
	{
		wxMessageBox(L"",L"");
	}
	currentSpell = *spellItr;
	spellNumber = 1;

	return currentSchool;
}

void subPageSpells::OnSchoolChange()
{
	subPageSpells *pPage = ((PageSpells*)currentPage)->pageSpells;

	pPage->SetCurrent();
	pPage->GetSelectedSchool();
	pPage->GetCurrent();

	pPage->UpdatePageNumber();
}
