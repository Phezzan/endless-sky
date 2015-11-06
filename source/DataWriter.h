/* DataWriter.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DATA_WRITER_H_
#define DATA_WRITER_H_

#include <string>
#include <sstream>
#include <mutex>

class DataNode;

using namespace std;


// This class writes data in a hierarchical format, where an indented line is
// considered the "child" of the first line above it that is less indented. By
// using this class, you can have a function add data to the file without having
// to tell that function what indentation level it is at. This class also
// automatically adds quotation marks around strings if they contain whitespace.
class DataWriter {
public:
	DataWriter(const string &path);
	~DataWriter();
	
  template <class ...B>
	void Write(const char *a, B... others);
  template <class ...B>
	void Write(const string &a, B... others);
  template <class A, class ...B>
	void Write(const A &a, B... others);
	void Write(const DataNode &node);
	void Write();
	
	void BeginChild();
	void EndChild();
	
	void WriteComment(const string &str);
	
	
private:
	void WriteToken(const char *a);
	
	template <class ...B>
		void WriteLock(const char *a, B... others);
	template <class ...B>
		void WriteLock(const string &a, B... others);
	template <class A, class ...B>
		void WriteLock(const A &a, B... others);
	void WriteLock(const DataNode &node);
	void WriteLock();
	void BeginChildLock();
	void EndChildLock();

private:

	string path;
	string indent;
	static const string space;
	const string *before;
	ostringstream out;
	mutex writeMutex;
};



template <class ...B>
void DataWriter::Write(const char *a, B... others)
{
	lock_guard<mutex> lock(writeMutex);
	WriteToken(a);
	WriteLock(others...);
}



template <class ...B>
void DataWriter::Write(const string &a, B... others)
{
	lock_guard<mutex> lock(writeMutex);
	WriteLock(a.c_str(), others...);
}



template <class A, class ...B>
void DataWriter::Write(const A &a, B... others)
{
	static_assert(is_arithmetic<A>::value,
		"DataWriter cannot output anything but strings and arithmetic types.");

	lock_guard<mutex> lock(writeMutex);
	out << *before << a;
	before = &space;

	WriteLock(others...);
}


template <class ...B>
void DataWriter::WriteLock(const char *a, B... others)
{
	WriteToken(a);
	WriteLock(others...);
}



template <class ...B>
void DataWriter::WriteLock(const string &a, B... others)
{
	WriteLock(a.c_str(), others...);
}



template <class A, class ...B>
void DataWriter::WriteLock(const A &a, B... others)
{
	static_assert(is_arithmetic<A>::value,
		"DataWriter cannot output anything but strings and arithmetic types.");
	
	out << *before << a;
	before = &space;
	
	WriteLock(others...);
}


#endif
