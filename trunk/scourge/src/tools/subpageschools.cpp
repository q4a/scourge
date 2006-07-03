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
	colorSelector->Init(page, 10,110 );

	// symbol
	wxStaticText *symbolText = new wxStaticText(page, -1, _("Symbol"), wxPoint(590,10));
	symbolEdit = new wxTextCtrl(page, -1, std2wx(school->symbol), wxPoint(590,30), wxSize(-1,25));

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
}

void subPageSchools::SetCurrent()
{
	School *school = dfSpells->GetCurrent();

	school->name = wx2std( nameEdit->GetValue());
	school->deity = wx2std( deityEdit->GetValue());
	school->skill = wx2std( skillEdit->GetValue());
	school->resistSkill = wx2std( resistSkillEdit->GetValue());

	// color
	Color color = colorSelector->GetColor();
	char buffer[16];
	sprintf(buffer, "%.3f", color.r);
	school->r = buffer;
	sprintf(buffer, "%.3f", color.g);
	school->g = buffer;
	sprintf(buffer, "%.3f", color.b);
	school->b = buffer;


	school->symbol = wx2std( symbolEdit->GetValue());
}

void subPageSchools::ClearCurrent()
{
}

void subPageSchools::OnColorSliderChange()
{
	subPageSchools *pPage = ((PageSpells*)currentPage)->pageSchools;

	float r = pPage->rColorSlider->GetValue() / 1000.0f;
	float g = pPage->gColorSlider->GetValue() / 1000.0f;
	float b = pPage->bColorSlider->GetValue() / 1000.0f;

	char buffer[16];
	sprintf(buffer, "%.3f", r);
	pPage->rColorText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	pPage->gColorText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	pPage->bColorText->SetLabel( std2wx( std::string(buffer) ) );
}
