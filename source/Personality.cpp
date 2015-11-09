/* Personality.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Personality.h"

#include "Angle.h"
#include "DataNode.h"
#include "DataWriter.h"

#include <map>

using namespace std;

namespace {
	
	static const map<string, int> TOKEN = {
		{"pacifist", Personality::PACIFIST},
		{"forbearing", Personality::FORBEARING},
		{"timid", Personality::TIMID},
		{"disables", Personality::DISABLES},
		{"plunders", Personality::PLUNDERS},
		{"heroic", Personality::HEROIC},
		{"staying", Personality::STAYING},
		{"entering", Personality::ENTERING},
		{"nemesis", Personality::NEMESIS},
		{"surveillance", Personality::SURVEILLANCE},
		{"uninterested", Personality::UNINTERESTED},
		{"waiting", Personality::WAITING},
		{"derelict", Personality::DERELICT},
		{"fleeing", Personality::FLEEING},
		{"escort", Personality::ESCORT}
	};
	
	double DEFAULT_CONFUSION = 10. * .001;
}



// Default settings for player's ships.
Personality::Personality()
	: flags(DISABLES), confusionMultiplier(DEFAULT_CONFUSION)
{
}

Personality::Personality(enum type t)
	: flags(t), confusionMultiplier(DEFAULT_CONFUSION)
{
    if (t & DERELICT && t != DERELICT)
        flags = DERELICT;
}



void Personality::Load(const DataNode &node)
{
	flags = 0;
	for(int i = 1; i < node.Size(); ++i)
		Parse(node.Token(i));
	
	for(const DataNode &child : node)
	{
		if(child.Token(0) == "confusion" && child.Size() >= 2)
			confusionMultiplier = child.Value(1) * .001;
		else
		{
			for(int i = 0; i < child.Size(); ++i)
				Parse(child.Token(i));
		}
	}
}



void Personality::Save(DataWriter &out) const
{
	out.Write("personality");
	out.BeginChild();
	{
		out.Write("confusion", confusionMultiplier * 1000.);
		for(const auto &it : TOKEN)
			if(flags & it.second)
				out.Write(it.first);
	}
	out.EndChild();
}

void Personality::Add(unsigned f)
{
    flags |= f;
}

void Personality::Remove(unsigned f)
{
    flags &= ~f;
}


bool Personality::IsPacifist() const
{
	return flags & PACIFIST;
}


bool Personality::IsForbearing() const
{
	return flags & FORBEARING;
}



bool Personality::IsTimid() const
{
	return flags & TIMID;
}



bool Personality::Disables() const
{ return flags & DISABLES;
}



bool Personality::Plunders() const
{
	return flags & PLUNDERS;
}



bool Personality::IsHeroic() const
{
	return flags & HEROIC;
}



bool Personality::IsStaying() const
{
	return flags & STAYING;
}



bool Personality::IsEntering() const
{
	return flags & ENTERING;
}



bool Personality::IsNemesis() const
{
	return flags & NEMESIS;
}



bool Personality::IsSurveillance() const
{
	return flags & SURVEILLANCE;
}



bool Personality::IsUninterested() const
{
	return flags & UNINTERESTED;
}



bool Personality::IsWaiting() const
{
	return flags & WAITING;
}



bool Personality::IsDerelict() const
{
	return flags & DERELICT;
}



bool Personality::IsFleeing() const
{
	return flags & FLEEING;
}



bool Personality::IsEscort() const
{
	return flags & ESCORT;
}



const Point &Personality::Confusion() const
{
	confusion += Angle::Random().Unit() * confusionMultiplier;
	confusion *= .999;
	return confusion;
}



Personality Personality::Defender()
{
	static const Personality defender(static_cast<enum type>(
        Personality::STAYING | Personality::NEMESIS | Personality::HEROIC));
	return defender;
}

Personality Personality::Derelict()
{
	static const Personality derelict(static_cast<enum type>(Personality::DERELICT));
	return derelict;
}

void Personality::Parse(const string &token)
{
	auto it = TOKEN.find(token);
	if(it != TOKEN.end())
		flags |= it->second;
}
