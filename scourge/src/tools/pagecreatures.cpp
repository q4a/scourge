#include "pagecreatures.h"
#include "dfcreatures.h"
#include <wx/wx.h>
#include "common.h"

PageCreatures::PageCreatures()
{
	//ctor
}

PageCreatures::~PageCreatures()
{
	//dtor
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
}
void PageCreatures::SetCurrent()
{
	Creature *creature = dfCreatures->GetCurrent();

	creature->name = wx2std( nameEdit->GetValue() );
}
void PageCreatures::ClearCurrent()
{
	Creature *creature = dfCreatures->GetCurrent();
}
