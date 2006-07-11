#include "pagelocations.h"
#include "dflocations.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"

PageLocations::PageLocations()
{
	//ctor
	types[0] = "City";
	types[1] = "Dungeon";
	types[2] = "Forest";
	types[3] = "Mountains";
	types[4] = "Plains";
	types[5] = "Ocean";
	types[6] = "Hills";
}

PageLocations::~PageLocations()
{
	//dtor
}

void PageLocations::Init(wxNotebook *notebook, DF *dataFile)
{
	dfLocations = (DFLocations*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_LocationsPage);

	Location *location = dfLocations->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(location->name), wxPoint(10,30), wxSize(300,-1));

	// map coord
	wxStaticText *rarenessText = new wxStaticText(page, -1, _("Map Coords"), wxPoint(330,10));
	xSpin = new wxSpinCtrl(page, -1, L"", wxPoint(320,30),wxSize(50,-1), wxSP_ARROW_KEYS, 0,2000, atoi(location->x.c_str()));
	ySpin = new wxSpinCtrl(page, -1, L"", wxPoint(370,30),wxSize(50,-1), wxSP_ARROW_KEYS, 0,2000, atoi(location->y.c_str()));

	// type
	wxStaticText *missionText = new wxStaticText(page, -1, _("Mission Name"), wxPoint(10,80));
	wxString wxTypes[] = { L"City", L"Dungeon", L"Forest", L"Mountains", L"Plains", L"Ocean", L"Hills" };
	typeCombo = new wxComboBox(page, -1, GetType(location) , wxPoint(10,100),wxSize(100,25),
			7,wxTypes, wxCB_READONLY);

	// random
	randomCheck = new wxCheckBox(page, -1, L"Random", wxPoint(120,100));
	randomCheck->SetValue( location->random );

	notebook->AddPage(page, _("Locations"));
}

void PageLocations::UpdatePage()
{
}

void PageLocations::GetCurrent()
{
	Location *location = dfLocations->GetCurrent();

	nameEdit->SetValue(std2wx(location->name));
	xSpin->SetValue(std2wx(location->x));
	ySpin->SetValue(std2wx(location->y));

	typeCombo->SetValue( GetType(location) );
	randomCheck->SetValue( location->random );
}

void PageLocations::SetCurrent()
{
	Location *location = dfLocations->GetCurrent();
	char buffer[16];

	location->name = wx2std( nameEdit->GetValue() );
	sprintf(buffer, "%i", xSpin->GetValue());
	location->x = buffer;
	sprintf(buffer, "%i", ySpin->GetValue());
	location->y = buffer;
	location->type = typeCombo->GetValue()[0];

	if ( location->random = randomCheck->IsChecked() )
		location->type += "R";
}

void PageLocations::ClearCurrent()
{
	Location *location = dfLocations->GetCurrent();

	location->name = "";
	location->x = "0";
	location->y = "0";
	location->type = "CR";
}

wxString PageLocations::GetType(Location *location)
{
	for ( int i = 0; i < 7; i++ )
		if ( types[i][0] == location->type[0] )
			return std2wx(types[i]);
}
