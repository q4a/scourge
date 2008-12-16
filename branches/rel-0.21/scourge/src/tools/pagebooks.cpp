#include "pagebooks.h"
#include "dfbooks.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "common.h"

PageBooks::PageBooks()
{
	pageHelp = "Add books to the Scourge data files.";
}

PageBooks::~PageBooks()
{
}

void PageBooks::Init(wxNotebook *notebook, DF *dataFile)
{
	dfBooks = (DFBooks*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_BooksPage);

	Book *book = dfBooks->GetCurrent();

	// name
	/*wxStaticText *nameText =*/ new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, ID_BookNameEdit, std2wx(book->name), wxPoint(10,30), wxSize(300,-1));

	// rareness
	/*wxStaticText *rarenessText =*/ new wxStaticText(page, -1, _("Rareness"), wxPoint(320,10));
	rarenessSpin = new wxSpinCtrl(page, -1, L"", wxPoint(320,30),wxSize(45,-1), wxSP_ARROW_KEYS, 1,10, atoi(book->rareness.c_str()));

	// mission
	/*wxStaticText *missionText = */new wxStaticText(page, -1, _("Mission Name"), wxPoint(10,80));
	missionEdit = new wxTextCtrl(page, ID_BookMissionEdit, std2wx(book->missionName), wxPoint(10,100), wxSize(350,150), wxTE_MULTILINE);

	// text
	/*wxStaticText *textText =*/ new wxStaticText(page, -1, _("Text"), wxPoint(450,10));
	textEdit = new wxTextCtrl(page, ID_BookTextEdit, std2wx(book->text), wxPoint(450,30), wxSize(350,150), wxTE_MULTILINE);

	notebook->AddPage(page, _("Books"));
}

void PageBooks::UpdatePage()
{
}

void PageBooks::GetCurrent()
{
	Book *book = dfBooks->GetCurrent();

	nameEdit->SetValue(std2wx(book->name));
	rarenessSpin->SetValue(std2wx(book->rareness));
	missionEdit->SetValue(std2wx(book->missionName));
	textEdit->SetValue(std2wx(book->text));
}

void PageBooks::SetCurrent()
{
	Book *book = dfBooks->GetCurrent();
	char buffer[16];

	book->name = wx2std( nameEdit->GetValue() );
	sprintf(buffer, "%i", rarenessSpin->GetValue());
	book->rareness = buffer;
	book->missionName = wx2std( missionEdit->GetValue() );
	book->text = wx2std( textEdit->GetValue() );
}

void PageBooks::ClearCurrent()
{
	nameEdit->SetValue(L"");
	rarenessSpin->SetValue(L"");
	missionEdit->SetValue(L"");
	textEdit->SetValue(L"");
}
