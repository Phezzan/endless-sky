/* DataWriter.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DataWriter.h"

#include "DataNode.h"
#include "Files.h"

using namespace std;

const string DataWriter::space = " ";



DataWriter::DataWriter(const string &path)
	: path(path), before(&indent)
{
	out.precision(8);
}



DataWriter::~DataWriter()
{
	Files::Write(path, out.str());
}



void DataWriter::Write(const DataNode &node)
{
	lock_guard<mutex> lock(writeMutex);
	WriteLock(node);
}


void DataWriter::WriteLock(const DataNode &node)
{
	for(int i = 0; i < node.Size(); ++i)
		WriteToken(node.Token(i).c_str());
	WriteLock();
	
	if(node.begin() != node.end())
	{
		BeginChildLock();
		{
			for(const DataNode &child : node)
				WriteLock(child);
		}
		EndChildLock();
	}
}

void DataWriter::Write()
{
	lock_guard<mutex> lock(writeMutex);
	out << '\n';
	before = &indent;
}

void DataWriter::WriteLock()
{
	out << '\n';
	before = &indent;
}


void DataWriter::BeginChild()
{
	lock_guard<mutex> lock(writeMutex);
	indent += '\t';
}

void DataWriter::EndChild()
{
	lock_guard<mutex> lock(writeMutex);
	indent.erase(indent.length() - 1);
}

void DataWriter::BeginChildLock()
{
	indent += '\t';
}

void DataWriter::EndChildLock()
{
	indent.erase(indent.length() - 1);
}



void DataWriter::WriteComment(const string &str)
{
	lock_guard<mutex> lock(writeMutex);
	out << indent << "# " << str << '\n';
}



void DataWriter::WriteToken(const char *a)
{
	bool hasSpace = !*a;
	bool hasQuote = false;

	for(const char *it = a; *it; ++it)
	{
		hasSpace |= (*it <= ' ');
		hasQuote |= (*it == '"');
	}
	
	out << *before;
	if(hasSpace && hasQuote)
		out << '`' << a << '`';
	else if(hasSpace)
		out << '"' << a << '"';
	else
		out << a;
	before = &space;
}
