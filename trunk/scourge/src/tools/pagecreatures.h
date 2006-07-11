#ifndef PAGECREATURES_H
#define PAGECREATURES_H

#include "page.h"

/** Forward Declarations **/
class DFCreatures;
class wxWindow;
class wxTextCtrl;
class wxComboBox;
class wxSpinCtrl;
class wxListBox;
class wxArrayString;
class wxListView;
class ListAddDel;

class PageCreatures : public Page
{
protected:

public:
	DFCreatures *dfCreatures;

public:
	PageCreatures();
	virtual ~PageCreatures();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	void OnAddSkill();
	void OnDelSkill();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *portraitEdit;
	wxTextCtrl *md2Edit;
	wxTextCtrl *skinEdit;
	wxSpinCtrl *levelSpin;
	wxSpinCtrl *hpSpin;
	wxSpinCtrl *mpSpin;
	wxSpinCtrl *armorSpin;
	wxSpinCtrl *rarenessSpin;
	wxSpinCtrl *speedSpin;
	wxTextCtrl *scaleEdit;
	wxTextCtrl *npcEdit;
	wxSpinCtrl *npcStartXSpin;
	wxSpinCtrl *npcStartYSpin;
	ListAddDel *invList;
	ListAddDel *spellList;

	wxListView *skillList;
};

#endif // PAGECREATURES_H
