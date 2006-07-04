#include "subpageschools.h"
#include "pagespells.h"
#include "dfspells.h"
#include "colorselector.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"

subPageSchools::subPageSchools()
{
	colorSelector = new ColorSelector;
}

subPageSchools::~subPageSchools()
{
	delete colorSelector;
}

void subPageSchools::Init(wxNotebook *notebook, DF* dataFile, PageSpells *parent)
{
	dfSpells = (DFSpells*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_Schools_subPage);
	this->parent = parent;

	School *school = dfSpells->GetCurrent();


	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(school->name), wxPoint(10,30), wxSize(200,25));

	// deity
	wxStaticText *deityText = new wxStaticText(page, -1, _("Deity"), wxPoint(220,10));
	deityEdit = new wxTextCtrl(page, -1, std2wx(school->deity), wxPoint(220,30), wxSize(150,25));

	// skill
	wxStaticText *skillText = new wxStaticText(page, -1, _("Skill"), wxPoint(380,10));
	skillEdit = new wxTextCtrl(page, -1, std2wx(school->skill), wxPoint(380,30), wxSize(200,25));

	// resist skill
	wxStaticText *resistSkillText = new wxStaticText(page, -1, _("Resist Skill"), wxPoint(380,60));
	resistSkillEdit = new wxTextCtrl(page, -1, std2wx(school->resistSkill), wxPoint(380,80), wxSize(200,25));

/* Color */
	colorSelector->Init(page, 10,100 );

	// symbol
	wxStaticText *symbolText = new wxStaticText(page, -1, _("Symbol"), wxPoint(590,10));
	symbolEdit = new wxTextCtrl(page, -1, std2wx(school->symbol), wxPoint(590,30), wxSize(-1,25));

	// deity description
	wxStaticText *descText = new wxStaticText(page, -1, _("Deity Description"), wxPoint(400,120));
	descEdit = new wxTextCtrl(page, -1, std2wx(school->deityDescription), wxPoint(400,140), wxSize(350,150), wxTE_MULTILINE);

/**
	donation messages
**/
	wxStaticBox *donationsBox = new wxStaticBox(page, -1, L"Donation Messages", wxPoint(10,140),wxSize(360,170));

	// list
	wxArrayString donationStrArray;
	for ( int i = 0; i < school->lowDonation.size(); i++ )
		donationStrArray.Add( L"Low: " + std2wx(school->lowDonation[i]) );
	for ( int i = 0; i < school->neutralDonation.size(); i++ )
		donationStrArray.Add( L"Neutral:" + std2wx(school->neutralDonation[i]) );
	for ( int i = 0; i < school->highDonation.size(); i++ )
		donationStrArray.Add( L"High: " + std2wx(school->highDonation[i]) );
	donationList = new wxListBox(page, -1, wxPoint(20,170), wxSize(340,100), donationStrArray);
		// Add message
		wxButton *addMessage = new wxButton(page, -1,L"Add",wxPoint(20,275),wxSize(50,30));
		addMessage->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageSchools::OnAddMessage, NULL, (wxEvtHandler*)this);
		// Delete message
		wxButton *delMessage = new wxButton(page, -1,L"Delete",wxPoint(75,275),wxSize(55,30));
		delMessage->Connect( wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&subPageSchools::OnDelMessage, NULL, (wxEvtHandler*)this);


	notebook->AddPage(page, _("Schools"));
}

void subPageSchools::UpdatePage()
{
}

void subPageSchools::New()
{
	Page::New();

	School *school = dfSpells->GetCurrent();
	school->NewSpell();
}

void subPageSchools::UpdatePageNumber()
{
	char buffer[64];
	sprintf(buffer, "Page %i/%i", dfSpells->GetCurrentNum(), dfSpells->GetTotal());
	g_pageNumText->SetLabel(std2wx(buffer));
}

