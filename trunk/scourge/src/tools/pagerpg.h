#ifndef PAGERPG_H
#define PAGERPG_H

#include "page.h"

/** Forward Declarations **/
class DFRpg;
class Skill;
class wxWindow;
class wxTextCtrl;
class wxComboBox;
class wxListBox;
class wxArrayString;
class wxStaticText;
class wxSpinCtrl;
class wxString;
class ListAddDel;

class PageRpg : public Page
{
protected:

public:
	DFRpg *dfRpg;
	Skill *currentSkill;
	wxString *currentSkillName;

public:
	PageRpg();
	virtual ~PageRpg();

	void Init(wxNotebook*, DF*);

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

#endif // PAGERPG_H
