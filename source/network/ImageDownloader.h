#ifndef IMAGE_DOWNLOADER_H_
#define IMAGE_DOWNLOADER_H_

#include <vector>
#include <string>
#include "network/http.h"

class ImageDownloader
{
	public:
		static void DownloadImages();
	private:
		void Start();
		void SetChoices(int c) { choices = c; }
		void FindMissingImages();
		void FindMissing(const char *writepath, const char *downloadURL, const char *backupURL, const char *progressTitle, const char *backupProgressTitle, const char *fileExt);
		int DownloadProcess(int TotalDownloadCount);
		struct block DownloadImage(const char * url, const char * gameID, const char *fileExt);
		void CreateCSVLog();
		bool IsValidBanner(unsigned char *banner);

		struct ImageLink
		{
			std::string gameID;
			const char *downloadURL;
			const char *backupURL;
			const char *writepath;
			const char *progressTitle;
			const char *backupProgressTitle;
			const char *fileExt;
		};
		int choices;
		u32 MissingImagesCount;
		std::vector<ImageLink> MissingImages;
};

#endif
