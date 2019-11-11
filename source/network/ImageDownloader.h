#ifndef IMAGE_DOWNLOADER_H_
#define IMAGE_DOWNLOADER_H_

#include <vector>
#include <string>
#include "network/https.h"

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
		void DownloadImage(const char * url, const char * gameID, const char *fileExt, struct download *file);
		void CreateCSVLog();
		bool IsValidBanner(char *banner);

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
