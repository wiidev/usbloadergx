/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gctypes.h>
#include "OptionList.hpp"

OptionList::OptionList()
{
}

OptionList::~OptionList()
{
	ClearList();
}

void OptionList::SetName(int i, const char *format, ...)
{
	if(i < (int) name.size())
		name[i].clear();

	if(!format)
		return;

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		if(i >= (int) name.size())
		{
			Resize(i+1);
		}

		name[i].assign(tmp);

		listChanged = true;
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

void OptionList::SetValue(int i, const char *format, ...)
{
	if(i < (int) value.size())
		value[i].clear();

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		if(i >= (int) value.size())
		{
			Resize(i+1);
		}

		value[i].assign(tmp);

		listChanged = true;
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

const char * OptionList::GetName(int i)
{
	if(i < 0 || i >= (int) name.size())
		return NULL;

	return name.at(i).c_str();
}

const char * OptionList::GetValue(int i)
{
	if(i < 0 || i >= (int) value.size())
		return NULL;

	return value.at(i).c_str();
}

void OptionList::Resize(int size)
{
	name.resize(size);
	value.resize(size);
	listChanged = true;
}

void OptionList::RemoveOption(int i)
{
	if(i < 0 || i >= (int) name.size())
		return;

	name.erase(name.begin()+i);
	value.erase(value.begin()+i);
	listChanged = true;
}

void OptionList::ClearList()
{
	name.clear();
	value.clear();
	std::vector<std::string>().swap(name);
	std::vector<std::string>().swap(value);
	listChanged = true;
}
