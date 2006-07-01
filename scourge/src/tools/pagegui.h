#ifndef PAGEGUI_H
#define PAGEGUI_H

#include "page.h"

/** Forward Declarations **/
class DFGui;
class Element;
class Color;
class wxWindow;
class wxStaticText;
class wxTextCtrl;
class wxScrollBar;
class wxListBox;
class wxString;
class wxArrayString;
class wxSlider;
class wxPanel;

class PageGui : public Page
{
protected:

public:
	DFGui *dfGui;

public:
	PageGui();
	virtual ~PageGui();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	void GetElement();
	void SetElement();
	void GetColor();
	void SetColor();

	Element* GetSelectedElement();
	Color* GetSelectedColor();

	wxString GetSelectedColorName();

	void OnElementChange();
	void OnColorChange();
	void OnElementSliderChange();
	void OnColorSliderChange();

	void OnLineWidthChange();

protected:
	Element *currentElement;
	Color *currentColor;
	wxString *currentElementName;
	wxString *currentColorName;

	// List of editable controls
	wxTextCtrl *nameEdit;
	wxListBox *elementList;
		wxArrayString *elementStrArray;
		wxArrayString *colorStrArray;
	wxListBox *colorList;
	wxStaticText *elementNameText;
	wxStaticText *colorNameText;

	wxScrollBar *lineWidthScroll;

	wxTextCtrl *lineWidthEdit;

	wxTextCtrl *textureEdit;
	wxTextCtrl *northEdit;
	wxTextCtrl *southEdit;
	wxTextCtrl *eastEdit;
	wxTextCtrl *westEdit;

	wxSlider *rElementSlider;
	wxSlider *gElementSlider;
	wxSlider *bElementSlider;
	wxSlider *aElementSlider;
		wxStaticText *rElementText;
		wxStaticText *gElementText;
		wxStaticText *bElementText;
		wxStaticText *aElementText;
	wxPanel *elementColorPanel;

	wxSlider *rColorSlider;
	wxSlider *gColorSlider;
	wxSlider *bColorSlider;
	wxSlider *aColorSlider;
		wxStaticText *rColorText;
		wxStaticText *gColorText;
		wxStaticText *bColorText;
		wxStaticText *aColorText;
	wxPanel *colorPanel;

};

#endif // PAGEGUI_H
