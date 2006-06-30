#ifndef PAGE_H
#define PAGE_H

/** Forward Declarations **/
class wxNotebook;
class wxWindow;
class DF;

class Page
{
protected:
	wxWindow *page;
	DF *dataFile;

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

	virtual void UpdatePage() = 0;

/*	void LoadAll();*/
	void SaveAll();
	virtual void GetCurrent() = 0;
	virtual void SetCurrent() = 0;
	virtual void ClearCurrent() = 0;

	wxWindow* GetPage() { return page; }
	DF *GetDataFile() { return dataFile; }

	void Prev();
	void Next();
	void New();
	void Del();

};

enum
{
	ID_MenuQuit=1,
	ID_MenuSave,
	ID_MenuSaveCurrent,
	ID_MenuAbout,
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
		ID_MissionAddItem,
		ID_MissionDelItem,
		ID_MissionAddCreature,
		ID_MissionDelCreature,
	ID_GuiPage,
		ID_GuiElementList,
		ID_GuiColorList,
		ID_GuiLineWidthScroll,
		ID_GuiElementSlider,
		ID_GuiColorSlider,
	ID_ClassesPage,
		ID_ClassesSkillList,
	ID_SkillsPage,
		ID_SkillsTypeCombo,
		ID_SkillsIconXScroll,
		ID_SkillsIconYScroll,
	ID_SpellsPage,
	ID_Page,
		ID_Prev,
		ID_Next,
		ID_New,
		ID_Del,
		ID_PageNum,
};

#endif // PAGE_H
