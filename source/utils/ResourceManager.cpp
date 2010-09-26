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
	std::map<const u8 *, GuiImageData *>::iterator imgitr;
	for (imgitr = images.begin(); imgitr != images.end(); imgitr++)
	{
		delete imgitr->second;
	}
	images.clear();
	imageCount.clear();
}

bool ResourceManager::Exists(const u8 *img, u32 imgSize)
{
	return ResourceManager::Instance()->InternalExists(img, imgSize);
}

bool ResourceManager::InternalExists(const u8 *img, u32 imgSize)
{
	std::map<const u8 *, GuiImageData *>::iterator itr = images.find(img);

	return (itr != images.end());
}

GuiImageData * ResourceManager::GetImageData(const u8 *img, u32 imgSize)
{
	return ResourceManager::Instance()->InternalGetImageData(img, imgSize);
}

void ResourceManager::Remove(GuiImageData *img)
{
	if(!img)
		return;

	ResourceManager::Instance()->InternalRemoveImageData(img->GetImage());
}

void ResourceManager::Remove(u8 *img)
{
	ResourceManager::Instance()->InternalRemoveImageData(img);
}

GuiImageData * ResourceManager::InternalGetImageData(const u8 *img, u32 imgSize)
{
	std::map<const u8 *, GuiImageData *>::iterator itr = images.find(img);
	if (itr == images.end())
	{
		// Not found, create a new one
		GuiImageData *d = new GuiImageData(img, imgSize);
		images[img] = d;
		imageCount[d->GetImage()] = 1;
		return d;

	}
	imageCount[itr->second->GetImage()]++;
	return itr->second;
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

			std::map<const u8 *, GuiImageData *>::iterator iitr;
			for (iitr = images.begin(); iitr != images.end(); iitr++)
			{
				if (iitr->second->GetImage() == img)
				{
					delete iitr->second;
					images.erase(iitr);
					break;
				}
			}
		}
	}
}

void ResourceManager::InternalRemoveImageData(GuiImageData * img)
{
	std::map<u8 *, int>::iterator itr = imageCount.find(img->GetImage());
	if (itr != imageCount.end())
	{
		itr->second--;

		if (itr->second == 0) // Remove the resource
		{
			imageCount.erase(itr);

			std::map<const u8 *, GuiImageData *>::iterator iitr;
			for (iitr = images.begin(); iitr != images.end(); iitr++)
			{
				if (iitr->second == img)
				{
					delete iitr->second;
					images.erase(iitr);
					break;
				}
			}
		}
	}
}

