#include "pagemissions.h"
#include "dfmissions.h"
#include <wx/wx.h>
#include "common.h"

PageMissions::PageMissions()
{
	itemStrArray = new wxArrayString;
	creatureStrArray = new wxArrayString;
}

PageMissions::~PageMissions()
{
	delete itemStrArray;
	delete creatureStrArray;
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
	typeCombo = new wxComboBox(page, ID_MissionTypeCombo, std2wx(std::string(buffer)), wxPoint(220,30),wxSize(50,25),
			2,choices, wxCB_READONLY);

	// storyline
	wxStaticText *storylineText = new wxStaticText(page, -1, _("Storyline"), wxPoint(280,10));
	wxString choicesYesNo[2] = { L"Yes", L"No" };
	wxString startChoice=L"No"; if ( mission->storyline ) startChoice=L"Yes";
	storylineCombo = new wxComboBox(page, ID_MissionStorylineCombo, startChoice, wxPoint(280,30),wxSize(60,25),
			2,choicesYesNo, wxCB_READONLY);

	// description
	wxStaticText *descText = new wxStaticText(page, -1, _("Description"), wxPoint(450,10));
	descEdit = new wxTextCtrl(page, ID_MissionDescEdit, std2wx(mission->description), wxPoint(450,30), wxSize(350,150), wxTE_MULTILINE);

	// items
	wxStaticText *itemsText = new wxStaticText(page, -1, L"Items", wxPoint(400,185));
	for ( int i = 0; i < mission->items.size(); i++ )
		itemStrArray->Add( std2wx(mission->items[i]) );
	itemList = new wxListBox(page, -1, wxPoint(400,205), wxSize(230,100), *itemStrArray);

		// Add item
		wxButton *addItem = new wxButton(page, ID_MissionAddItem,L"Add",wxPoint(400,310),wxSize(50,30));
		// Delete item
		wxButton *delItem = new wxButton(page, ID_MissionDelItem,L"Delete",wxPoint(455,310),wxSize(55,30));

	// creatures
	wxStaticText *creaturesText = new wxStaticText(page, -1, L"Creatures", wxPoint(650,185));
	for ( int i = 0; i < mission->creatures.size(); i++ )
		creatureStrArray->Add( std2wx(mission->creatures[i]) );
	creatureList = new wxListBox(page, -1, wxPoint(650,205), wxSize(-1,100), *creatureStrArray);

		// Add creature
		wxButton *addCreature = new wxButton(page, ID_MissionAddCreature,L"Add",wxPoint(650,310),wxSize(50,30));
		// Delete creature
		wxButton *delCreature = new wxButton(page, ID_MissionDelCreature,L"Delete",wxPoint(705,310),wxSize(55,30));

	// completed
	wxStaticText *succText = new wxStaticText(page, -1, _("Success"), wxPoint(10,60));
	succEdit = new wxTextCtrl(page, ID_MissionSuccEdit, std2wx(mission->success), wxPoint(10,80), wxSize(300,60), wxTE_MULTILINE);

	// not completed
	wxStaticText *failText = new wxStaticText(page, -1, _("Faliure"), wxPoint(10,150));
	failEdit = new wxTextCtrl(page, ID_MissionFailEdit, std2wx(mission->failure), wxPoint(10,170), wxSize(300,60), wxTE_MULTILINE);

	notebook->AddPage(page, _("Missions"));

	if ( !mission->storyline )		// Cheap and nasty fix, as UpdatePage doesn't work for unknown reasons
	{
		itemList->Disable();
		creatureList->Disable();
	}
}

