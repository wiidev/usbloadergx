#ifndef _NEWTITLES_H
#define _NEWTITLES_H

#include <time.h>

class NewTitles
{
	public:
		static NewTitles *Instance() { if(!instance) instance = new NewTitles(); return instance; }
		static void DestroyInstance() { delete instance; instance = NULL; }

		void Save();
		void CheckGame(const u8 *titleid);
		bool IsNew(const u8 *titleid) const;
		void Remove(const u8 *titleid);
	private:
		NewTitles();
		~NewTitles();

		static NewTitles *instance;

		class Title
		{
			public:
				char titleId[7];
				time_t timestamp;
				Title *next;
				bool isNew;
		};

		Title *firstTitle;
		Title *lastTitle;
		bool isDirty;
		bool isNewFile;
};

#endif //_NEWTITLES_H
