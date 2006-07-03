#include "colorselector.h"
#include <wx/wx.h>
#include <wx/colordlg.h>
#include "../common/constants.h"

ColorSelector::ColorSelector()
{
	//ctor
}

ColorSelector::~ColorSelector()
{
	//dtor
}

void ColorSelector::Init(wxWindow *parent, int x,int y, int panelX,int panelY)
{
	if ( panelX == -1 )		panelX = x+100;
	if ( panelY == -1 )		panelY = y-30;

	rText = new wxStaticText(parent, -1, L"Red: ",	wxPoint(x+20,	y));  rText->SetForegroundColour(wxColor(255,0,0));
	gText = new wxStaticText(parent, -1, L"Green: ",	wxPoint(x+110,	y));  gText->SetForegroundColour(wxColor(0,255,0));
	bText = new wxStaticText(parent, -1, L"Blue: ",	wxPoint(x+200,	y));  bText->SetForegroundColour(wxColor(0,0,255));
	aText = new wxStaticText(parent, -1, L"Alpha: ",	wxPoint(x+290,	y));

	// Sliders
	rSlider = new wxSlider(parent, -1, 0,0,1000, wxPoint(x	,y+20),wxSize(90,-1));
	gSlider = new wxSlider(parent, -1, 0,0,1000, wxPoint(x+90 ,y+20),wxSize(90,-1));
	bSlider = new wxSlider(parent, -1, 0,0,1000, wxPoint(x+180,y+20),wxSize(90,-1));
	aSlider = new wxSlider(parent, -1, 0,0,1000, wxPoint(x+270,y+20),wxSize(90,-1));
	// Events
	rSlider->Connect( wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ColorSelector::OnSliderChange, NULL, (wxEvtHandler*)this);
	gSlider->Connect( wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ColorSelector::OnSliderChange, NULL, (wxEvtHandler*)this);
	bSlider->Connect( wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ColorSelector::OnSliderChange, NULL, (wxEvtHandler*)this);
	aSlider->Connect( wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ColorSelector::OnSliderChange, NULL, (wxEvtHandler*)this);

	panel = new wxPanel(parent, -1, wxPoint(panelX,panelY),wxSize(130,25));
	panel->Connect( wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ColorSelector::OnPanelClick, NULL, (wxEvtHandler*)this);
}

void ColorSelector::GetColor(uchar *r, uchar *g, uchar *b, uchar *a)
{
	*r = (uchar)( (rSlider->GetValue() / 1000.0f) * 255 );
	*g = (uchar)( (gSlider->GetValue() / 1000.0f) * 255 );
	*b = (uchar)( (bSlider->GetValue() / 1000.0f) * 255 );
	*a = (uchar)( (aSlider->GetValue() / 1000.0f) * 255 );
}
Color ColorSelector::GetColor()
{
	float r = rSlider->GetValue() / 1000.0f;
	float g = gSlider->GetValue() / 1000.0f;
	float b = bSlider->GetValue() / 1000.0f;
	float a = aSlider->GetValue() / 1000.0f;

	return Color( r,g,b,a );
}

void ColorSelector::SetColor(uchar r, uchar g, uchar b, uchar a)
{
	rSlider->SetValue( (int)(((float)r/255.0f)*1000.0f) );
	gSlider->SetValue( (int)(((float)g/255.0f)*1000.0f) );
	bSlider->SetValue( (int)(((float)b/255.0f)*1000.0f) );
	aSlider->SetValue( (int)(((float)a/255.0f)*1000.0f) );

	char buffer[64];
	sprintf(buffer, "%.3f", r);
	rText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	gText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	bText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", a);
	aText->SetLabel( std2wx( std::string(buffer) ) );

	panel->SetBackgroundColour( wxColour(r,g,b) );
}
void ColorSelector::SetColor(Color *c)
{
	rSlider->SetValue( (int)((c->r)*1000.0f) );
	gSlider->SetValue( (int)((c->g)*1000.0f) );
	bSlider->SetValue( (int)((c->b)*1000.0f) );
	aSlider->SetValue( (int)((c->a)*1000.0f) );

	char buffer[64];
	sprintf(buffer, "%.3f", c->r);
	rText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", c->g);
	gText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", c->b);
	bText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", c->a);
	aText->SetLabel( std2wx( std::string(buffer) ) );

	panel->SetBackgroundColour( wxColour((uchar)(c->r*255),(uchar)(c->g*255),(uchar)(c->b*255)) );
}

void ColorSelector::OnSliderChange()
{
	float r = rSlider->GetValue() / 1000.0f;
	float g = gSlider->GetValue() / 1000.0f;
	float b = bSlider->GetValue() / 1000.0f;
	float a = aSlider->GetValue() / 1000.0f;

	char buffer[64];
	sprintf(buffer, "%.3f", r);
	rText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	gText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	bText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", a);
	aText->SetLabel( std2wx( std::string(buffer) ) );

	panel->SetBackgroundColour( wxColour((uchar)(r*255),(uchar)(g*255),(uchar)(b*255)) );
}
void ColorSelector::OnPanelClick()
{
	float r = ((float)rSlider->GetValue())/1000.0f;
	float g = ((float)gSlider->GetValue())/1000.0f;
	float b = ((float)bSlider->GetValue())/1000.0f;

	wxColourData colorData;
	colorData.SetColour( wxColour( (uchar)(r*255),(uchar)(g*255),(uchar)(b*255) ) );
	wxColourDialog colorDialog(0, &colorData);
	colorDialog.ShowModal();
	wxColour color = colorDialog.GetColourData().GetColour();

	panel->SetBackgroundColour( wxColour(color.Red(),color.Green(),color.Blue()) );

	r = ((float)color.Red())/255.0f;
	g = ((float)color.Green())/255;
	b = ((float)color.Blue())/255;
	rSlider->SetValue((int)(r*1000));
	gSlider->SetValue((int)(g*1000));
	bSlider->SetValue((int)(b*1000));

	char buffer[16];
	sprintf(buffer, "%.3f", r);
	rText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", g);
	gText->SetLabel( std2wx( std::string(buffer) ) );
	sprintf(buffer, "%.3f", b);
	bText->SetLabel( std2wx( std::string(buffer) ) );
}
