#ifndef DATAFILE_H
#define DATAFILE_H

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>

struct Data
{
};

class DF
{
public:
	virtual bool Load(std::string, std::string) = 0;
	virtual void Save() = 0;

	virtual void Prev() = 0;
	virtual void Next() = 0;
	virtual void New() = 0;
	virtual void Del() = 0;

	virtual unsigned int GetCurrentNum() = 0;
	virtual unsigned int GetTotal() = 0;
};

template <class T>
class DataFile : public DF
{
public:
	std::vector <T*> data;
protected:
	typename std::vector <T*>::iterator current;
	unsigned int currentNum, total;

	virtual bool LoadSingle(std::ifstream*, T* t) = 0;

public:
	DataFile();
	virtual ~DataFile();

	bool Load(std::string, std::string);
	virtual void Save() = 0;

	T* GetCurrent();
	void Prev();
	void Next();
	void New();
	void Del();

	unsigned int GetCurrentNum() { return currentNum; }
	unsigned int GetTotal() { return total; }

};

//****
template <class T>
DataFile<T>::DataFile()
{
	currentNum = 1;
	total = 0;
}

template <class T>
DataFile<T>::~DataFile()
{
	for ( typename std::vector<T*>::iterator itr = data.begin(); itr != data.end(); itr++ )
	{
		delete (*itr);
	}
	data.clear();
}

template <class T>
bool DataFile<T>::Load(std::string fileName, std::string dataStart)
{
	std::ifstream fin(fileName.c_str(), std::ios::binary);
	if ( !fin )
	{
		std::cerr << "\n --- Error loading a data file ---\nfileName: " << fileName << "\n\n";
		return false;
	}
	std::string line;
	char buffer[512];
	T *t;
	char c;

	while ( !fin.eof() )
	{
		c = fin.peek();

		if ( dataStart.find(c) != std::string::npos )
		{
			t = new T;
			if ( !LoadSingle(&fin,t) )
			{
				std::cerr << "\n --- Error loading a data file ---\nfileName: " << fileName << "\n\n";
				delete t;
				continue;
			}
			data.push_back(t);
			total++;
		}
		else
			fin.getline(buffer, 512, '\n');
	}
	if ( data.begin() == data.end() )		// No books in file
	{
		data.push_back(new T);
	}

	fin.close();

	current = data.begin();

	return true;
}

template <class T>
T* DataFile<T>::GetCurrent()
{
	return (*current);
}

template <class T>
void DataFile<T>::Prev()
{
	currentNum--;
	if ( current == data.begin() )
	{
		current = data.end();		currentNum = total;
	}
	current--;
}

template <class T>
void DataFile<T>::Next()
{
	current++;		currentNum++;
	if ( current == data.end() )
	{
		current = data.begin();
		currentNum = 1;
	}
}

template <class T>
void DataFile<T>::New()
{
	T *t = new T;
	data.push_back(t);
	current = data.end();
	current--;
	total++;		currentNum = total;
}

template <class T>
void DataFile<T>::Del()
{
	if ( total == 1 )
		return;

	total--;
	current = data.erase(current);
	if ( current == data.end() )
	{
		current--;
		currentNum--;
	}
}

#endif // DATAFILE_H
