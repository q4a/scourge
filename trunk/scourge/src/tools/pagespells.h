#ifndef PAGESPELLS_H
#define PAGESPELLS_H

#include "page.h"

/** Forward Declarations **/
class DFSpells;
class wxTextCtrl;
class wxScrollBar;
class wxComboBox;
class wxSlider;
class wxWindow;
class subPageSchools;
class subPageSpells;
class wxCommandEvent;

class PageSpells : public Page
{
protected:

public:
	DFSpells *dfSpells;

public:
	PageSpells();
	virtual ~PageSpells();

	void Init(wxNotebook*, DF*);

	void UpdatePage();


	void Prev();
	void Next();

	void UpdatePageNumber();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();


	void OnSubPageChange(wxCommandEvent& event);
	void OnIconXChange();
	void OnIconYChange();
	void UpdateIcon();

protected:
	wxNotebook *subNotebook;
	subPageSchools *pageSchools;
	subPageSpells *pageSpells;
	Page *currentSubPage;
};

#endif // PAGESPELLS_H
