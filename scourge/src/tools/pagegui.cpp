#include "pagegui.h"
#include "dfgui.h"
#include <wx/wx.h>
#include "common.h"

PageGui::PageGui()
{
	currentElement = 0;
	currentColor = 0;
}

PageGui::~PageGui()
{
	delete elementStrArray;
	delete colorStrArray;
}

void PageGui::Init(wxNotebook *notebook, DF *dataFile)
{
	dfGui = (DFGui*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, -1);

	Theme *theme = dfGui->GetCurrent();

	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, /*L""*/std2wx(theme->name), wxPoint(10,30), wxSize(-1,25));

	wxStaticBox *elementsBox = new wxStaticBox(page, -1, L"Elements", wxPoint(10,65),wxSize(400,280));
	wxStaticBox *colorsBox = new wxStaticBox(page, -1, L"Colors", wxPoint(420,10),wxSize(400,180));

/**
	Elements
**/
	wxString str[12] = { L"windowBack", L"windowTop", L"windowBorder", L"buttonBackground", L"buttonSelectionBackground",
			L"buttonHighlight", L"buttonBorder", L"listBackground", L"inputBackground", L"selectionBackground",
			L"selectedBorder", L"selectedCharacterBorder" };
	elementStrArray = new wxArrayString(12, str);
	elementList = new wxListBox(page, ID_GuiElementList, wxPoint(20,90), wxSize(230,90), *elementStrArray);

	elementNameText = new wxStaticText(page, -1, L"No Element Selected", wxPoint(260,100));
		elementNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	wxStaticText *lineWidthText = new wxStaticText(page, -1, _("Line width"), wxPoint(280,135));
	lineWidthEdit = new wxTextCtrl(page, -1, L"", wxPoint(295,155), wxSize(30,25));
	lineWidthScroll = new wxScrollBar(page, ID_GuiLineWidthScroll, wxPoint(325,155), wxSize(-1,25), wxSB_VERTICAL);
		lineWidthScroll->SetScrollbar(0,1,10,1);

	wxStaticText *textureText = new wxStaticText(page, -1, _("Textures"), wxPoint(60,195));
		textureText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	textureEdit = new wxTextCtrl(page, -1, L"texture", wxPoint(160,225), wxSize(100,25));
	northEdit = new wxTextCtrl(page, -1, L"north", wxPoint(140,195), wxSize(140,25));
	southEdit = new wxTextCtrl(page, -1, L"south", wxPoint(140,255), wxSize(140,25));
	eastEdit = new wxTextCtrl(page, -1, L"east", wxPoint(270,225), wxSize(100,25));
	westEdit = new wxTextCtrl(page, -1, L"west", wxPoint(50,225), wxSize(100,25));


	rElementText = new wxStaticText(page, -1, _("Red: "), wxPoint(50,295));    rElementText->SetForegroundColour(wxColor(255,0,0));
	gElementText = new wxStaticText(page, -1, _("Green: "), wxPoint(140,295)); gElementText->SetForegroundColour(wxColor(0,255,0));
	bElementText = new wxStaticText(page, -1, _("Blue: "), wxPoint(230,295));  bElementText->SetForegroundColour(wxColor(0,0,255));
	aElementText = new wxStaticText(page, -1, _("Alpha: "), wxPoint(320,295));

	rElementSlider = new wxSlider(page, ID_GuiElementSlider, 0,0,1000, wxPoint(30 ,315),wxSize(90,-1));
	gElementSlider = new wxSlider(page, ID_GuiElementSlider, 0,0,1000, wxPoint(120,315),wxSize(90,-1));
	bElementSlider = new wxSlider(page, ID_GuiElementSlider, 0,0,1000, wxPoint(210,315),wxSize(90,-1));
	aElementSlider = new wxSlider(page, ID_GuiElementSlider, 0,0,1000, wxPoint(300,315),wxSize(90,-1));

