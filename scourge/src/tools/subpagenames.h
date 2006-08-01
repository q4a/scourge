#ifndef SUBPAGENAMES_H
#define SUBPAGENAMES_H

#include "page.h"

/** Forward Declarations **/
class PageRpg;
class DFRpg;
class wxNotebook;
class wxTextCtrl;
class wxListView;

class subPageNames : public Page
{
protected:

public:
	PageRpg *parent;
	DFRpg *dfRpg;

public:
	subPageNames();
	virtual ~subPageNames();

	void Init(wxNotebook*, DF*) {}		// DO NOT USE
	void Init(wxNotebook*, DF*, PageRpg*);

	void UpdatePage();

	void Prev(unsigned int){}
	void Next(unsigned int){}
	void New(){}
	void Del(){}

	void GetCurrent();
	void SetCurrent();
	void ClearCurrent();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *descEdit;
	wxListView *list;

};

#endif // SUBPAGENAMES_H
