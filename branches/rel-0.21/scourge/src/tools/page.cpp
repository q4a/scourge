#include "page.h"
#include "datafile.h"
#include <wx/wx.h>
#include "common.h"

Page *Page::currentPage;
/*void Page::LoadAll()
{
}*/

void Page::OnPageHelp()
{
	wxMessageBox(std2wx(pageHelp), L"Page help",
                wxOK|wxICON_INFORMATION, 0);
}

void Page::SaveAll()
{
	SetCurrent();
	dataFile->Save();
}

void Page::Prev(int n)
{
	SetCurrent();
		dataFile->Prev(n);
	GetCurrent();
	UpdatePage();
}

void Page::Next(int n)
{
	SetCurrent();
		dataFile->Next(n);
	GetCurrent();
	UpdatePage();
}

void Page::New()
{
	SetCurrent();
		dataFile->New();
	GetCurrent();
	UpdatePage();
}

void Page::Del()
{
	if ( dataFile->GetTotal() == 1 )
		ClearCurrent();
	else
		dataFile->Del();
	GetCurrent();
	UpdatePage();
}

void Page::JumpTo(int n)
{
	SetCurrent();
		dataFile->JumpTo(n);
	GetCurrent();
	UpdatePage();
}

void Page::UpdatePageNumber()
{
	char buffer[64];
	sprintf(buffer, "Page %i/%i", dataFile->GetCurrentNum(), dataFile->GetTotal());
	g_pageNumText->SetLabel(std2wx(buffer));
}
