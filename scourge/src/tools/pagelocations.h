#ifndef PAGELOCATIONS_H
#define PAGELOCATIONS_H

#include "page.h"

/** Forward Declarations **/
class DFLocations;
class Location;
class wxWindow;
class wxTextCtrl;
class wxSpinCtrl;
class wxComboBox;
class wxCheckBox;
class wxString;
class wxMouseEvent;

class PageLocations : public Page
{
protected:
	std::string types[7];

public:
	DFLocations *dfLocations;

public:
	PageLocations();
	virtual ~PageLocations();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	wxString GetType(Location*);

	void UpdateMap(int=-1,int=-1);

	void OnPaint();
	void OnClick(wxMouseEvent& event);

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxSpinCtrl *xSpin;
	wxSpinCtrl *ySpin;
	wxComboBox *typeCombo;
	wxCheckBox *randomCheck;

};

#endif // PAGELOCATIONS_H
