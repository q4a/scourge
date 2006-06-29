#ifndef PAGESKILLS_H
#define PAGESKILLS_H

#include "page.h"

/** Forward Declarations **/
class DFSkills;
class wxTextCtrl;
class wxScrollBar;
class wxComboBox;

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
	void OnIconXChange();
	void OnIconYChange();
	void UpdateIcon();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *squirrelNameEdit;

	wxComboBox *typeCombo;
	wxComboBox *eventCombo;

	wxScrollBar *iconXScroll;
	wxScrollBar *iconYScroll;
	wxTextCtrl *iconXEdit;
	wxTextCtrl *iconYEdit;
	wxTextCtrl *descriptionEdit;
};

#endif // PAGESKILLS_H
