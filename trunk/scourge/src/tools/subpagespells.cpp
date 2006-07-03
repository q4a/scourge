#include "subpagespells.h"
#include "pagespells.h"
#include "dfspells.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"

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

	schoolNameText = new wxStaticText(page, -1, std2wx(school->name), wxPoint(220,40));
		schoolNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );


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

	// CLIP
	wxStaticText *clipText = new wxStaticText(page, -1, _("Targets"), wxPoint(420,130));
	wxString clipStr[] = { L"Creature", L"Location", L"Item", L"Party" };
	clipCheckList = new wxCheckListBox(page, -1, wxPoint(420,150), wxSize(85,90), 4,clipStr);
	clipCheckList->Check(0, spell->target.find("C") != std::string::npos );
	clipCheckList->Check(1, spell->target.find("L") != std::string::npos );
	clipCheckList->Check(2, spell->target.find("I") != std::string::npos );
	clipCheckList->Check(3, spell->target.find("P") != std::string::npos );

	// speed
	wxStaticText *speedText = new wxStaticText(page, -1, _("Speed"), wxPoint(515,130));
	speedEdit = new wxTextCtrl(page, -1, std2wx(spell->speed), wxPoint(515,150), wxSize(50,25));

	// effect
	wxStaticText *effectText = new wxStaticText(page, -1, _("Effect"), wxPoint(575,130));
	effectEdit = new wxTextCtrl(page, -1, std2wx(spell->effect), wxPoint(575,150), wxSize(150,25));

/**
	Icon
**/
	wxStaticText *iconText = new wxStaticText(page, -1, _("Icon (x,y)"), wxPoint(25,180));
	// icon_x
	int x = atoi( spell->icon_x.c_str() );
	iconXEdit = new wxTextCtrl(page, -1, std2wx(spell->icon_x), wxPoint(10,200), wxSize(30,25));
	iconXScroll = new wxScrollBar(page, ID_subSpellsIconXScroll, wxPoint(40,200), wxSize(-1,25), wxSB_VERTICAL);
		iconXScroll->SetScrollbar(x,1,20,1);
	// icon_y
	int y = atoi( spell->icon_y.c_str() );
	iconYEdit = new wxTextCtrl(page, -1, std2wx(spell->icon_y), wxPoint(55,200), wxSize(30,25));
	iconYScroll = new wxScrollBar(page, ID_subSpellsIconYScroll, wxPoint(85,200), wxSize(-1,25), wxSB_VERTICAL);
		iconYScroll->SetScrollbar(y,1,17,1);


	// disposition
	wxStaticText *dispositionText = new wxStaticText(page, -1, _("Disposition"), wxPoint(160,180));
	wxString dispositionChoices[2] = { L"Friendly", L"Hostile" };
	wxString disposition = L"Friendly";
	if ( spell->disposition == "H" )	disposition = L"Hostile";
	dispositionCombo = new wxComboBox(page, -1, disposition, wxPoint(160,200),wxSize(80,25),
			2,dispositionChoices, wxCB_READONLY);

	// prerequisite
	wxStaticText *prereqText = new wxStaticText(page, -1, _("Prerequisite"), wxPoint(250,180));
	wxString prereqChoices[] = { L"-none-", L"HP", L"AC",
			L"blessed", L"empowered", L"enraged", L"ac_protected", L"magic_protected", L"invisible",	// good
			L"drunk", L"poisoned", L"cursed", L"possessed", L"blinded", L"charmed", L"dead",			// bad
			L"overloaded", L"leveled" };																// neutral
	wxString prereq = std2wx(spell->prerequisite);
	if ( prereq == L"" )	prereq = L"-none-";
	prereqCombo = new wxComboBox(page, -1, std2wx(spell->prerequisite), wxPoint(250,200),wxSize(130,25),
			17,prereqChoices, wxCB_READONLY);

	// sound
	wxStaticText *soundText = new wxStaticText(page, -1, _("Sound"), wxPoint(160,230));
	soundEdit = new wxTextCtrl(page, -1, std2wx(spell->sound), wxPoint(160,250), wxSize(200,25));

	// notes
	wxStaticText *notesText = new wxStaticText(page, -1, _("Notes"), wxPoint(515,180));
	notesEdit = new wxTextCtrl(page, -1, std2wx(spell->notes), wxPoint(515,200), wxSize(230,90), wxTE_MULTILINE);


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
	SetCurrent();

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
	SetCurrent();

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

	// CLIP
	clipCheckList->Check(0, spell->target.find("C") != std::string::npos );
	clipCheckList->Check(1, spell->target.find("L") != std::string::npos );
	clipCheckList->Check(2, spell->target.find("I") != std::string::npos );
	clipCheckList->Check(3, spell->target.find("P") != std::string::npos );


	speedEdit->SetValue(std2wx(spell->speed));
	effectEdit->SetValue(std2wx(spell->effect));

	// icon
	int x = atoi( spell->icon_x.c_str() );
	iconXEdit->SetValue( std2wx(spell->icon_x) );
		iconXScroll->SetScrollbar(x,1,20,1);
	int y = atoi( spell->icon_y.c_str() );
	iconYEdit->SetValue( std2wx(spell->icon_y) );
		iconYScroll->SetScrollbar(y,1,17,1);
	UpdateIcon();

	// disposition
	wxString disposition = L"Friendly";
	if ( spell->disposition == "H" )	disposition = L"Hostile";
	dispositionCombo->SetValue(disposition);

	wxString prereq = std2wx(spell->prerequisite);
	if ( prereq == L"" )	prereq = L"-none-";
	prereqCombo->SetValue(prereq);

	soundEdit->SetValue(std2wx(spell->sound));
	notesEdit->SetValue(std2wx(spell->notes));
}