void subPageSchools::GetCurrent()
{
	School *school = dfSpells->GetCurrent();

	nameEdit->SetValue(std2wx(school->name));
	deityEdit->SetValue(std2wx(school->deity));
	skillEdit->SetValue(std2wx(school->skill));
	resistSkillEdit->SetValue(std2wx(school->resistSkill));

	// color
	float r = atof(school->r.c_str());
	float g = atof(school->g.c_str());
	float b = atof(school->b.c_str());
	colorSelector->SetColor( &Color(r,g,b) );

	symbolEdit->SetValue(std2wx(school->symbol));
	descEdit->SetValue(std2wx(school->deityDescription));

	// donation messages
	wxArrayString donationStrArray;
	for ( int i = 0; i < school->lowDonation.size(); i++ )
		donationStrArray.Add( L"Low: " + std2wx(school->lowDonation[i]) );
	for ( int i = 0; i < school->neutralDonation.size(); i++ )
		donationStrArray.Add( L"Neutral:" + std2wx(school->neutralDonation[i]) );
	for ( int i = 0; i < school->highDonation.size(); i++ )
		donationStrArray.Add( L"High: " + std2wx(school->highDonation[i]) );
	donationList->Set(donationStrArray);
}
void subPageSchools::SetCurrent()
{
	School *school = dfSpells->GetCurrent();

	school->name = wx2std( nameEdit->GetValue() );
	school->deity = wx2std( deityEdit->GetValue() );
	school->skill = wx2std( skillEdit->GetValue() );
	school->resistSkill = wx2std( resistSkillEdit->GetValue() );

	// color
	Color color = colorSelector->GetColor();
	char buffer[16];
	sprintf(buffer, "%.3f", color.r);
	school->r = buffer;
	sprintf(buffer, "%.3f", color.g);
	school->g = buffer;
	sprintf(buffer, "%.3f", color.b);
	school->b = buffer;


	school->symbol = wx2std( symbolEdit->GetValue() );
	school->deityDescription = wx2std( descEdit->GetValue() );

	// No need to set donation messages here, that is done in real-time
}
void subPageSchools::ClearCurrent()
{
	School *school = dfSpells->GetCurrent();

	school->name = "";
	school->deity = "";
	school->skill = "";
	school->resistSkill = "";

	// color
	school->r = "1";
	school->g = "1";
	school->b = "1";

	school->symbol = "";
	school->deityDescription = "";

	school->lowDonation.clear();
	school->neutralDonation.clear();
	school->highDonation.clear();
}

// TODO Whole donation message handling is horrible! Think of a better way to do it.

class MessageEntryDialog : public wxTextEntryDialog
{
public:
	MessageEntryDialog() : wxTextEntryDialog(0,L"message",L"title")
	{
		wxString choices[] = { L"Low", L"Neutral", L"High" };
		donationTypeCombo = new wxComboBox(this, -1, L"Low", wxPoint(235,10),wxSize(80,25),
			3,choices, wxCB_READONLY);

		wxTextEntryDialog::ShowModal();
	}
	wxString GetDonationMessage(School *school)
	{
		wxString message = wxTextEntryDialog::GetValue();
		if ( message == L"" )
			return L"";

		wxString donationType = donationTypeCombo->GetValue();
		if ( donationType == L"Low" )
		{
			school->lowDonation.push_back( wx2std(message) );
			message = L"Low: " + message;
		}
		else if ( donationType == L"Neutral" )
		{
			school->neutralDonation.push_back( wx2std(message) );
			message = L"Neutral: " + message;
		}
		else
		{
			school->highDonation.push_back( wx2std(message) );
			message = L"High: " + message;
		}

		return message;
	}

protected:
	wxComboBox *donationTypeCombo;
};
void subPageSchools::OnAddMessage()
{
	if ( !donationList->IsEnabled() )
		return;

	wxString str;
	MessageEntryDialog dialog;
	if ( (str = dialog.GetDonationMessage(dfSpells->GetCurrent())) != L"" )
	{
		donationList->Insert(str, donationList->GetCount());
	}
}
void subPageSchools::OnDelMessage()
{
	wxArrayInt selected;
	if ( donationList->GetSelections(selected) == 0 )
		return;

	wxString str = donationList->GetString( selected[0] );
	donationList->Delete( selected[0] );

	School *school = dfSpells->GetCurrent();
	std::vector<std::string>::iterator itr;
	for ( itr = school->lowDonation.begin(); itr != school->lowDonation.end(); itr++ )
	{
		if ( (*itr) == wx2std(str.Mid(5)) )
		{
			school->lowDonation.erase(itr);
			return;
		}
	}
	for ( itr = school->neutralDonation.begin(); itr != school->neutralDonation.end(); itr++ )
	{
		if ( (*itr) == wx2std(str.Mid(9)) )
		{
			school->neutralDonation.erase(itr);
			return;
		}
	}
	for ( itr = school->highDonation.begin(); itr != school->highDonation.end(); itr++ )
	{
		if ( (*itr) == wx2std(str.Mid(6)) )
		{
			school->highDonation.erase(itr);
			return;
		}
	}
}