/**
	Colors
**/
	wxString strColors[6] = { L"windowTitleText", L"windowText", L"buttonText", L"buttonSelectionText",
			L"inputText", L"selectionText" };
	colorStrArray = new wxArrayString(6, strColors);
	colorList = new wxListBox(page, ID_GuiColorList, wxPoint(430,35), wxSize(230,90), *colorStrArray);

	colorNameText = new wxStaticText(page, -1, L"No Color Selected", wxPoint(670,45));
		colorNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	rColorText = new wxStaticText(page, -1, _("Red: "), wxPoint(450,140));   rColorText->SetForegroundColour(wxColor(255,0,0));
	gColorText = new wxStaticText(page, -1, _("Green: "), wxPoint(540,140)); gColorText->SetForegroundColour(wxColor(0,255,0));
	bColorText = new wxStaticText(page, -1, _("Blue: "), wxPoint(630,140));  bColorText->SetForegroundColour(wxColor(0,0,255));
	aColorText = new wxStaticText(page, -1, _("Alpha: "), wxPoint(720,140));

	rColorSlider = new wxSlider(page, ID_GuiColorSlider, 0,0,1000, wxPoint(430,160),wxSize(90,-1));
	gColorSlider = new wxSlider(page, ID_GuiColorSlider, 0,0,1000, wxPoint(520,160),wxSize(90,-1));
	bColorSlider = new wxSlider(page, ID_GuiColorSlider, 0,0,1000, wxPoint(610,160),wxSize(90,-1));
	aColorSlider = new wxSlider(page, ID_GuiColorSlider, 0,0,1000, wxPoint(700,160),wxSize(90,-1));

	notebook->AddPage(page, _("GUI"));

	UpdatePage();
}

void PageGui::UpdatePage()
{
}

void PageGui::GetCurrent()
{
	Theme *theme = dfGui->GetCurrent();

	// name
	nameEdit->SetValue( std2wx(theme->name) );

	GetElement();
	GetColor();
}

void PageGui::SetCurrent()
{
	SetElement();
	SetColor();
}

void PageGui::ClearCurrent()
{
	Theme *theme = dfGui->GetCurrent();

	theme->name = "";

	std::map <std::string,Element*>::iterator eItr;
	std::map <std::string,Color*>::iterator cItr;

	for ( eItr = theme->elements.begin(); eItr != theme->elements.end(); eItr++ )
		eItr->second->Clear();

	for ( cItr = theme->colors.begin(); cItr != theme->colors.end(); cItr++ )
		cItr->second->Clear();
}

void PageGui::GetElement()
{
	Element *element = GetSelectedElement();
	if ( element )
	{
		// line width
		char buffer[16];	sprintf(buffer, "%i", element->lineWidth);
		lineWidthEdit->SetValue( std2wx(buffer) );
		lineWidthScroll->SetScrollbar(element->lineWidth,1,10,1);

		// textures
		textureEdit->SetValue( std2wx(element->texture) );
		northEdit->SetValue( std2wx(element->north) );
		southEdit->SetValue( std2wx(element->south) );
		eastEdit->SetValue( std2wx(element->east) );
		westEdit->SetValue( std2wx(element->west) );

		// colors
		Color *color = &element->color;
		rElementSlider->SetValue((int)(color->r*1000));
		gElementSlider->SetValue((int)(color->g*1000));
		bElementSlider->SetValue((int)(color->b*1000));
		aElementSlider->SetValue((int)(color->a*1000));

		sprintf(buffer, "%.3f", color->r);
		rElementText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->g);
		gElementText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->b);
		bElementText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->a);
		aElementText->SetLabel( std2wx( std::string(buffer) ) );
	}
}
void PageGui::SetElement()
{
	Element *element = currentElement;
	if ( element )
	{
		// line width
		element->lineWidth = atoi( wx2std(lineWidthEdit->GetValue()).c_str() );

		// textures
		element->texture = wx2std( textureEdit->GetValue() );
		element->north = wx2std( northEdit->GetValue() );
		element->south = wx2std( southEdit->GetValue() );
		element->east = wx2std( eastEdit->GetValue() );
		element->west = wx2std( westEdit->GetValue() );

		// colors
		Color *color = &element->color;
		color->r = rElementSlider->GetValue() / 1000.0f;
		color->g = gElementSlider->GetValue() / 1000.0f;
		color->b = bElementSlider->GetValue() / 1000.0f;
		color->a = aElementSlider->GetValue() / 1000.0f;
	}
}

