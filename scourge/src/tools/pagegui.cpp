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
	delete elementColorSelector;
	delete colorSelector;
}

void PageGui::Init(wxNotebook *notebook, DF *dataFile)
{
	dfGui = (DFGui*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, -1);

	Theme *theme = dfGui->GetCurrent();

	new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(theme->name), wxPoint(10,30), wxSize(-1,25));

	new wxStaticBox(page, -1, L"Elements", wxPoint(10,65),wxSize(400,280));
	new wxStaticBox(page, -1, L"Colors", wxPoint(420,10),wxSize(400,180));

/**
	Elements
**/
	wxString str[13] = { L"windowBack", L"windowTop", L"windowBorder", L"buttonBackground", L"buttonSelectionBackground",
			L"buttonHighlight", L"buttonBorder", L"listBackground", L"inputBackground", L"selectionBackground",
			L"selectedBorder", L"selectedCharacterBorder", L"texturedBorder" };

	elementList = new wxListBox(page, ID_GuiElementList, wxPoint(20,90), wxSize(230,90), 13, str);

	elementNameText = new wxStaticText(page, -1, L"No Element Selected", wxPoint(260,100));
		elementNameText->SetFont( wxFont(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );

	new wxStaticText(page, -1, _("Line width"), wxPoint(130,285));
	lineWidthEdit = new wxTextCtrl(page, -1, L"", wxPoint(145,305), wxSize(30,25));
	lineWidthScroll = new wxScrollBar(page, ID_GuiLineWidthScroll, wxPoint(175,305), wxSize(-1,25), wxSB_VERTICAL);
		lineWidthScroll->SetScrollbar(0,1,10,1);


	textureEdit = new wxTextCtrl(page, -1, L"texture", wxPoint(160,225), wxSize(100,25));
	northEdit = new wxTextCtrl(page, -1, L"north", wxPoint(140,195), wxSize(140,25));
	southEdit = new wxTextCtrl(page, -1, L"south", wxPoint(140,255), wxSize(140,25));
	eastEdit = new wxTextCtrl(page, -1, L"east", wxPoint(270,225), wxSize(100,25));
	westEdit = new wxTextCtrl(page, -1, L"west", wxPoint(50,225), wxSize(100,25));

	northWestEdit = new wxTextCtrl(page, -1, L"north west", wxPoint(25,190), wxSize(100,25));	northWestEdit->Show(false);
	northEastEdit = new wxTextCtrl(page, -1, L"north east", wxPoint(295,190), wxSize(100,25));	northEastEdit->Show(false);
	southWestEdit = new wxTextCtrl(page, -1, L"south west", wxPoint(25,260), wxSize(100,25));	southWestEdit->Show(false);
	southEastEdit = new wxTextCtrl(page, -1, L"south east", wxPoint(295,260), wxSize(100,25));	southEastEdit->Show(false);

	elementColorSelector->Init(page, 30,295, 260,140);

/**
	Colors
**/
	wxString strColors[6] = { L"windowTitleText", L"windowText", L"buttonText", L"buttonSelectionText",
			L"inputText", L"selectionText" };

	colorList = new wxListBox(page, ID_GuiColorList, wxPoint(430,35), wxSize(230,90), 6, strColors);

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

		if ( elementNameText->GetLabel() == L"texturedBorder" )
		{
			TexturedBorder *texturedBorder = (TexturedBorder*)element;
			northWestEdit->SetValue( std2wx(texturedBorder->nw) );
			northEastEdit->SetValue( std2wx(texturedBorder->ne) );
			southWestEdit->SetValue( std2wx(texturedBorder->sw) );
			southEastEdit->SetValue( std2wx(texturedBorder->se) );
		}

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

		if ( elementNameText->GetLabel() == L"texturedBorder" )
		{
			TexturedBorder *texturedBorder = (TexturedBorder*)element;
			texturedBorder->nw = wx2std( northWestEdit->GetValue() );
			texturedBorder->ne = wx2std( northEastEdit->GetValue() );
			texturedBorder->sw = wx2std( southWestEdit->GetValue() );
			texturedBorder->se = wx2std( southEastEdit->GetValue() );
		}

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

	wxString str = pPage->elementList->GetStringSelection();

	// Hack - set the element name here (probably not needed for most calls)
	elementNameText->SetLabel( str );

	bool showCorners;
	if ( str == L"texturedBorder" )
	{
		currentElement = theme->texturedBorder;
		showCorners = true;
	}
	else
	{
		currentElement = theme->elements[ wx2std(str) ];
		showCorners = false;
	}

	northWestEdit->Show(showCorners);
	northEastEdit->Show(showCorners);
	southWestEdit->Show(showCorners);
	southEastEdit->Show(showCorners);

	return currentElement;
}
Color* PageGui::GetSelectedColor()
{
	Theme *theme = dfGui->GetCurrent();

	wxString str = colorList->GetStringSelection();

	// Hack - set the element name here (probably not needed for most calls)
	colorNameText->SetLabel( str );

	currentColor = theme->colors[ wx2std(str) ];
	return currentColor;
}

wxString PageGui::GetSelectedColorName()
{
	wxString str = colorList->GetStringSelection();
	return str;
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
