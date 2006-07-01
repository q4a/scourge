#include "page.h"
#include "datafile.h"
#include <wx/wx.h>
#include "common.h"

Page *Page::currentPage;
/*void Page::LoadAll()
{
}*/

void Page::SaveAll()
{
	SetCurrent();
	dataFile->Save();
}

void Page::Prev()
{
	SetCurrent();
		dataFile->Prev();
	GetCurrent();
	UpdatePage();
}

void Page::Next()
{
	SetCurrent();
		dataFile->Next();
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

void Page::UpdatePageNumber()
{
	char buffer[64];
	sprintf(buffer, "Page %i/%i", dataFile->GetCurrentNum(), dataFile->GetTotal());
	g_pageNumText->SetLabel(std2wx(buffer));
}
