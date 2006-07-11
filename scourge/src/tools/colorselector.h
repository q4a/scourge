#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include "common.h"

/** Forward Declarations **/
class Color;
class wxWindow;
class wxStaticText;
class wxSlider;
class wxPanel;
class wxMouseEvent;

class ColorSelector
{
protected:
	Color *color;

public:
	ColorSelector();
	virtual ~ColorSelector();

	void Init(wxWindow*,int x,int y, int panelX=-1,int panelY=-1, Color* = 0);

	void GetColor(uchar*,uchar*,uchar*,uchar*);
	Color GetColor();
	void SetColor(uchar,uchar,uchar,uchar);
	void SetColor(Color*);

	void UpdateText();

	void OnSliderChange();
	void OnPanelClick();

protected:
	// List of editable controls
	wxSlider *rSlider;
	wxSlider *gSlider;
	wxSlider *bSlider;
	wxSlider *aSlider;
		wxStaticText *rText;
		wxStaticText *gText;
		wxStaticText *bText;
		wxStaticText *aText;
	wxPanel *panel;

};

#endif // COLORSELECTOR_H
