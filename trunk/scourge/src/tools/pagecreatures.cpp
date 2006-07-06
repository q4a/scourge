#include "pagecreatures.h"
#include "dfcreatures.h"
#include <wx/wx.h>
#include <wx/listbook.h>
#include "listadddel.h"
#include "common.h"

PageCreatures::PageCreatures()
{
	//ctor
	invList = new ListAddDel;
	spellList = new ListAddDel;
}

PageCreatures::~PageCreatures()
{
	//dtor
	delete invList;
	delete spellList;
}

void PageCreatures::Init(wxNotebook *notebook, DF *dataFile)
{
	dfCreatures = (DFCreatures*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_CreaturesPage);

	Creature *creature = dfCreatures->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(creature->name), wxPoint(10,30), wxSize(200,25));

	// portrait
	wxStaticText *portraitText = new wxStaticText(page, -1, _("Portrait"), wxPoint(220,10));
	portraitEdit = new wxTextCtrl(page, -1, std2wx(creature->portrait), wxPoint(220,30), wxSize(200,25));

	// md2
	wxStaticText *md2Text = new wxStaticText(page, -1, _("MD2 Dir"), wxPoint(430,10));
	md2Edit = new wxTextCtrl(page, -1, std2wx(creature->md2), wxPoint(430,30), wxSize(200,25));

	// skin
	wxStaticText *skinText = new wxStaticText(page, -1, _("Skin"), wxPoint(640,10));
	skinEdit = new wxTextCtrl(page, -1, std2wx(creature->skin), wxPoint(640,30), wxSize(150,25));

	// level
	wxStaticText *levelText = new wxStaticText(page, -1, _("Level"), wxPoint(10,70));
	levelEdit = new wxTextCtrl(page, -1, std2wx(creature->level), wxPoint(10,90), wxSize(50,25));

	// hp
	wxStaticText *hpText = new wxStaticText(page, -1, _("HP"), wxPoint(70,70));
	hpEdit = new wxTextCtrl(page, -1, std2wx(creature->hp), wxPoint(70,90), wxSize(50,25));

	// mp
	wxStaticText *mpText = new wxStaticText(page, -1, _("MP"), wxPoint(130,70));
	mpEdit = new wxTextCtrl(page, -1, std2wx(creature->mp), wxPoint(130,90), wxSize(50,25));

	// armor
	wxStaticText *armorText = new wxStaticText(page, -1, _("Armor"), wxPoint(190,70));
	armorEdit = new wxTextCtrl(page, -1, std2wx(creature->armor), wxPoint(190,90), wxSize(50,25));

	// rareness
	wxStaticText *rarenessText = new wxStaticText(page, -1, _("Rareness"), wxPoint(250,70));
	rarenessEdit = new wxTextCtrl(page, -1, std2wx(creature->rareness), wxPoint(250,90), wxSize(50,25));

	// speed
	wxStaticText *speedText = new wxStaticText(page, -1, _("Speed"), wxPoint(310,70));
	speedEdit = new wxTextCtrl(page, -1, std2wx(creature->speed), wxPoint(310,90), wxSize(50,25));

	// scale
	wxStaticText *scaleText = new wxStaticText(page, -1, _("Scale"), wxPoint(370,70));
	scaleEdit = new wxTextCtrl(page, -1, std2wx(creature->scale), wxPoint(370,90), wxSize(50,25));

	// npc
	wxStaticText *npcText = new wxStaticText(page, -1, _("NPC"), wxPoint(430,70));
	npcEdit = new wxTextCtrl(page, -1, std2wx(creature->npc), wxPoint(430,90), wxSize(50,25));

	// npcStartX
	wxStaticText *npcStartText = new wxStaticText(page, -1, _("NPC Start (X,Y)"), wxPoint(495,70));
	npcStartXEdit = new wxTextCtrl(page, -1, std2wx(creature->npcStartX), wxPoint(490,90), wxSize(50,25));
	npcStartYEdit = new wxTextCtrl(page, -1, std2wx(creature->npcStartY), wxPoint(550,90), wxSize(50,25));

	// inventory
	invList->Init(page, L"Inventory", creature->inventory, 10,120, 150);
	// spells
	spellList->Init(page, L"Spells", creature->spells, 170,120, 170);

	// skills
	skillList = new wxListView(page, -1, wxPoint(350,140),wxSize(250,100), wxLC_REPORT|wxLC_EDIT_LABELS);
		skillList->InsertColumn(0,L"Skill");
		skillList->InsertColumn(1,L"");

	std::map<std::string,std::string>::iterator itr = creature->skills.begin();
	for ( int i = 0; itr != creature->skills.end(); itr++, i++ )
	{
		skillList->InsertItem(i,L"");
		skillList->SetItem(i,0, std2wx(itr->first));
		skillList->SetItem(i,1, std2wx(itr->second));
	}
	skillList->SetColumnWidth(0,-1);
	skillList->SetColumnWidth(1,-1);

	wxButton *addSkill = new wxButton(page, -1, L"Add", wxPoint(350,245), wxSize(50,-1));
	addSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PageCreatures::OnAddSkill, NULL, (wxEvtHandler*)this);
	wxButton *delSkill = new wxButton(page, -1, L"Del", wxPoint(405,245), wxSize(50,-1));
	delSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PageCreatures::OnDelSkill, NULL, (wxEvtHandler*)this);

	notebook->AddPage(page, _("Creatures"));
}

