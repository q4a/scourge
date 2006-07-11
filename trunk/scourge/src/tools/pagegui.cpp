#include "pagegui.h"
#include "dfgui.h"
#include <wx/wx.h>
#include <wx/colordlg.h>
#include "colorselector.h"
#include "../common/constants.h"

PageGui::PageGui()
{
	pageHelp = "Use the list boxes to select the theme element/color to edit.\nTo edit colors click on the color panel to open the color selection dialog, ";
	pageHelp += "and use the slider to change the alpha value.";

	currentElement = 0;
	currentColor = 0;
	elementColorSelector = new ColorSelector;
	colorSelector = new ColorSelector;
}

PageGui::~PageGui()
{
	delete elementStrArray;
	delete colorStrArray;
	delete elementColorSelector;
	delete colorSelector;
}

void PageGui::Init(wxNotebook *notebook, DF *dataFile)
{
	dfGui = (DFGui*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, -1);

	Theme *theme = dfGui->GetCurrent();

	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(theme->name), wxPoint(10,30), wxSize(-1,25));

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

	wxStaticText *lineWidthText = new wxStaticText(page, -1, _("Line width"), wxPoint(310,175));
	lineWidthEdit = new wxTextCtrl(page, -1, L"", wxPoint(325,195), wxSize(30,25));
	lineWidthScroll = new wxScrollBar(page, ID_GuiLineWidthScroll, wxPoint(355,195), wxSize(-1,25), wxSB_VERTICAL);
		lineWidthScroll->SetScrollbar(0,1,10,1);

	wxStaticText *textureText = new wxStaticText(page, -1, _("Textures"), wxPoint(60,195));
		textureText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	textureEdit = new wxTextCtrl(page, -1, L"texture", wxPoint(160,225), wxSize(100,25));
	northEdit = new wxTextCtrl(page, -1, L"north", wxPoint(140,195), wxSize(140,25));
	southEdit = new wxTextCtrl(page, -1, L"south", wxPoint(140,255), wxSize(140,25));
	eastEdit = new wxTextCtrl(page, -1, L"east", wxPoint(270,225), wxSize(100,25));
	westEdit = new wxTextCtrl(page, -1, L"west", wxPoint(50,225), wxSize(100,25));

	elementColorSelector->Init(page, 30,295, 260,140);

/**
	Colors
**/
	wxString strColors[6] = { L"windowTitleText", L"windowText", L"buttonText", L"buttonSelectionText",
			L"inputText", L"selectionText" };
	colorStrArray = new wxArrayString(6, strColors);
	colorList = new wxListBox(page, ID_GuiColorList, wxPoint(430,35), wxSize(230,90), *colorStrArray);

	colorNameText = new wxStaticText(page, -1, L"No Color Selected", wxPoint(670,45));
		colorNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	colorSelector->Init(page, 690,120, 670,90);

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
		elementColorSelector->SetColor(color);
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
		*color = elementColorSelector->GetColor();
	}
}

void PageGui::GetColor()
{
	Color *color = GetSelectedColor();
	if ( color )
	{
		colorSelector->SetColor(color);
	}
}
void PageGui::SetColor()
{
	Color *color = currentColor;
	if ( color )
	{
		*color = colorSelector->GetColor();
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

void PageGui::OnLineWidthChange()
{
	PageGui *pPage = ((PageGui*)currentPage);

	int pos = pPage->lineWidthScroll->GetThumbPosition();
	char buffer[16]; sprintf(buffer, "%i", pos);
	pPage->lineWidthEdit->SetValue( std2wx( buffer ) );
}
