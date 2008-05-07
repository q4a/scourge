#include "pagecreatures.h"
#include "dfcreatures.h"
#include <wx/wx.h>
#include <wx/listbook.h>
#include <wx/spinctrl.h>
#include "listadddel.h"
#include "common.h"

PageCreatures::PageCreatures()
{
	//ctor
	pageHelp = "If you want an npc to have a random start point (as in most cases) set NPC Start to (-1,-1)";
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
	new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(creature->name), wxPoint(10,30), wxSize(200,25));

	// portrait
	new wxStaticText(page, -1, _("Portrait"), wxPoint(220,10));
	portraitEdit = new wxTextCtrl(page, -1, std2wx(creature->portrait), wxPoint(220,30), wxSize(200,25));

	// md2
	new wxStaticText(page, -1, _("MD2 Dir"), wxPoint(430,10));
	md2Edit = new wxTextCtrl(page, -1, std2wx(creature->md2), wxPoint(430,30), wxSize(200,25));

	// skin
	new wxStaticText(page, -1, _("Skin"), wxPoint(640,10));
	skinEdit = new wxTextCtrl(page, -1, std2wx(creature->skin), wxPoint(640,30), wxSize(150,25));

	// level
	new wxStaticText(page, -1, _("Level"), wxPoint(10,70));
	levelSpin = new wxSpinCtrl(page, -1, L"", wxPoint(10,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100, atoi(creature->level.c_str()));

	// hp
	new wxStaticText(page, -1, _("HP"), wxPoint(70,70));
	hpSpin = new wxSpinCtrl(page, -1, L"", wxPoint(70,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100, atoi(creature->hp.c_str()));

	// mp
	new wxStaticText(page, -1, _("MP"), wxPoint(130,70));
	mpSpin = new wxSpinCtrl(page, -1, L"", wxPoint(130,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100, atoi(creature->mp.c_str()));

	// armor
	new wxStaticText(page, -1, _("Armor"), wxPoint(190,70));
	armorSpin = new wxSpinCtrl(page, -1, L"", wxPoint(190,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100, atoi(creature->armor.c_str()));

	// rareness
	new wxStaticText(page, -1, _("Rareness"), wxPoint(250,70));
	rarenessSpin = new wxSpinCtrl(page, -1, L"", wxPoint(250,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,10, atoi(creature->rareness.c_str()));

	// speed
	new wxStaticText(page, -1, _("Speed"), wxPoint(310,70));
	speedSpin = new wxSpinCtrl(page, -1, L"", wxPoint(310,90),wxSize(45,-1), wxSP_ARROW_KEYS, 1,10, atoi(creature->speed.c_str()));

	// scale
	new wxStaticText(page, -1, _("Scale"), wxPoint(370,70));
	scaleEdit = new wxTextCtrl(page, -1, std2wx(creature->scale), wxPoint(370,90), wxSize(50,25));

	// npc
	new wxStaticText(page, -1, _("NPC"), wxPoint(430,70));
	npcEdit = new wxTextCtrl(page, -1, std2wx(creature->npc), wxPoint(430,90), wxSize(50,25));

	// npcStartX
	new wxStaticText(page, -1, _("NPC Start (X,Y)"), wxPoint(495,70));
	npcStartXSpin = new wxSpinCtrl(page, -1, L"", wxPoint(500,90),wxSize(45,-1), wxSP_ARROW_KEYS, -1,500, atoi(creature->npcStartX.c_str()));
	npcStartYSpin = new wxSpinCtrl(page, -1, L"", wxPoint(550,90),wxSize(45,-1), wxSP_ARROW_KEYS, -1,500, atoi(creature->npcStartY.c_str()));

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

	wxButton *addSkill = new wxButton(page, -1, L"Add/Edit", wxPoint(350,245), wxSize(70,-1));
	addSkill->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PageCreatures::OnAddSkill, NULL, (wxEvtHandler*)this);
	wxButton *delSkill = new wxButton(page, -1, L"Del", wxPoint(425,245), wxSize(50,-1));
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
	levelSpin->SetValue( atoi(creature->level.c_str()) );
	hpSpin->SetValue( atoi(creature->hp.c_str()) );
	mpSpin->SetValue( atoi(creature->mp.c_str()) );
	armorSpin->SetValue( atoi(creature->armor.c_str()) );
	rarenessSpin->SetValue( atoi(creature->rareness.c_str()) );
	speedSpin->SetValue( atoi(creature->speed.c_str()) );
	scaleEdit->SetValue(std2wx(creature->scale));
	npcEdit->SetValue(std2wx(creature->npc));
	npcStartXSpin->SetValue( atoi(creature->npcStartX.c_str()) );
	npcStartYSpin->SetValue( atoi(creature->npcStartY.c_str()) );

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
	char buffer[16];

	creature->name = wx2std( nameEdit->GetValue() );
	creature->portrait = wx2std( portraitEdit->GetValue() );
	creature->md2 = wx2std( md2Edit->GetValue() );
	creature->skin = wx2std( skinEdit->GetValue() );

	sprintf(buffer, "%i", levelSpin->GetValue());
	creature->level = buffer;
	sprintf(buffer, "%i", hpSpin->GetValue());
	creature->hp = buffer;
	sprintf(buffer, "%i", mpSpin->GetValue());
	creature->mp = buffer;
	sprintf(buffer, "%i", armorSpin->GetValue());
	creature->armor = buffer;
	sprintf(buffer, "%i", rarenessSpin->GetValue());
	creature->rareness = buffer;
	sprintf(buffer, "%i", speedSpin->GetValue());
	creature->speed = buffer;

	creature->scale = wx2std( scaleEdit->GetValue() );
	creature->npc = wx2std( npcEdit->GetValue() );

	sprintf(buffer, "%i", npcStartXSpin->GetValue());
	creature->npcStartX = buffer;
	sprintf(buffer, "%i", npcStartYSpin->GetValue());
	creature->npcStartY = buffer;

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

	creature->name = "";
	creature->portrait = "";
	creature->md2 = "";
	creature->skin = "";
	creature->level = "";
	creature->hp = "";
	creature->mp = "";
	creature->armor = "";
	creature->rareness = "";
	creature->speed = "";
	creature->scale = "";
	creature->npc = "";
	creature->npcStartX = "";
	creature->npcStartY = "";

	creature->inventory.clear();
	creature->spells.clear();
	creature->skills.clear();
}

class CreatureSkillEntryDialog : public wxTextEntryDialog
{
public:
	CreatureSkillEntryDialog(wxListView *skillList) : wxTextEntryDialog(0,L"Enter skill",L"Skill Entry")
	{
		skillSpin = new wxSpinCtrl(this, -1, L"", wxPoint(235,10),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100,1);

		wxString skill;
		if ( skillList->GetSelectedItemCount() == 1 )
			skill = skillList->GetItemText ( skillList->GetFirstSelected() );
		this->SetValue(skill);

		if ( wxTextEntryDialog::ShowModal() == wxID_CANCEL )
			return;


		skill = wxTextEntryDialog::GetValue();
		if ( skill == L"" )
			return;

		char buffer[16];
		sprintf( buffer, "%i", skillSpin->GetValue() );

		long itemPos = skillList->FindItem(-1, skill);
		if ( itemPos == -1)
		{
			itemPos = skillList->InsertItem( skillList->GetItemCount(), skill );
			skillList->SetItem( itemPos, 1, std2wx(buffer));
		}
		else
			skillList->SetItem( itemPos, 1, std2wx(buffer));
	}
protected:
	wxSpinCtrl *skillSpin;
};
void PageCreatures::OnAddSkill()
{
	CreatureSkillEntryDialog skillDialog( skillList );
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
