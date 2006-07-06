#ifndef PAGE_H
#define PAGE_H

#include <string>

/** Forward Declarations **/
class wxNotebook;
class wxWindow;
class DF;

class Page
{
protected:
	wxWindow *page;
	DF *dataFile;
	std::string pageHelp;

public:
	static Page *currentPage;
	Page* SetAsCurrent()
	{
		currentPage = this;
//		UpdatePage();
		return this;
	}

public:
	virtual void Init(wxNotebook*, DF*) = 0;
	void OnPageHelp();

	virtual void UpdatePage() = 0;

/*	void LoadAll();*/
	void SaveAll();
	virtual void GetCurrent() = 0;
	virtual void SetCurrent() = 0;
	virtual void ClearCurrent() = 0;

	wxWindow* GetPage() { return page; }
	DF *GetDataFile() { return dataFile; }

	virtual void Prev(int=1);
	virtual void Next(int=1);
	virtual void New();
	virtual void Del();
	virtual void JumpTo(int);

	virtual void UpdatePageNumber();

};

enum
{
	ID_MenuQuit=1,
	ID_MenuSave,
	ID_MenuSaveCurrent,
	ID_MenuAbout,
	ID_MenuPageHelp,
	ID_Button,
	ID_Notebook,
	ID_BooksPage,
		ID_BookNameEdit,
		ID_BookRarenessEdit,
		ID_BookMissionEdit,
		ID_BookTextEdit,
	ID_MissionsPage,
		ID_MissionNameEdit,
		ID_MissionTypeCombo,
		ID_MissionStorylineCombo,
		ID_MissionDescEdit,
		ID_MissionSuccEdit,
		ID_MissionFailEdit,
	ID_GuiPage,
		ID_GuiElementList,
		ID_GuiColorList,
		ID_GuiLineWidthScroll,
		ID_GuiElementSlider,
		ID_GuiColorSlider,
		ID_GuiColorPanel,
	ID_ClassesPage,
		ID_ClassesSkillList,
	ID_SkillsPage,
		ID_SkillsTypeCombo,
		ID_SkillsIconXSpin,
		ID_SkillsIconXScroll,
		ID_SkillsIconYScroll,
	ID_SpellsPage,
		ID_SpellsColorSlider,
		ID_SpellsSubNotebook,
		ID_Schools_subPage,
		ID_Spells_subPage,
		ID_subSpellsSchoolList,
		ID_subSpellsIconXScroll,
		ID_subSpellsIconYScroll,
	ID_CreaturesPage,
	ID_Page,
		ID_Prev,
		ID_PrevFast,
		ID_Next,
		ID_NextFast,
		ID_JumpTo,
		ID_New,
		ID_Del,
		ID_PageNum,
};

#endif // PAGE_H