void subPageSpells::SetCurrent()
{
	Spell *spell = currentSpell;

	spell->name = wx2std( nameEdit->GetValue() );
	spell->symbol = wx2std( symbolEdit->GetValue() );
	spell->level = wx2std( levelEdit->GetValue() );
	spell->mana = wx2std( manaEdit->GetValue() );
	spell->exp = wx2std( expEdit->GetValue() );
	spell->failureRate = wx2std( failureRateEdit->GetValue() );
	spell->action = wx2std( actionEdit->GetValue() );
	spell->distance = wx2std( distanceEdit->GetValue() );
	spell->area = wx2std( areaCombo->GetValue() );

	// CLIP
	spell->target = "";
	if ( spell->target.find("C") != std::string::npos ) spell->target += "C";
	if ( spell->target.find("L") != std::string::npos ) spell->target += "L";
	if ( spell->target.find("I") != std::string::npos ) spell->target += "I";
	if ( spell->target.find("P") != std::string::npos ) spell->target += "P";

	spell->speed = wx2std( speedEdit->GetValue() );
	spell->effect = wx2std( effectEdit->GetValue() );

	// icon
	spell->icon_x = wx2std( iconXEdit->GetValue() );
	spell->icon_y = wx2std( iconYEdit->GetValue() );
	if ( spell->icon_x == "0" )		spell->icon_x = "1";
	if ( spell->icon_y == "0" )		spell->icon_y = "1";

	// disposition
	spell->disposition = "F";
	if ( dispositionCombo->GetValue() == L"Hostile" )	spell->disposition = "H";

	spell->prerequisite = wx2std( prereqCombo->GetValue() );
	if ( spell->prerequisite == "-none-" )
		spell->prerequisite = "";

	spell->sound = wx2std( soundEdit->GetValue() );
	spell->notes = wx2std( notesEdit->GetValue() );
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
	currentSpell = *spellItr;
	spellNumber = 1;

	schoolNameText->SetLabel( *schoolName );

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

void subPageSpells::OnIconXChange()
{
	subPageSpells *pPage = ((PageSpells*)currentPage)->pageSpells;
	int newPos = pPage->iconXScroll->GetThumbPosition();

	char buffer[16]; sprintf(buffer, "%i", newPos);
	pPage->iconXEdit->SetValue( std2wx( std::string(buffer) ) );

	pPage->UpdateIcon();
}
void subPageSpells::OnIconYChange()
{
	subPageSpells *pPage = ((PageSpells*)currentPage)->pageSpells;
	int newPos = pPage->iconYScroll->GetThumbPosition();

	char buffer[16]; sprintf(buffer, "%i", newPos);
	pPage->iconYEdit->SetValue( std2wx( std::string(buffer) ) );

	pPage->UpdateIcon();
}
void subPageSpells::UpdateIcon()
{
	wxImage image(std2wx(std::string(GetDataPath("%s/textures/spells.bmp"))));
	wxBitmap bitmap(image);

	int icon_x = iconXScroll->GetThumbPosition();
	int icon_y = iconYScroll->GetThumbPosition();
	wxBitmap icon = bitmap.GetSubBitmap( wxRect(32*(icon_x-1),32*(icon_y-1),32,32) );

	wxClientDC dc(page);
	dc.DrawBitmap(icon, 35,230, false);
}
