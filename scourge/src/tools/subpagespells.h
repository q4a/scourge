#ifndef SUBPAGESPELLS_H
#define SUBPAGESPELLS_H

#include "page.h"
#include <vector>
#include <map>
#include <string>

/** Forward Declarations **/
class DFSpells;
struct School;
struct Spell;
class wxTextCtrl;
class wxStaticText;
class wxScrollBar;
class wxComboBox;
class wxSlider;
class wxWindow;
class wxListBox;
class wxArrayString;
class wxCheckListBox;
class PageSpells;

class subPageSpells : public Page
{
protected:

public:
	PageSpells *parent;
	DFSpells *dfSpells;
	School *currentSchool;
	std::vector<Spell*>::iterator spellItr;
	Spell *currentSpell;
	int spellNumber;

public:
	subPageSpells();
	virtual ~subPageSpells();

	void Init(wxNotebook*, DF*) {}		// DO NOT USE
	void Init(wxNotebook*, DF*, PageSpells*);

	void UpdatePage();

	void Prev();
	void Next();
	void New();
	void Del();

	void UpdatePageNumber();

	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	School* GetSelectedSchool();

	/*void GetSchool();
	void SetSchool();*/

	void OnSchoolChange();
	void OnIconXChange();
	void OnIconYChange();
	void UpdateIcon();

protected:
	// List of editable controls
	wxListBox *schoolList;
		wxArrayString *schoolStrArray;
	wxStaticText *schoolNameText;

	wxTextCtrl *nameEdit;
	wxTextCtrl *symbolEdit;
	wxTextCtrl *levelEdit;
	wxTextCtrl *manaEdit;
	wxTextCtrl *expEdit;
	wxTextCtrl *failureRateEdit;
	wxTextCtrl *actionEdit;
	wxTextCtrl *distanceEdit;
	wxComboBox *areaCombo;
	wxCheckListBox *clipCheckList;
	wxTextCtrl *speedEdit;
	wxTextCtrl *effectEdit;

	wxScrollBar *iconXScroll;
	wxScrollBar *iconYScroll;
	wxTextCtrl *iconXEdit;
	wxTextCtrl *iconYEdit;

	wxComboBox *dispositionCombo;
	wxComboBox *prereqCombo;
	wxTextCtrl *soundEdit;
	wxTextCtrl *notesEdit;
};


#endif // SUBPAGESPELLS_H
