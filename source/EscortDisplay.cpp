/* EscortDisplay.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "EscortDisplay.h"

#include "Color.h"
#include "GameData.h"
#include "Font.h"
#include "FontSet.h"
#include "LineShader.h"
#include "RingShader.h"
#include "Point.h"
#include "OutlineShader.h"
#include "Screen.h"
#include "Ship.h"
#include "Sprite.h"
#include "System.h"

#include <algorithm>
#include <set>

using namespace std;



void EscortDisplay::Clear()
{
	icons.clear();
}



void EscortDisplay::Add(const Ship &ship, bool isHere)
{
	icons.emplace_back(ship, isHere);

	unsigned const width  = Screen::Width() / 4;
	unsigned const maxCol = max(1u, width / 70u);
	iconsPerColumn = min(12u, (Screen::Bottom() - Screen::Top()-450) / 40u);
	columns		   = max(1u, min(maxCol, static_cast<unsigned>(0.9 +
				static_cast<double>(icons.size()) /	static_cast<double>(iconsPerColumn))));
}


// The display starts in the lower left corner of the screen and takes up
// all but the top 450 pixels of the screen.
void EscortDisplay::Draw() const
{
	if (!icons.size())
		return;
	MergeStacks();
	//sort(icons.begin(), icons.end());
	
	// Draw escort status.
	double const escortWidth = Screen::Width()/(4 * columns);
	static const Font &font  = FontSet::Get(14);
	Point pos				 = Point(Screen::Left() + 20., Screen::Bottom() - 20.);
	Point  const end		 = Point(Screen::Left() + escortWidth * columns, Screen::Top() + 450);
	static const Color hereColor(.8, 1.);
	static const Color elsewhereColor(.4, .4, .6, 1.);
	static const Color readyToJumpColor(.2, .8, .2, 1.);

	static const Color shieldColor = *GameData::Colors().Get("shields");
	static const Color hullColor   = *GameData::Colors().Get("hull");
	static const Color energyColor = *GameData::Colors().Get("energy");
	static const Color heatColor   = *GameData::Colors().Get("heat");
	static const Color fuelColor   = *GameData::Colors().Get("fuel");
	static const Color shield2Color = *GameData::Colors().Get("shields2");
	static const Color hull2Color	= *GameData::Colors().Get("hull2");
	static const Color energy2Color = *GameData::Colors().Get("energy2");
	static const Color heat2Color	= *GameData::Colors().Get("heat2");
	static const Color fuel2Color	= *GameData::Colors().Get("fuel2");
	for(const Icon &escort : icons)
	{
		if(!escort.sprite)
			continue;
		

		Color color;
		if(!escort.isHere)
			color = elsewhereColor;
		else if(escort.isReadyToJump)
			color = readyToJumpColor;
		else
			color = hereColor;
		
		// Figure out what scale should be applied to the ship sprite.
		double scale = min(20. / escort.sprite->Width(), 20. / escort.sprite->Height());
		Point size(escort.sprite->Width() * scale, escort.sprite->Height() * scale);
		OutlineShader::Draw(escort.sprite, pos, size, color);
		// Draw the number of ships in this stack.
		double width = escortWidth - Icon::Height - 3.;
		if(escort.stackSize > 1)
		{
			string number = to_string(escort.stackSize);
		
			Point numberPos = pos;
			numberPos.X() += 14.; // - font.Width(number);
			numberPos.Y() += 5.; // + 0.5 * font.Height();
			font.Draw(number, numberPos, elsewhereColor);
		}

		// using high / low with these ringshaders is not tested. probably doesn't work
		if(escort.high[0] > escort.low[0])
			RingShader::Draw(pos, 19., 1.5, escort.high[0], shield2Color, 6.);
		RingShader::Draw(pos, 19., 1.5, escort.low[0], shieldColor, 6.);
		if(escort.high[1] > escort.low[1])
			RingShader::Draw(pos, 19., 1.5, escort.high[1], hull2Color, 6.);
		RingShader::Draw(pos, 15., 1.5, escort.low[1], hullColor, 6.); //Color(.33, .99, .11, 0)

		Point from(pos.X() + 19., pos.Y() - 9.);
		for(int i = 2; i < 5; ++i)
		{
			// Draw the status bars.
			static const Color barColor[] = {
				energyColor, energy2Color, 
				heatColor, heat2Color,
				fuelColor,fuel2Color
			};
			// If the low and high levels are different, draw a fully opaque bar up
			// to the low limit, and half-transparent up to the high limit.
			if(escort.high[i] > 0.)
			{
				bool isSplit = (escort.low[i] != escort.high[i]);
				const Color &color = barColor[(i-2)*2 + isSplit];
				
				Point to = from + Point(width * min(1., escort.high[i]), 0.);
				LineShader::Draw(from, to, 1.5, color);
				
				if(isSplit)
				{
					Point to = from + Point(width * max(0., escort.low[i]), 0.);
					LineShader::Draw(from, to, 1.5, color);
				}
			}

			from.Y() += 4.;
			from.X() += 2.;
			width -= 2.;
		} 
		
		// Draw the system name for any escort not in the current system.
		if(!escort.system.empty())
			font.Draw(escort.system, pos + Point(28., 5.), elsewhereColor);

		// Show only as many escorts as we have room for on screen.
		if(pos.Y() > end.Y())
			pos.Y() -= 40.; //Icon::Height;
		else if(pos.X() < end.X())
			pos.X() += escortWidth,
			pos.Y() = Screen::Bottom() - 19.5;
		else
			break;
	}
}



EscortDisplay::Icon::Icon(const Ship &ship, bool isHere)
	: sprite(ship.GetSprite().GetSprite()),
	isHere(isHere && !ship.IsDisabled()),
	isReadyToJump(ship.CheckHyperspace()),
	stackSize(1),
	cost(ship.Cost()),
	system((!isHere && ship.GetSystem()) ? ship.GetSystem()->Name() : ""),
	low{ship.Shields(), ship.Hull(), ship.Energy(), ship.Heat(), ship.Fuel()},
	high(low)
{
}



// Sorting operator. It comes sooner if it costs more.
bool EscortDisplay::Icon::operator<(const Icon &other) const
{
	return (cost > other.cost);
}

void EscortDisplay::Icon::Merge(const Icon &other)
{
	isHere &= other.isHere;
	isReadyToJump &= other.isReadyToJump;
	stackSize += other.stackSize;
	if(system.empty() && !other.system.empty())
		system = other.system;
	
	for(unsigned i = 0; i < low.size(); ++i)
	{
		low[i] = min(low[i], other.low[i]);
		high[i] = max(high[i], other.high[i]);
	}
}



void EscortDisplay::MergeStacks() const
{
	if(icons.empty())
		return;
	
	set<const Sprite *> unstackable;
	while(true)
	{
		Icon *cheapest = nullptr;
		
		for(Icon &icon : icons)
		{
			if(unstackable.find(icon.sprite) == unstackable.end() && (!cheapest || *cheapest < icon))
				cheapest = &icon;
		}
		
		if(icons.size() <= iconsPerColumn * columns || !cheapest)
			break;
		
		// Merge all other instances of this ship's sprite with this icon.
		vector<Icon>::iterator it = icons.begin();
		while(it != icons.end())
		{
			if(&*it == cheapest || it->sprite != cheapest->sprite || it->isHere != cheapest->isHere)
			{
				++it;
				continue;
			}
			
			cheapest->Merge(*it);
			it = icons.erase(it);	
		}
		unstackable.insert(cheapest->sprite);
	}
}
