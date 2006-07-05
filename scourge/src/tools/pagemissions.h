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

	void OnStorylineChange();
	void OnAddItem();
	void OnDelItem();
	void OnAddCreature();
	void OnDelCreature();

	void OnPaint();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxComboBox *typeCombo;
	wxComboBox *storylineCombo;
	wxTextCtrl *descEdit;
	wxListBox *itemList;
		wxArrayString *itemStrArray;
		wxArrayString *creatureStrArray;
	wxListBox *creatureList;
	wxTextCtrl *succEdit;
	wxTextCtrl *failEdit;

};

#endif // PAGEMISSIONS_H
