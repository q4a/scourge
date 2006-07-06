#include "colorselector.h"
#include <wx/wx.h>
#include <wx/colordlg.h>
#include <wx/spinctrl.h>
#include "../common/constants.h"

ColorSelector::ColorSelector()
{
	//ctor
	color = new Color;
}

ColorSelector::~ColorSelector()
{
	//dtor
	delete color;
}

void ColorSelector::Init(wxWindow *parent, int x,int y, int panelX,int panelY)
{
	if ( panelX == -1 )		panelX = x+100;
	if ( panelY == -1 )		panelY = y-30;

	aText = new wxStaticText(parent, -1, L"Alpha: ",	wxPoint(x+20,	y));

	// Sliders
	aSlider = new wxSlider(parent, -1, 0,0,1000, wxPoint(x,y+20),wxSize(90,-1));
	// Events
	aSlider->Connect( wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ColorSelector::OnSliderChange, NULL, (wxEvtHandler*)this);

	panel = new wxPanel(parent, -1, wxPoint(panelX,panelY),wxSize(130,25));
	panel->Connect( wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ColorSelector::OnPanelClick, NULL, (wxEvtHandler*)this);
}

void ColorSelector::GetColor(uchar *r, uchar *g, uchar *b, uchar *a)
{
	*r = (uchar)( color->r * 255 );
	*g = (uchar)( color->g * 255 );
	*b = (uchar)( color->b * 255 );
	*a = (uchar)( (aSlider->GetValue() / 1000.0f) * 255 );
}
Color ColorSelector::GetColor()
{
	float a = aSlider->GetValue() / 1000.0f;

	return Color( color->r,color->g,color->b,a );
}

void ColorSelector::SetColor(uchar r, uchar g, uchar b, uchar a)
{
	aSlider->SetValue( (int)(((float)a/255.0f)*1000.0f) );

	char buffer[64];
	sprintf(buffer, "%.3f", a);
	aText->SetLabel( std2wx( std::string(buffer) ) );

	panel->SetBackgroundColour( wxColour(r,g,b) );
}
void ColorSelector::SetColor(Color *c)
{
	color->r = c->r;
	color->g = c->g;
	color->b = c->b;
	aSlider->SetValue( (int)((c->a)*1000.0f) );

	char buffer[64];
	sprintf(buffer, "%.3f", c->a);
	aText->SetLabel( std2wx( std::string(buffer) ) );

	panel->SetBackgroundColour( wxColour((uchar)(c->r*255),(uchar)(c->g*255),(uchar)(c->b*255)) );
}

void ColorSelector::OnSliderChange()
{
	float a = aSlider->GetValue() / 1000.0f;

	char buffer[64];
	sprintf(buffer, "%.3f", a);
	aText->SetLabel( std2wx( std::string(buffer) ) );
}

class ColorDialog : public wxColourDialog
{
public:
	uchar a;

	ColorDialog(): wxColourDialog(0)
	{
//		wxSpinCtrl *s = new wxSpinCtrl(this, -1, L"", wxPoint(100,100));
	}

protected:

};
void ColorSelector::OnPanelClick()
{
//	wxColourData colorData;
//	colorData.SetColour( wxColour( (uchar)(color->r*255),(uchar)(color->g*255),(uchar)(color->b*255) ) );
	/*wxColourDialog*/ColorDialog colorDialog;//0, &colorData);
	colorDialog.ShowModal();
	wxColour c = colorDialog.GetColourData().GetColour();
	panel->SetBackgroundColour( wxColour(c.Red(),c.Green(),c.Blue()) );

	color->r = ((float)c.Red())/255.0f;
	color->g = ((float)c.Blue())/255.0f;
	color->b = ((float)c.Green())/255.0f;
}
