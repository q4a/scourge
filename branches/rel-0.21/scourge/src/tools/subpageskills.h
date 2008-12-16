#ifndef SUBPAGESKILLS_H
#define SUBPAGESKILLS_H

#include "page.h"

/** Forward Declarations **/
class PageRpg;
class DFRpg;
class Skill;
class wxWindow;
class wxNotebook;
class wxTextCtrl;
class wxComboBox;
class wxListBox;
class wxArrayString;
class wxStaticText;
class wxSpinCtrl;
class wxString;
class ListAddDel;

class subPageSkills : public Page
{
protected:

public:
	PageRpg *parent;
	DFRpg *dfRpg;
	Skill *currentSkill;
	wxString *currentSkillName;

public:
	subPageSkills();
	virtual ~subPageSkills();

	void Init(wxNotebook*, DF*) {}		// DO NOT USE
	void Init(wxNotebook*, DF*, PageRpg*);

	void UpdatePage();

	void GetCurrent();
	void SetCurrent();
	void GetSkill();
	void SetSkill();
	void ClearCurrent();

	Skill* GetSelectedSkill();

	void OnSkillChange();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *descEdit;
	wxTextCtrl *symbolEdit;
	wxTextCtrl *skillDescEdit;
	wxSpinCtrl *multiplierSpin;
	wxStaticText *multiplierText;

	ListAddDel *skillList;
		wxStaticText *skillNameText;
	ListAddDel *statList;

};

#endif // SUBPAGESKILLS_H
