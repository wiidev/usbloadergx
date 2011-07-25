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
 ***************************************************************************/
#include "ResourceManager.h"

ResourceManager * ResourceManager::instance = NULL;

ResourceManager * ResourceManager::Instance()
{
	if (instance == NULL)
	{
		instance = new ResourceManager();
	}
	return instance;
}

void ResourceManager::DestroyInstance()
{
	if (instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
}

ResourceManager::~ResourceManager()
{
	// Delete all images...
	std::map<const u8 *, ImageData>::iterator imgitr;
	for (imgitr = images.begin(); imgitr != images.end(); imgitr++)
	{
		if(imgitr->second.data)
			free(imgitr->second.data);
	}
	images.clear();
	imageCount.clear();
}

void ResourceManager::AddImageData(const u8 *img, ImageData & Data)
{
	ResourceManager::Instance()->InternalAddImageData(img, Data);
}

ImageData * ResourceManager::GetImageData(const u8 *img)
{
	return ResourceManager::Instance()->InternalGetImageData(img);
}

void ResourceManager::Remove(u8 * img)
{
	ResourceManager::Instance()->InternalRemoveImageData(img);
}

void ResourceManager::InternalAddImageData(const u8 * img, ImageData & Data)
{
	std::map<const u8 *, ImageData>::iterator itr = images.find(img);
	if (itr != images.end())
		return;

	images[img] = Data;
	imageCount[Data.data] = 1;
}

ImageData * ResourceManager::InternalGetImageData(const u8 *img)
{
	std::map<const u8 *, ImageData>::iterator itr = images.find(img);
	if (itr == images.end())
		return NULL;

	imageCount[itr->second.data]++;

	return &itr->second;
}

void ResourceManager::InternalRemoveImageData(u8 * img)
{
	std::map<u8 *, int>::iterator itr = imageCount.find(img);
	if (itr != imageCount.end())
	{
		itr->second--;

		if (itr->second == 0) // Remove the resource
		{
			imageCount.erase(itr);

			std::map<const u8 *, ImageData>::iterator iitr;
			for (iitr = images.begin(); iitr != images.end(); iitr++)
			{
				if (iitr->second.data == img)
				{
					if(iitr->second.data)
						free(iitr->second.data);
					images.erase(iitr);
					break;
				}
			}
		}
	}
	else if(img)
	{
		//! This case should actually never accur
		free(img);
	}
}