void PageMissions::UpdatePage()
{
	wxColour c(255,255,255);
	bool enabled = true;
	if ( storylineCombo->GetValue() == L"No" )
	{
		c.Set(240,240,240);
		enabled = false;
	}
	itemList->SetOwnBackgroundColour(c);
	itemList->Enable(enabled);
	creatureList->SetOwnBackgroundColour(c);
	creatureList->Enable(enabled);
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
	typeCombo->SetValue(std2wx( std::string(buffer) ));

	if ( mission->storyline )
		storylineCombo->SetValue(L"Yes");
	else
		storylineCombo->SetValue(L"No");

	wxString startChoice=L"No"; if ( mission->storyline ) startChoice=L"Yes";
	storylineCombo->SetValue(startChoice);

	descEdit->SetValue(std2wx(mission->description));

	// items
	itemStrArray->Clear();
	for ( int i = 0; i < mission->items.size(); i++ )
		itemStrArray->Add( std2wx(mission->items[i]) );
	itemList->Set(*itemStrArray);

	// creatures
	creatureStrArray->Clear();
	for ( int i = 0; i < mission->creatures.size(); i++ )
		creatureStrArray->Add( std2wx(mission->creatures[i]) );
	creatureList->Set(*creatureStrArray);

	succEdit->SetValue(std2wx(mission->success));
	failEdit->SetValue(std2wx(mission->failure));
}

void PageMissions::SetCurrent()
{
	Mission *mission = dfMissions->GetCurrent();
	wxWindow *w, *parent = page->GetParent();

	mission->name = wx2std( nameEdit->GetValue() );

	char buffer[2]; buffer[0] = mission->type; buffer[1] = 0;
	mission->type = *( wx2std( typeCombo->GetValue() ).c_str() );

	mission->storyline = ( storylineCombo->GetValue() == L"Yes" );

	mission->description = wx2std( descEdit->GetValue() );

	// items
	mission->items.clear();
	for ( int i = 0; i < itemStrArray->GetCount(); i++ )
		mission->items.push_back( wx2std( (*itemStrArray)[i] ) );

	// creatures
	mission->creatures.clear();
	for ( int i = 0; i < creatureStrArray->GetCount(); i++ )
		mission->creatures.push_back( wx2std( (*creatureStrArray)[i] ) );

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

void PageMissions::OnStorylineChange()
{
	/*wxColor c(255,255,255);
	bool enabled = true;
	PageMissions *pPage = ((PageMissions*)currentPage);
	if ( pPage->storylineCombo->GetValue() == L"No" )
	{
		c.Set(240,240,240);
		enabled = false;
	}
	pPage->itemList->Enable(enabled);
	pPage->itemList->SetOwnBackgroundColour(c);
	pPage->creatureList->Enable(enabled);
	pPage->creatureList->SetOwnBackgroundColour(c);*/
	((PageMissions*)currentPage)->UpdatePage();
}

void PageMissions::OnAddItem()
{
	PageMissions *pPage = ((PageMissions*)currentPage);
	if ( !pPage->itemList->IsEnabled() )
		return;

	wxString str;
	if ( GetTextDialog(L"Insert text to add", L"Add item", str ) )
	{
		wxArrayString *pItemStrArray = pPage->itemStrArray;
		pItemStrArray->Add(str);
		pPage->itemList->Set(*pItemStrArray);
	}
}

void PageMissions::OnDelItem()
{
	PageMissions *pPage = ((PageMissions*)currentPage);

	wxArrayInt selected;
	if ( pPage->itemList->GetSelections(selected) == 0 )
		return;
	wxArrayString *pItemStrArray = pPage->itemStrArray;
	pItemStrArray->RemoveAt(selected[0]);
	pPage->itemList->Set(*pItemStrArray);
}

void PageMissions::OnAddCreature()
{
	PageMissions *pPage = ((PageMissions*)currentPage);
	if ( !pPage->creatureList->IsEnabled() )
		return;

	wxString str;
	if ( GetTextDialog(L"Insert text to add", L"Add item", str) )
	{
		wxArrayString *pCreatureStrArray = pPage->creatureStrArray;
		pCreatureStrArray->Add(str);
		pPage->creatureList->Set(*pCreatureStrArray);
	}
}

void PageMissions::OnDelCreature()
{
	PageMissions *pPage = ((PageMissions*)currentPage);

	wxArrayInt selected;
	if ( pPage->creatureList->GetSelections(selected) == 0 )
		return;
	wxArrayString *pCreatureStrArray = pPage->creatureStrArray;
	pCreatureStrArray->RemoveAt(selected[0]);
	pPage->creatureList->Set(*pCreatureStrArray);
}
