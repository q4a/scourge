/**
  * Scourge Data Editor
  */

#include <stdio.h>
#include <wx/wx.h>
	#include <wx/glcanvas.h>
#include <map>
#include "dfbooks.h"
#include "dfmissions.h"
#include "dfgui.h"
#include "dfclasses.h"
#include "dfskills.h"
#include "dfspells.h"
#include "pagebooks.h"
#include "pagemissions.h"
#include "pagegui.h"
#include "pageclasses.h"
#include "pageskills.h"
#include "pagespells.h"
#include "common.h"
#include "../common/constants.h"

std::map <std::string,DF*> g_DFList;
//DF *g_DFCurrent;		-- not needed?

std::map <std::string,Page*> g_PageList;
Page *g_currentPage;

class MyApp : public wxApp
{
	~MyApp()
	{
		for ( std::map<std::string,DF*>::iterator itr = g_DFList.begin(); itr != g_DFList.end(); itr++ )
		{
			delete itr->second;
		}

		for ( std::map<std::string,Page*>::iterator itr = g_PageList.begin(); itr != g_PageList.end(); itr++ )
		{
			delete itr->second;
		}
	}
	virtual bool OnInit();
	virtual bool Initialize(int& argc, wxChar **argv);
};

IMPLEMENT_APP(MyApp)


class MyFrame : public wxFrame
{
public:
	wxNotebookPage *currentPage;
	wxNotebook *notebook;

public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	void OnQuit(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveCurrent(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
		void OnPrev(wxCommandEvent& event);
		void OnNext(wxCommandEvent& event);
		void OnNew(wxCommandEvent& event);
		void OnDel(wxCommandEvent& event);
	void OnButton(wxCommandEvent& event);

	void OnPageChange(wxCommandEvent& event);

		void SaveCurrent();
		void LoadCurrent();
	void UpdatePageNumber();

};

bool MyApp::Initialize( int& argc, wxChar **argv ) {
	// initialize scourge rootDir
	int err = Constants::initRootDir( (int)argc, (char**)argv );
	if( err ) return false;

	// continue initializing our app
	wxApp::Initialize( argc, argv );
}

bool MyApp::OnInit()
{char path[300];

	DFBooks *dfBooks = new DFBooks;
	dfBooks->Load( GetDataPath("%s/world/books.txt"), "B");

	DFMissions *dfMissions = new DFMissions;
	dfMissions->Load( GetDataPath("%s/world/missions.txt"), "MT");

	DFGui *dfGui = new DFGui;
	dfGui->Load( GetDataPath("%s/world/gui.txt"), "T");

//	DFClasses *dfClasses = new DFClasses;
//	dfClasses->Load( GetDataPath("%s/world/characters.txt"), "C");

	DFSkills *dfSkills = new DFSkills;
	dfSkills->Load( GetDataPath("%s/world/skills.txt"), "S");

	DFSpells *dfSpells = new DFSpells;
	dfSpells->Load( GetDataPath("%s/world/spells.txt"), "S");

//	g_DFCurrent = dfBooks;	-- not needed?

	g_DFList["Books"] = dfBooks;
	g_DFList["Missions"] = dfMissions;
	g_DFList["GUI"] = dfGui;
//	g_DFList["Classes"] = dfClasses;
	g_DFList["Skills"] = dfSkills;
	g_DFList["Spells"] = dfSpells;

	g_PageList["Books"] = new PageBooks;
	g_PageList["Missions"] = new PageMissions;
	g_PageList["GUI"] = new PageGui;
//	g_PageList["Classes"] = new PageClasses;
	g_PageList["Skills"] = new PageSkills;
	g_PageList["Spells"] = new PageSpells;

	MyFrame *frame = new MyFrame(_("Scourge Data Editor"), wxPoint(50,50),
                wxSize(840,480));

	frame->Connect( ID_MenuQuit, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction) &MyFrame::OnQuit );
	frame->Connect( ID_MenuSave, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction) &MyFrame::OnSave );
	frame->Connect( ID_MenuSaveCurrent, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction) &MyFrame::OnSaveCurrent );
	frame->Connect( ID_MenuAbout, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction) &MyFrame::OnAbout );

	frame->Connect( ID_Prev, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &MyFrame::OnPrev );
	frame->Connect( ID_Next, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &MyFrame::OnNext );
	frame->Connect( ID_New, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &MyFrame::OnNew );
	frame->Connect( ID_Del, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &MyFrame::OnDel );
	frame->Connect( wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &MyFrame::OnQuit );

	frame->Connect( ID_Notebook, wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
			(wxObjectEventFunction) &MyFrame::OnPageChange );

	// Missions page events
	frame->Connect( ID_MissionStorylineCombo, wxEVT_COMMAND_COMBOBOX_SELECTED,
			(wxObjectEventFunction) &PageMissions::OnStorylineChange );
	frame->Connect( ID_MissionAddItem, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &PageMissions::OnAddItem );
	frame->Connect( ID_MissionDelItem, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &PageMissions::OnDelItem );
	frame->Connect( ID_MissionAddCreature, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &PageMissions::OnAddCreature );
	frame->Connect( ID_MissionDelCreature, wxEVT_COMMAND_BUTTON_CLICKED,
			(wxObjectEventFunction) &PageMissions::OnDelCreature );

	// Gui page events
	frame->Connect( ID_GuiElementList, wxEVT_COMMAND_LISTBOX_SELECTED,
			(wxObjectEventFunction) &PageGui::OnElementChange );
	frame->Connect( ID_GuiColorList, wxEVT_COMMAND_LISTBOX_SELECTED,
			(wxObjectEventFunction) &PageGui::OnColorChange );
	frame->Connect( ID_GuiLineWidthScroll, wxEVT_SCROLL_THUMBTRACK,
			(wxObjectEventFunction) &PageGui::OnLineWidthChange );
	frame->Connect( ID_GuiElementSlider, wxEVT_SCROLL_THUMBTRACK,
			(wxObjectEventFunction) &PageGui::OnElementSliderChange );
	frame->Connect( ID_GuiColorSlider, wxEVT_SCROLL_THUMBTRACK,
			(wxObjectEventFunction) &PageGui::OnColorSliderChange );

	// Classes page events