void PageGui::GetColor()
{
	Color *color = GetSelectedColor();
	if ( color )
	{
		rColorSlider->SetValue((int)(color->r*1000));
		gColorSlider->SetValue((int)(color->g*1000));
		bColorSlider->SetValue((int)(color->b*1000));
		aColorSlider->SetValue((int)(color->a*1000));

		char buffer[64];
		sprintf(buffer, "%.3f", color->r);
		rColorText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->g);
		gColorText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->b);
		bColorText->SetLabel( std2wx( std::string(buffer) ) );
		sprintf(buffer, "%.3f", color->a);
		aColorText->SetLabel( std2wx( std::string(buffer) ) );
	}
}
void PageGui::SetColor()
{
	Color *color = currentColor;
	if ( color )
	{
		color->r = rColorSlider->GetValue() / 1000.0f;
		color->g = gColorSlider->GetValue() / 1000.0f;
		color->b = bColorSlider->GetValue() / 1000.0f;
		color->a = aColorSlider->GetValue() / 1000.0f;
	}
}

Element* PageGui::GetSelectedElement()
{
	PageGui *pPage = ((PageGui*)currentPage);		// wxWidgets event callback safety
	Theme *theme = pPage->dfGui->GetCurrent();

	wxArrayInt selected;
	if ( pPage->elementList->GetSelections(selected) == 0 )
		return 0;

	wxArrayString *pElementStrArray = pPage->elementStrArray;
	currentElementName = &(*pElementStrArray)[ (selected[0]) ];
	wxString str = *currentElementName;

	// Hack - set the element name here (probably not needed for most calls)
	elementNameText->SetLabel( str );

	currentElement = theme->elements[ wx2std(str) ];
	return currentElement;
}
Color* PageGui::GetSelectedColor()
{
	Theme *theme = dfGui->GetCurrent();

	wxArrayInt selected;
	if ( colorList->GetSelections(selected) == 0 )
		return 0;

	wxArrayString *pColorStrArray = colorStrArray;
	currentColorName = &(*pColorStrArray)[ (selected[0]) ];
	wxString str = *currentColorName;

	// Hack - set the element name here (probably not needed for most calls)
	colorNameText->SetLabel( str );

	currentColor = theme->colors[ wx2std(str) ];
	return currentColor;
}

wxString PageGui::GetSelectedColorName()
{
	wxArrayInt selected;
	if ( colorList->GetSelections(selected) == 0 )
		return L"";

	wxArrayString *pColorStrArray = colorStrArray;
	return (*pColorStrArray)[ (selected[0]) ];
}

void PageGui::OnElementChange()
{
	PageGui *pPage = ((PageGui*)currentPage);
	pPage->SetElement();
	pPage->GetElement();
}
void PageGui::OnColorChange()
{
	PageGui *pPage = ((PageGui*)currentPage);
	pPage->SetColor();		// Set color last selected
	pPage->GetColor();		// Set color now selected
}

void PageGui::OnElementSliderChange()
{
	PageGui *pPage = ((PageGui*)currentPage);

	float r = pPage->rElementSlider->GetValue() / 1000.0f;
	float g = pPage->gElementSlider->GetValue() / 1000.0f;
	float b = pPage->bElementSlider->GetValue() / 1000.0f;
	float a = pPage->aElementSlider->GetValue() / 1000.0f;

	char buffer[64];
	sprintf(buffer, "%.3f", r);
	pPage->rElementText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	pPage->gElementText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	pPage->bElementText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", a);
	pPage->aElementText->SetLabel( std2wx( std::string(buffer) ) );
}
void PageGui::OnColorSliderChange()
{
	PageGui *pPage = ((PageGui*)currentPage);

	float r = pPage->rColorSlider->GetValue() / 1000.0f;
	float g = pPage->gColorSlider->GetValue() / 1000.0f;
	float b = pPage->bColorSlider->GetValue() / 1000.0f;
	float a = pPage->aColorSlider->GetValue() / 1000.0f;

	char buffer[64];
	sprintf(buffer, "%.3f", r);
	pPage->rColorText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	pPage->gColorText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	pPage->bColorText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", a);
	pPage->aColorText->SetLabel( std2wx( std::string(buffer) ) );
}

void PageGui::OnLineWidthChange(wxScrollEvent &event)
{
	PageGui *pPage = ((PageGui*)currentPage);

	int pos = pPage->lineWidthScroll->GetThumbPosition();
	char buffer[16]; sprintf(buffer, "%i", pos);
	pPage->lineWidthEdit->SetValue( std2wx( std::string(buffer) ) );
}
