#include "pagemissions.h"
#include "dfmissions.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"
#include "listadddel.h"

PageMissions::PageMissions()
{
}

PageMissions::~PageMissions()
{
	delete itemList;
	delete creatureList;
}

void PageMissions::Init(wxNotebook *notebook, DF *dataFile)
{
	dfMissions = (DFMissions*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_MissionsPage);

	Mission *mission = dfMissions->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, ID_MissionNameEdit, std2wx(mission->name), wxPoint(10,30), wxSize(200,25));

	// type
	wxStaticText *typeText = new wxStaticText(page, -1, _("Type"), wxPoint(220,10));
	char buffer[2]; buffer[0] = mission->type; buffer[1] = 0;
	wxString choices[2] = { L"D", L"C" };
	typeCombo = new wxComboBox(page, ID_MissionTypeCombo, std2wx(buffer), wxPoint(220,30),wxSize(50,25),
			2,choices, wxCB_READONLY);

	// storyline
	wxStaticText *storylineText = new wxStaticText(page, -1, _("Storyline"), wxPoint(280,10));
	wxString choicesYesNo[2] = { L"Yes", L"No" };
	wxString startChoice=L"No"; if ( mission->storyline ) startChoice=L"Yes";
	storylineCombo = new wxComboBox(page, ID_MissionStorylineCombo, startChoice, wxPoint(280,30),wxSize(60,25),
			2,choicesYesNo, wxCB_READONLY);
	storylineCombo->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, (wxObjectEventFunction)&PageMissions::OnStorylineChange, NULL, (wxEvtHandler*)this);

	// level
	levelText = new wxStaticText(page, -1, _("Min-level"), wxPoint(10,60));
	int n = atoi( mission->level.c_str() );
	levelSpin = new wxSpinCtrl(page, -1, L"", wxPoint(15,80),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100,n);
	// stories
	storiesText = new wxStaticText(page, -1, _("Floors"), wxPoint(70,60));
	n = atoi( mission->stories.c_str() );
	storiesSpin = new wxSpinCtrl(page, -1, L"", wxPoint(70,80),wxSize(45,-1), wxSP_ARROW_KEYS, 1,100,n);
	// map
	mapText = new wxStaticText(page, -1, _("Map Name"), wxPoint(120,60));
	mapEdit = new wxTextCtrl(page, -1, std2wx(mission->mapname), wxPoint(120,80), wxSize(100,-1));

	// description
	wxStaticText *descText = new wxStaticText(page, -1, _("Description"), wxPoint(450,10));
	descEdit = new wxTextCtrl(page, ID_MissionDescEdit, std2wx(mission->description), wxPoint(450,30), wxSize(350,150), wxTE_MULTILINE);

	// items
	itemList = new ListAddDel;
	itemList->Init(page, L"Items", mission->items, 400,185);
	// creatures
	creatureList = new ListAddDel;
	creatureList->Init(page, L"Creatures", mission->creatures, 650,185, 180);

	// success
	wxStaticText *succText = new wxStaticText(page, -1, _("Success"), wxPoint(10,160));
	succEdit = new wxTextCtrl(page, ID_MissionSuccEdit, std2wx(mission->success), wxPoint(10,180), wxSize(300,60), wxTE_MULTILINE);

	// failure
	wxStaticText *failText = new wxStaticText(page, -1, _("Faliure"), wxPoint(10,250));
	failEdit = new wxTextCtrl(page, ID_MissionFailEdit, std2wx(mission->failure), wxPoint(10,270), wxSize(300,60), wxTE_MULTILINE);

	// special
	specialText = new wxStaticText(page, -1, _("Special"), wxPoint(230,60));
	specialEdit = new wxTextCtrl(page, -1, std2wx(mission->special), wxPoint(230,80), wxSize(200,-1));

	ShowStoryControls( mission->storyline );

	notebook->AddPage(page, _("Missions"));
}

void PageMissions::UpdatePage()
{
//	bool lock = ( storylineCombo->GetValue() == L"No" );
//	itemList->Lock(lock);
//	creatureList->Lock(lock);
}

/*void PageMissions::LoadAll()
{}*/

/*void PageMissions::SaveAll()
{}*/

void PageMissions::GetCurrent()
{
	Mission *mission = dfMissions->GetCurrent();

	nameEdit->SetValue(std2wx(mission->name));

	char buffer[2]; buffer[0] = mission->type; buffer[1] = 0;
	typeCombo->SetValue(std2wx( buffer ));

	if ( mission->storyline )
		storylineCombo->SetValue(L"Yes");
	else
		storylineCombo->SetValue(L"No");

	wxString startChoice=L"No"; if ( mission->storyline ) startChoice=L"Yes";
	storylineCombo->SetValue(startChoice);

	int n = atoi( mission->level.c_str() );
	levelSpin->SetValue(n);
	n = atoi( mission->stories.c_str() );
	storiesSpin->SetValue(n);
	mapEdit->SetValue(std2wx(mission->mapname));

	ShowStoryControls( mission->storyline );


	descEdit->SetValue(std2wx(mission->description));

	// items
	itemList->Get( mission->items );
	// creatures
	creatureList->Get( mission->creatures );

	itemList->Show( mission->storyline );
	creatureList->Show( mission->storyline );


	succEdit->SetValue(std2wx(mission->success));
	failEdit->SetValue(std2wx(mission->failure));

	specialEdit->SetValue(std2wx(mission->special));
}

void PageMissions::SetCurrent()
{
	Mission *mission = dfMissions->GetCurrent();

	mission->name = wx2std( nameEdit->GetValue() );

	char buffer[2]; buffer[0] = mission->type; buffer[1] = 0;
	mission->type = *( wx2std( typeCombo->GetValue() ).c_str() );

	mission->storyline = ( storylineCombo->GetValue() == L"Yes" );

	mission->description = wx2std( descEdit->GetValue() );

	// items
	itemList->Set( mission->items );
	// creatures
	creatureList->Set( mission->creatures );

	mission->success = wx2std( succEdit->GetValue() );

	mission->failure = wx2std( failEdit->GetValue() );
}

void PageMissions::ClearCurrent()
{
	Mission *mission = dfMissions->GetCurrent();
	mission->type = 'D';
	mission->name = mission->description = mission->success = mission->failure = "";
	mission->items.clear();
	mission->creatures.clear();
}

void PageMissions::ShowStoryControls(bool show)
{
	// Show/hide labels
	levelText->Show( show );
	storiesText->Show( show );
	mapText->Show( show );
	specialText->Show( show );
	// Show/hide edits
	levelSpin->Show( show );
	storiesSpin->Show( show );
	mapEdit->Show( show );
	specialEdit->Show( show );

	itemList->Show( show );
	creatureList->Show( show );
}

void PageMissions::OnStorylineChange()
{
	bool show = (storylineCombo->GetValue() == L"Yes");
	this->ShowStoryControls( show );
	UpdatePage();
}