void PageCreatures::UpdatePage()
{
}

void PageCreatures::GetCurrent()
{
	Creature *creature = dfCreatures->GetCurrent();

	nameEdit->SetValue(std2wx(creature->name));
	portraitEdit->SetValue(std2wx(creature->portrait));
	md2Edit->SetValue(std2wx(creature->md2));
	skinEdit->SetValue(std2wx(creature->skin));
	levelEdit->SetValue(std2wx(creature->level));
	hpEdit->SetValue(std2wx(creature->hp));
	mpEdit->SetValue(std2wx(creature->mp));
	armorEdit->SetValue(std2wx(creature->armor));
	rarenessEdit->SetValue(std2wx(creature->rareness));
	speedEdit->SetValue(std2wx(creature->speed));
	scaleEdit->SetValue(std2wx(creature->scale));
	npcEdit->SetValue(std2wx(creature->npc));
	npcStartXEdit->SetValue(std2wx(creature->npcStartX));
	npcStartYEdit->SetValue(std2wx(creature->npcStartY));

	invList->Get( creature->inventory );
	spellList->Get( creature->spells );

	skillList->DeleteAllItems();
	std::map<std::string,std::string>::iterator itr = creature->skills.begin();
	for ( int i = 0; itr != creature->skills.end(); itr++, i++ )
	{
		skillList->InsertItem(i,L"");
		skillList->SetItem(i,0, std2wx(itr->first));
		skillList->SetItem(i,1, std2wx(itr->second));
	}
}
void PageCreatures::SetCurrent()
{
	Creature *creature = dfCreatures->GetCurrent();

	creature->name = wx2std( nameEdit->GetValue() );
	creature->portrait = wx2std( portraitEdit->GetValue() );
	creature->md2 = wx2std( md2Edit->GetValue() );
	creature->skin = wx2std( skinEdit->GetValue() );
	creature->level = wx2std( levelEdit->GetValue() );
	creature->hp = wx2std( hpEdit->GetValue() );
	creature->mp = wx2std( mpEdit->GetValue() );
	creature->armor = wx2std( armorEdit->GetValue() );
	creature->rareness = wx2std( rarenessEdit->GetValue() );
	creature->speed = wx2std( speedEdit->GetValue() );
	creature->scale = wx2std( scaleEdit->GetValue() );
	creature->npc = wx2std( npcEdit->GetValue() );
	creature->npcStartX = wx2std( npcStartXEdit->GetValue() );
	creature->npcStartY = wx2std( npcStartYEdit->GetValue() );

	invList->Set( creature->inventory );
	spellList->Set( creature->spells );

	creature->skills.clear();
	wxListItem l;
	for ( int i = 0; i < skillList->GetItemCount(); i++ )
	{
		l.SetId(i);

		l.SetColumn(0);
		skillList->GetItem(l);
		std::string str = wx2std( l.GetText() );

		l.SetColumn(1);
		skillList->GetItem(l);
		creature->skills[ str ] = wx2std( l.GetText() );
	}
}
void PageCreatures::ClearCurrent()
{
	Creature *creature = dfCreatures->GetCurrent();
}

void PageCreatures::OnAddSkill()
{
	skillList->InsertItem( skillList->GetItemCount(), L"blank" );
}
void PageCreatures::OnDelSkill()
{
	long selected = skillList->GetFirstSelected();

	while ( selected != -1 )
	{
		skillList->DeleteItem( selected );

		selected = skillList->GetFirstSelected();
	}
}
