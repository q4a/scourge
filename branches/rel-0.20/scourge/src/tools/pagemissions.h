#ifndef PAGEMISSIONS_H
#define PAGEMISSIONS_H

#include "page.h"

/** Forward Declarations **/
class DFMissions;
class wxWindow;
class wxTextCtrl;
class wxComboBox;
class wxListBox;
class wxArrayString;
class wxStaticText;
class wxSpinCtrl;
class ListAddDel;

class PageMissions : public Page
{
protected:

public:
	DFMissions *dfMissions;

public:
	PageMissions();
	virtual ~PageMissions();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	void ShowStoryControls(bool);

	void OnStorylineChange();
	void OnPaint();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxComboBox *typeCombo;
	wxComboBox *storylineCombo;
	wxStaticText *levelText;
	wxStaticText *storiesText;
	wxStaticText *mapText;
	wxStaticText *specialText;
	wxSpinCtrl *levelSpin;
	wxSpinCtrl *storiesSpin;
	wxTextCtrl *mapEdit;
	wxTextCtrl *descEdit;
	wxTextCtrl *succEdit;
	wxTextCtrl *failEdit;
	wxTextCtrl *specialEdit;

	ListAddDel *itemList;
	ListAddDel *creatureList;

};

#endif // PAGEMISSIONS_H
