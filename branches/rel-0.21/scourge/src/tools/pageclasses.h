#ifndef PAGECLASSES_H
#define PAGECLASSES_H

#include "page.h"
#include <map>

/** Forward Declarations **/
class DFClasses;
class Skill;
class wxWindow;
class wxTextCtrl;
class wxListBox;
class wxArrayString;

struct SkillEdit
{
	wxListBox *list;
		wxArrayString *strArray;
	wxTextCtrl *minEdit;
	wxTextCtrl *maxEdit;
};

class PageClasses : public Page
{
protected:

public:
	DFClasses *dfClasses;

public:
	PageClasses();
	virtual ~PageClasses();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

	void GetSkills();
	void SetSkills();
	void GetSkill(std::string&);
	void SetSkill(std::string&);

	Skill* GetSelectedSkill(std::string&);

	void OnSkillChange();

protected:
	Skill *currentAttribute;

	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *shortNameEdit;
	wxTextCtrl *descEdit;

	wxTextCtrl *hpBonusEdit;
	wxTextCtrl *mpBonusEdit;
	wxTextCtrl *skillBonusEdit;
	wxTextCtrl *attackBonusEdit;
	wxTextCtrl *levelProgressionEdit;
	wxTextCtrl *additionalAttackLevelEdit;

	wxListBox *attributeList;
		wxArrayString *attributeStrArray;
	wxTextCtrl *attributeMinEdit;
	wxTextCtrl *attributeMaxEdit;

	std::map<std::string,SkillEdit*> skillEdits;
	std::map<std::string,Skill*> currentSkills;
};

#endif // PAGECLASSES_H
