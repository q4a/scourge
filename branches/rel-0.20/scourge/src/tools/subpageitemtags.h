#ifndef SUBPAGEITEMTAGS_H
#define SUBPAGEITEMTAGS_H

#include "page.h"

/** Forward Declarations **/
class PageRpg;
class DFRpg;
class wxNotebook;
class wxTextCtrl;
class wxListView;

class subPageItemTags : public Page
{
protected:

public:
	PageRpg *parent;
	DFRpg *dfRpg;

public:
	subPageItemTags();
	virtual ~subPageItemTags();

	void Init(wxNotebook*, DF*) {}		// DO NOT USE
	void Init(wxNotebook*, DF*, PageRpg*);

	void UpdatePage();

	void Prev(unsigned int){}
	void Next(unsigned int){}
	void New(){}
	void Del(){}

	void GetCurrent();
	void SetCurrent();
	void GetSkill();
	void SetSkill();
	void ClearCurrent();

	void OnAddTag();
	void OnDelTag();

protected:
	// List of editable controls
	wxTextCtrl *nameEdit;
	wxTextCtrl *descEdit;
	wxListView *list;

};

#endif // SUBPAGEITEMTAGS_H
