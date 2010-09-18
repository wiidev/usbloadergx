#ifndef _NEWTITLES_H
#define _NEWTITLES_H

#include <time.h>

class NewTitles
{
    public:
        static NewTitles *Instance();
        static void DestroyInstance();

        void Save();
        void CheckGame( u8 *titleid );
        bool IsNew( u8 *titleid );
        void Remove( u8 *titleid );
    private:
        NewTitles();
        ~NewTitles();

        static NewTitles *instance;

        class Title
        {
            public:
                u8 titleId[6];
                time_t timestamp;
                void *next;
        };

        Title *firstTitle;
        Title *lastTitle;
        bool isDirty;
        bool isNewFile;
};

#endif //_NEWTITLES_H
