#ifndef PAGESKILLS_H
#define PAGESKILLS_H

#include "page.h"

/** Forward Declarations **/
class DFSkills;
class wxTextCtrl;
class wxComboBox;
class wxSpinCtrl;
class wxBitmap;

class PageSkills : public Page
{
protected:

public:
	DFSkills *dfSkills;

public:
	PageSkills();
	virtual ~PageSkills();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();


	void OnTypeChange();
	void UpdateIcon();

	void OnPaint();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *squirrelNameEdit;

	wxComboBox *typeCombo;
	wxComboBox *eventCombo;

	wxSpinCtrl *iconXSpin;
	wxSpinCtrl *iconYSpin;
		wxBitmap *bitmap;
	wxTextCtrl *descriptionEdit;
};

#endif // PAGESKILLS_H
