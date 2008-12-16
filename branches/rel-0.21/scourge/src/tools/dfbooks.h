#ifndef DFBOOKS_H
#define DFBOOKS_H

#include "datafile.h"

struct Book : public Data
{
	std::string name;
	bool missionSpecific;
	std::string missionName;
	std::string text;
	std::string rareness;
	Book() : name(""), missionSpecific(false), missionName(""), text(""), rareness("") {}
};

class DFBooks : public DataFile <Book>
{
protected:
	bool LoadSingle(std::ifstream*, Book*);
public:
	void Save();
};
/*class DFBooks
{
private:
	std::list <Book*> books;
	std::list<Book*>::iterator currentBook;
	unsigned int currentBookNum, totalBooks;

	void LoadBook(std::ifstream*, Book*);

public:
	DFBooks();
	virtual ~DFBooks();

	bool Load();
	void Save();

	Book* GetBook();
	void NextBook();
	void PrevBook();
	void NewBook();

	unsigned int GetCurrentBookNum() { return currentBookNum; }
	unsigned int GetTotalBooks() { return totalBooks; }

};*/

#endif // DFBOOKS_H
