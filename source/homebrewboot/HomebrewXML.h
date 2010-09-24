/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#ifndef ___HOMEBREWXML_H_
#define ___HOMEBREWXML_H_

#include <string>

class HomebrewXML
{
    public:
        HomebrewXML();
        ~HomebrewXML();

        int LoadHomebrewXMLData(const char* filename);

        const char * GetName();
        void SetName(char * newName);
        const char * GetCoder();
        const char * GetVersion();
        const char * GetReleasedate();
        const char * GetShortDescription();
        const char * GetLongDescription();

    protected:
        std::string Name;
        std::string Coder;
        std::string Version;
        std::string Releasedate;
        std::string ShortDescription;
        std::string LongDescription;
};

#endif
