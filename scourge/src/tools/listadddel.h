#ifndef LISTADDDEL_H
#define LISTADDDEL_H

#include <string>
#include <vector>

/** Forward Declarations **/
class wxWindow;
class wxListBox;
class wxStaticText;
class wxButton;
class wxString;

class ListAddDel
{
protected:

public:
	ListAddDel();
	virtual ~ListAddDel();

	void Init(wxWindow*, wxString, std::vector<std::string>&, int,int, int=230,int=100);

	void OnAdd();
	void OnDel();

	void Lock(bool=true);
	void Show(bool);

	void Get( std::vector<std::string>& );
	void Set( std::vector<std::string>& );

protected:
	wxListBox *list;
	wxStaticText *text;
	wxButton *add;
	wxButton *del;

};

#endif // LISTADDDEL_H
