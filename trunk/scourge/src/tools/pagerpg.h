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
class subPageSkills;

class PageRpg : public Page
{
protected:

public:
	DFRpg *dfRpg;
//	Skill *currentSkill;
//	wxString *currentSkillName;
	subPageSkills *pageSkills;

public:
	PageRpg();
	virtual ~PageRpg();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

	void Prev(int=1);
	void Next(int=1);
	void New();
	void Del();

	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	void OnSubPageChange();

protected:
	wxNotebook *subNotebook;
	Page *currentSubPage;

};

#endif // PAGERPG_H
