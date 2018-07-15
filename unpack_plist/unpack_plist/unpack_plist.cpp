#include <iostream>
#include "../tinyxml2/tinyxml2.h"
#include <string>
#include "../cximage/ximage.h"
#include <direct.h>


tinyxml2::XMLElement* getValueNode(tinyxml2::XMLElement* infonode,const char* key)
{
	tinyxml2::XMLElement* node = infonode->FirstChildElement("key");
	while (node)
	{
		if (!strcmp(node->GetText(), key))
		{
			return node->NextSiblingElement();
		}

		node = node->NextSiblingElement("key");
	}
	return NULL;
}
int main(int agrc,char** argv)
{
	if (agrc < 2 )
	{
		std::cout << "arg error";
		return 1;
	}

	const char* filename = argv[1];

	std::string plistName = filename;
	plistName += ".plist";

	tinyxml2::XMLDocument plistDoc;
	if (tinyxml2::XML_SUCCESS!= plistDoc.LoadFile(plistName.c_str()))
	{
		std::cout << "plist file open fail.";
		return 1;
	}

	std::string imageName = argv[1];
	imageName += ".png";

	CxImage img;
	if (!img.Load(imageName.c_str()))
	{
		std::cout << "png file open fail.";
		return 1;
	}


	tinyxml2::XMLElement* node = plistDoc.FirstChildElement();
	if (!node || strcmp(node->Name(), "plist") )
	{
		std::cout << "plist error 1.";
		return 1;
	}

	node = node->FirstChildElement();
	if (!node || strcmp(node->Name(), "dict"))
	{
		std::cout << "plist error 2.";
		return 1;
	}

	node = node->FirstChildElement("key");
	if (!node || strcmp(node->GetText(),"frames"))
	{
		std::cout << "plist error 3.";
		return 1;
	}
	node = node->NextSiblingElement();
	if (!node || strcmp(node->Name(), "dict"))
	{
		std::cout << "plist error 4.";
		return 1;
	}


	_mkdir(filename);

	tinyxml2::XMLElement* namenode = node->FirstChildElement();
	while (namenode && !strcmp(namenode->Name(), "key"))
	{
		tinyxml2::XMLElement* infonode = namenode->NextSiblingElement();
		if (!infonode || strcmp(infonode->Name(), "dict") )
		{
			std::cout << "plist error 5£º" << namenode->GetText();
			return 1;
		}

		int posx, posy, width, height;
		bool rotated;

		tinyxml2::XMLElement* node = getValueNode(infonode,"frame");
		if (!node || strcmp(node->Name(), "string"))
		{
			std::cout << "plist error 6£º" << namenode->GetText();
			return 1;
		}

		if (4!=sscanf(node->GetText(), "{{%d,%d},{%d,%d}}", &posx, &posy, &width, &height))
		{
			std::cout << "plist error 7£º" << namenode->GetText();
			return 1;
		}

		node = getValueNode(infonode, "rotated");
		if (!node)
		{
			std::cout << "plist error 8£º" << namenode->GetText();
			return 1;
		}

		if (!strcmp(node->Name(), "false"))
		{
			rotated = false;
		}
		else if(!strcmp(node->Name(), "true"))
		{
			rotated = true;
		}
		else
		{
			std::cout << "plist error 9£º" << namenode->GetText();
			return 1;
		}


		int offsetx, offsety;
		node = getValueNode(infonode, "offset");
		if (!node || strcmp(node->Name(), "string"))
		{
			std::cout << "plist error 10£º" << namenode->GetText();
			return 1;
		}
		if (2 != sscanf(node->GetText(), "{%d,%d}", &offsetx, &offsety))
		{
			std::cout << "plist error 11£º" << namenode->GetText();
			return 1;
		}
		if (offsetx || offsety)
		{
			std::cout << "plist error 12£º" << namenode->GetText();
			return 1;
		}

		int srccx, srccy, srccw, srcch;
		node = getValueNode(infonode, "sourceColorRect");
		if (!node || strcmp(node->Name(), "string"))
		{
			std::cout << "plist error 13£º" << namenode->GetText();
			return 1;
		}
		if (4 != sscanf(node->GetText(), "{{%d,%d},{%d,%d}}", &srccx, &srccy, &srccw, &srcch))
		{
			std::cout << "plist error 14£º" << namenode->GetText();
			return 1;
		}
		if (srccx || srccy || srccw!=width || srcch != height)
		{
			std::cout << "plist error 15£º" << namenode->GetText();
			return 1;
		}


		int srcw, srch;
		node = getValueNode(infonode, "sourceSize");
		if (!node || strcmp(node->Name(), "string"))
		{
			std::cout << "plist error 16£º" << namenode->GetText();
			return 1;
		}
		if (2 != sscanf(node->GetText(), "{%d,%d}", &srcw, &srch))
		{
			std::cout << "plist error 17£º" << namenode->GetText();
			return 1;
		}
		if (srcw!=width || srch !=height)
		{
			std::cout << "plist error 18£º" << namenode->GetText();
			return 1;
		}
		CxImage dst;
		if (!img.Crop(posx, posy, rotated ? posx + height : posx + width, rotated ? posy + width : posy + height, &dst))
		{
			std::cout << "crop error" << namenode->GetText();
			return 1;
		}
		if (rotated)
		{
			dst.Rotate(-90);
		}

		const char* ext = strrchr(namenode->GetText(), '.');
		if (!ext)
		{
			std::cout << "crop type error" << namenode->GetText();
			return 1;
		}
		uint32_t  imgtype = CxImage::GetTypeIdFromName(++ext);

		std::string	 _filename = filename;
		_filename += "/";
		_filename += namenode->GetText();
		dst.Save(_filename.c_str(), imgtype);

		namenode = infonode->NextSiblingElement();
	}


    return 0;
}

