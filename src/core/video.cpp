// video.cpp
// Spontz Demogroup

#include <main.h>
#include <io.h>
#include "video.h"

Video::Video()
{
	textureNum.clear();
}

Video::~Video()
{
	textureNum.clear();	
}

bool Video::load(const std::string & folderPath, bool flip)
{
	string fullpath = folderPath + "/*.*";


	struct _finddata_t FindData;
	intptr_t hFile;
	int i = 0;
	// get number of textures to process
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			if (!(FindData.attrib & _A_SUBDIR))
				i++;
		} while (_findnext(hFile, &FindData) == 0);
	}
	if (i == 0) {
		LOG->Error("Error loading video: No files have been found in: %s", fullpath.c_str());
		return false;
	}
		

	this->totalFps = i;
	
	// Load each image as a texture in the texture Manager
	i = 0;
	string filename;
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			if (!(FindData.attrib & _A_SUBDIR)) { // If it's not a subfolder... we process it
				filename = folderPath + "/" + FindData.name;
				// Load a texture
				int texNumber =  DEMO->textureManager.addTexture(filename, flip, "texture_video");
				if (texNumber == -1) {
					LOG->Error("Error loading video-image in: %s", filename.c_str());
					return false;
				}	
				else
					this->textureNum.push_back(texNumber);
			}
		} while (_findnext(hFile, &FindData) == 0);
	}
	
	return true;
}

void Video::bind(int texUnit) const
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	//TODO: Calculate the textureID to draw, based in the seconds
	//glBindTexture(GL_TEXTURE_2D, textureID);
}