/*	frame->Connect( ID_ClassesSkillList, wxEVT_COMMAND_LISTBOX_SELECTED,
			(wxObjectEventFunction) &PageClasses::OnSkillChange );
*/
	// Skills page events
	frame->Connect( ID_SkillsTypeCombo, wxEVT_COMMAND_COMBOBOX_SELECTED,
			(wxObjectEventFunction) &PageSkills::OnTypeChange );
	frame->Connect( ID_SkillsIconXScroll, wxEVT_SCROLL_THUMBTRACK,
			(wxObjectEventFunction) &PageSkills::OnIconXChange );
	frame->Connect( ID_SkillsIconYScroll, wxEVT_SCROLL_THUMBTRACK,
			(wxObjectEventFunction) &PageSkills::OnIconYChange );

	frame->Show(TRUE);
	SetTopWindow(frame);

	return TRUE;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL,-1,title,pos,size)
{
	// Create menubar
	wxMenuBar *menuBar = new wxMenuBar;
	// Create menus
	wxMenu *menuFile = new wxMenu;
	wxMenu *menuHelp = new wxMenu;
	// Append menu entries
	menuFile->Append(ID_MenuSave,_("&Save All"));
	menuFile->Append(ID_MenuSaveCurrent,L"Save &Current");
	menuFile->AppendSeparator();
	menuFile->Append(ID_MenuQuit,_("E&xit"));
	menuHelp->Append(ID_MenuAbout,_("&About"));
	// Append menus to menubar
	menuBar->Append(menuFile,_("&File"));
	menuBar->Append(menuHelp,_("&Help"));
	// Set frame menubar
	SetMenuBar(menuBar);

/* Notebook */
	notebook = new wxNotebook(this, ID_Notebook, wxDefaultPosition, wxSize(840,385));
	wxPanel *panel = new wxPanel(this, -1, wxPoint(0,385), wxSize(840,200));

	// Pages
	g_PageList["Books"]->Init(notebook,g_DFList["Books"]);
	g_PageList["Missions"]->Init(notebook,g_DFList["Missions"]);
	g_PageList["GUI"]->Init(notebook,g_DFList["GUI"]);
//	g_PageList["Classes"]->Init(notebook,g_DFList["Classes"]);
	g_PageList["Skills"]->Init(notebook,g_DFList["Skills"]);
	g_PageList["Spells"]->Init(notebook,g_DFList["Spells"]);

	// prev
	wxButton *prev = new wxButton(panel, ID_Prev,_("<"),wxPoint(10,5),wxSize(20,30));
	// page number
	char buffer[64];
	sprintf(buffer, "Page %i/%i", g_DFList["Books"]->GetCurrentNum(), g_DFList["Books"]->GetTotal());
	wxStaticText *bookPageNumText = new wxStaticText(panel, ID_PageNum, std2wx( buffer ), wxPoint(35,15));
	// next
	wxButton *next = new wxButton(panel, ID_Next,_(">"),wxPoint(100,5),wxSize(20,30));
	// new
	wxButton *newBook = new wxButton(panel, ID_New,_("New"),wxPoint(200,5),wxSize(50,30));
	// del
	wxButton *delBook = new wxButton(panel, ID_Del,_("Delete"),wxPoint(260,5),wxSize(50,30));
	// exit
	wxButton *button = new wxButton(panel, wxID_EXIT,_(""),wxPoint(320,5));


	g_currentPage = g_PageList["Books"];
	Page::currentPage = g_currentPage;
	currentPage = g_currentPage->GetPage();

	// create frame statusbar
	CreateStatusBar();
	// set statusbar text
	SetStatusText(_("Welcome to Scourge Data Editor!"));
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void MyFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	/*for ( std::map<std::string,Page*>::iterator itr = g_PageList.begin(); itr != g_PageList.end(); itr++ )
	{
		itr->second->SaveAll();
	}

	wxMessageBox(_("All data files saved."),_("Save complete."),
			wxOK|wxICON_EXCLAMATION, this);*//*
	wxMessageBox(_("Not complete yet."),_("To be done."),
			wxOK|wxICON_EXCLAMATION, this);*/
	wxMessageDialog dialog(this, L"This will save all data files. Do you want to continue?", L"Save all data files?",
			wxYES_NO|wxNO_DEFAULT|wxICON_EXCLAMATION);
	if ( dialog.ShowModal() == wxID_YES )
	{
		g_PageList["Books"]->SaveAll();
		g_PageList["Missions"]->SaveAll();
		g_PageList["GUI"]->SaveAll();
	}
}
void MyFrame::OnSaveCurrent(wxCommandEvent& WXUNUSED(event))
{
	wxMessageDialog dialog(this, L"This will save all records in the current data file. Do you want to continue?", L"Save current data file?",
			wxYES_NO|wxNO_DEFAULT|wxICON_EXCLAMATION);
	if ( dialog.ShowModal() == wxID_YES )
		g_currentPage->SaveAll();
//	wxMessageBox(_("Not complete yet."),_("To be done."),
//			wxOK|wxICON_EXCLAMATION, this);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(_("Edit the Scourge data files."),_("About Scourge Data Editor"),
                wxOK|wxICON_INFORMATION, this);
}

