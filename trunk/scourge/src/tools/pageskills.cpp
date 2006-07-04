#include "pageskills.h"
#include "dfskills.h"
#include <wx/wx.h>
#include "common.h"
#include "../common/constants.h"

PageSkills::PageSkills()
{
	//ctor
}

PageSkills::~PageSkills()
{
	//dtor
}

void PageSkills::Init(wxNotebook *notebook, DF *dataFile)
{
	dfSkills = (DFSkills*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_SkillsPage);

	SpecialSkill *skill = dfSkills->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(skill->name), wxPoint(10,30), wxSize(200,-1));

	// squirrel name
	wxStaticText *squirrelNameText = new wxStaticText(page, -1, _("Squirrel Name"), wxPoint(220,10));
	squirrelNameEdit = new wxTextCtrl(page, ID_BookRarenessEdit, std2wx(std::string(skill->squirrelName)), wxPoint(220,30), wxSize(100,-1));

	// type
	wxStaticText *typeText = new wxStaticText(page, -1, _("Type"), wxPoint(330,10));
	wxString typeChoices[2] = { L"M", L"A" };
	typeCombo = new wxComboBox(page, ID_SkillsTypeCombo, std2wx(skill->type), wxPoint(330,30),wxSize(50,-1),
			2,typeChoices, wxCB_READONLY);
	// event
	wxStaticText *eventText = new wxStaticText(page, -1, _("Event"), wxPoint(390,10));
	wxString eventChoices[1] = { L"A" };
	eventCombo = new wxComboBox(page, -1, std2wx(skill->event), wxPoint(390,30),wxSize(50,-1),
			1,eventChoices, wxCB_READONLY);

/**
	Icon
**/
	wxStaticText *iconText = new wxStaticText(page, -1, _("Icon (x,y)"), wxPoint(25,60));
	// icon_x
	int x = atoi( skill->icon_x.c_str() );
	iconXEdit = new wxTextCtrl(page, -1, std2wx(skill->icon_x), wxPoint(10,80), wxSize(30,25));
	iconXScroll = new wxScrollBar(page, ID_SkillsIconXScroll, wxPoint(40,80), wxSize(-1,25), wxSB_VERTICAL);
		iconXScroll->SetScrollbar(x,1,20,1);
	// icon_y
	int y = atoi( skill->icon_y.c_str() );
	iconYEdit = new wxTextCtrl(page, -1, std2wx(skill->icon_y), wxPoint(55,80), wxSize(30,25));
	iconYScroll = new wxScrollBar(page, ID_SkillsIconYScroll, wxPoint(85,80), wxSize(-1,25), wxSB_VERTICAL);
		iconYScroll->SetScrollbar(y,1,17,1);

	page->Connect( wxEVT_PAINT, (wxObjectEventFunction)&PageSkills::OnPaint, NULL, (wxEvtHandler*)this);

	// description
	wxStaticText *descriptionText = new wxStaticText(page, -1, _("Description"), wxPoint(450,10));
	descriptionEdit = new wxTextCtrl(page, -1, std2wx(skill->description), wxPoint(450,30), wxSize(350,150), wxTE_MULTILINE);

	notebook->AddPage(page, _("Skills"));
}

void PageSkills::UpdatePage()
{
	eventCombo->Enable( (typeCombo->GetValue() == L"A") );
}

void PageSkills::GetCurrent()
{
	SpecialSkill *skill = dfSkills->GetCurrent();

	nameEdit->SetValue(std2wx(skill->name));
	squirrelNameEdit->SetValue(std2wx(skill->squirrelName));

	typeCombo->SetValue(std2wx(skill->type));
	eventCombo->SetValue(std2wx(skill->event));

	int x = atoi( skill->icon_x.c_str() );
	iconXEdit->SetValue( std2wx(skill->icon_x) );
		iconXScroll->SetScrollbar(x,1,20,1);
	int y = atoi( skill->icon_y.c_str() );
	iconYEdit->SetValue( std2wx(skill->icon_y) );
		iconYScroll->SetScrollbar(y,1,17,1);
	UpdateIcon();

	descriptionEdit->SetValue(std2wx(skill->description));
}

void PageSkills::SetCurrent()
{
	SpecialSkill *skill = dfSkills->GetCurrent();

	skill->name = wx2std( nameEdit->GetValue() );
	skill->squirrelName = wx2std( squirrelNameEdit->GetValue() );

	skill->type = wx2std( typeCombo->GetValue() );
	skill->event = wx2std( eventCombo->GetValue() );

	skill->icon_x = wx2std( iconXEdit->GetValue() );
	skill->icon_y = wx2std( iconYEdit->GetValue() );
	if ( skill->icon_x == "0" )		skill->icon_x = "1";
	if ( skill->icon_y == "0" )		skill->icon_y = "1";

	skill->description = wx2std( descriptionEdit->GetValue() );
}

void PageSkills::ClearCurrent()
{
//	nameEdit->SetValue(L"");
}

void PageSkills::OnTypeChange()
{
	((PageSkills*)currentPage)->UpdatePage();
}
void PageSkills::OnIconXChange()
{
	PageSkills *pPage = ((PageSkills*)currentPage);
	int newPos = pPage->iconXScroll->GetThumbPosition();

	char buffer[16]; sprintf(buffer, "%i", newPos);
	pPage->iconXEdit->SetValue( std2wx( std::string(buffer) ) );

	pPage->UpdateIcon();
}
void PageSkills::OnIconYChange()
{
	PageSkills *pPage = ((PageSkills*)currentPage);
	int newPos = pPage->iconYScroll->GetThumbPosition();

	char buffer[16]; sprintf(buffer, "%i", newPos);
	pPage->iconYEdit->SetValue( std2wx( std::string(buffer) ) );

	pPage->UpdateIcon();
}
void PageSkills::UpdateIcon()
{
	char path[300];
	sprintf( path, "%s/textures/spells.bmp", rootDir );
	wxImage image(std2wx(std::string(path)));
	wxBitmap bitmap(image);

	int icon_x = iconXScroll->GetThumbPosition();
	int icon_y = iconYScroll->GetThumbPosition();
	if ( icon_x == 0 ) icon_x = 1;
	if ( icon_y == 0 ) icon_y = 1;
	wxBitmap icon = bitmap.GetSubBitmap( wxRect(32*(icon_x-1),32*(icon_y-1),32,32) );

	wxClientDC dc(page);
	dc.DrawBitmap(icon, 110,75, false);
}

void PageSkills::OnPaint()
{
	UpdateIcon();
}
