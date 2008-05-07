#ifndef PAGEBOOKS_H
#define PAGEBOOKS_H

#include "page.h"

/** Forward Declarations **/
class DFBooks;
class wxWindow;
class wxTextCtrl;
class wxSpinCtrl;

class PageBooks : public Page
{
protected:

public:
	DFBooks *dfBooks;

public:
	PageBooks();
	virtual ~PageBooks();

	void Init(wxNotebook*, DF*);

	void UpdatePage();

/*	void LoadAll();*/
/*	void SaveAll();*/
	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxSpinCtrl *rarenessSpin;
	wxTextCtrl *missionEdit;
	wxTextCtrl *textEdit;

};

#endif // PAGEBOOKS_H