void MyFrame::UpdatePageNumber()
{
	char buffer[64];
	sprintf(buffer, "Page %i/%i", g_currentPage->GetDataFile()->GetCurrentNum(), g_currentPage->GetDataFile()->GetTotal());
	wxWindow *w = FindWindow(ID_PageNum);		((wxStaticText*)w)->SetLabel(std2wx(buffer));
}

void MyFrame::OnPrev(wxCommandEvent& WXUNUSED(event))
{
	g_currentPage->Prev();
	UpdatePageNumber();
}
void MyFrame::OnNext(wxCommandEvent& WXUNUSED(event))
{
	g_currentPage->Next();
	UpdatePageNumber();
}
void MyFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
	g_currentPage->New();
	UpdatePageNumber();
}
void MyFrame::OnDel(wxCommandEvent& WXUNUSED(event))
{
	g_currentPage->Del();
	UpdatePageNumber();
}

void MyFrame::OnPageChange(wxCommandEvent& WXUNUSED(event))
{
	currentPage = notebook->GetCurrentPage();
	wxString str = notebook->GetPageText( notebook->GetSelection() );
//	g_DFCurrent = g_DFList[ wx2std(str) ];		-- not needed?
	g_currentPage = g_PageList[ wx2std(str) ]->SetAsCurrent();

	UpdatePageNumber();
}
