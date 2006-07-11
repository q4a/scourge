#include "pagerpg.h"
#include "dfrpg.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"
#include "listadddel.h"

PageRpg::PageRpg()
{
	//ctor
	currentSkillName = new wxString;
}

PageRpg::~PageRpg()
{
	//dtor
	delete currentSkillName;
}

void PageRpg::Init(wxNotebook *notebook, DF *dataFile)
{
	dfRpg = (DFRpg*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_RpgPage);

	Group *group = dfRpg->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(group->name), wxPoint(10,30), wxSize(200,25));

	// group description
	wxStaticText *descText = new wxStaticText(page, -1, _("Description"), wxPoint(10,60));
	descEdit = new wxTextCtrl(page, -1, std2wx(group->description), wxPoint(10,80), wxSize(260,80), wxTE_MULTILINE);

/** Skills **/
	wxStaticBox *skillsBox = new wxStaticBox(page, -1, L"Skills", wxPoint(290,10),wxSize(540,280));

	// skills
	skillList = new ListAddDel;
	skillList->Init(page, L"", group->skills, 300,20, 260);
	skillList->GetList()->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&PageRpg::OnSkillChange, NULL, (wxEvtHandler*)this);
	currentSkill = (Skill*)group->skills[0];
	// skill name
	skillNameText = new wxStaticText(page, -1, std2wx(currentSkill->name), wxPoint(300,20));
		skillNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	// symbol
	wxStaticText *symbolText = new wxStaticText(page, -1, _("Symbol"), wxPoint(300,185));
	symbolEdit = new wxTextCtrl(page, -1, std2wx( currentSkill->symbol ), wxPoint(300,205), wxSize(100,25));

	// skill description
	wxStaticText *skillDescText = new wxStaticText(page, -1, _("Description"), wxPoint(410,185));
	skillDescEdit = new wxTextCtrl(page, -1, std2wx(currentSkill->description), wxPoint(410,205), wxSize(350,60), wxTE_MULTILINE);

	// multiplier
	multiplierText = new wxStaticText(page, -1, _("Multiplier"), wxPoint(715,150));
	multiplierSpin = new wxSpinCtrl(page, -1, L"", wxPoint(770,145),wxSize(45,-1), wxSP_ARROW_KEYS, 0,5, atoi(currentSkill->multiplier.c_str()));
	// multiplied stats
	statList = new ListAddDel;
	statList->Init(page, L"Multiplied Stats", currentSkill->statNames, 580,20);

	bool hasStats = ( group->name != "BASIC_GROUP" );
	statList->Show( hasStats );
	multiplierSpin->Show( hasStats );
	multiplierText->Show( hasStats );

	notebook->AddPage(page, _("RPG"));
}

void PageRpg::UpdatePage()
{
}

void PageRpg::GetCurrent()
{
	Group *group = dfRpg->GetCurrent();

	nameEdit->SetValue( std2wx(group->name) );
	descEdit->SetValue( std2wx(group->description) );
	skillList->Get( group->skills );

	bool hasStats = ( group->name != "BASIC_GROUP" );
	statList->Show( hasStats );
	multiplierSpin->Show( hasStats );
	multiplierText->Show( hasStats );

	GetSkill();
}
void PageRpg::SetCurrent()
{
	Group *group = dfRpg->GetCurrent();

	group->name = wx2std( nameEdit->GetValue() );
	group->description = wx2std( descEdit->GetValue() );

	SetSkill();
}

void PageRpg::GetSkill()
{
	Group *group = dfRpg->GetCurrent();
	skillList->SetListSelection(0);		// So there is a selection after clicking next

	Skill *skill = GetSelectedSkill();
	if ( skill )
	{
		skillNameText->SetLabel( *currentSkillName );
		symbolEdit->SetValue( std2wx( skill->symbol ) );
		skillDescEdit->SetValue( std2wx( skill->description ) );
		multiplierSpin->SetValue( std2wx(skill->multiplier) );

		statList->Get( skill->statNames );
	}
}
void PageRpg::SetSkill()
{
	Skill *skill = currentSkill;
	if ( skill )
	{
		skill->symbol = wx2std( symbolEdit->GetValue() );
		skill->description = wx2std( skillDescEdit->GetValue() );

		char buffer[16];
		sprintf(buffer, "%i", multiplierSpin->GetValue() );
		skill->multiplier = buffer;
	}
}

void PageRpg::ClearCurrent()
{
}

Skill* PageRpg::GetSelectedSkill()
{
	Group *group = dfRpg->GetCurrent();

	wxListBox *pList = skillList->GetList();
	*currentSkillName = pList->GetString( pList->GetSelection() );
	Skill *skill = (Skill*)skillList->GetPointer( wx2std(*currentSkillName) );
	if ( skill )
		currentSkill = skill;

	return currentSkill;
}

void PageRpg::OnSkillChange()
{
	Group *group = dfRpg->GetCurrent();

	SetSkill();

	GetSelectedSkill();

	skillNameText->SetLabel( *currentSkillName );

	symbolEdit->SetValue( std2wx( currentSkill->symbol ) );
	skillDescEdit->SetValue( std2wx( currentSkill->description ) );
	multiplierSpin->SetValue( std2wx(currentSkill->multiplier) );

	statList->Get( currentSkill->statNames );
}
