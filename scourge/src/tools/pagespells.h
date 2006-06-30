#ifndef PAGESPELLS_H
#define PAGESPELLS_H

#include "page.h"

/** Forward Declarations **/
class DFSpells;
class wxTextCtrl;
class wxScrollBar;
class wxComboBox;

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

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();


	void OnIconXChange();
	void OnIconYChange();
	void UpdateIcon();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;

	wxComboBox *typeCombo;
	wxComboBox *eventCombo;

	wxScrollBar *iconXScroll;
	wxScrollBar *iconYScroll;
	wxTextCtrl *iconXEdit;
	wxTextCtrl *iconYEdit;
	wxTextCtrl *descriptionEdit;
};

#endif // PAGESPELLS_H
