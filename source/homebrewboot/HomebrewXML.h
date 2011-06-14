/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#ifndef ___HOMEBREWXML_H_
#define ___HOMEBREWXML_H_

#include <string>
#include <vector>

class HomebrewXML
{
    public:
        HomebrewXML() { };
        HomebrewXML(const char* filename) { LoadHomebrewXMLData(filename); };

        int LoadHomebrewXMLData(const char* filename);

        const char * GetName() const;
        void SetName(char * newName);
        const char * GetCoder() const;
        const char * GetVersion() const;
        const char * GetReleasedate() const;
        const char * GetShortDescription() const;
        const char * GetLongDescription() const;
        const std::vector<std::string> & GetArguments() const { return Arguments; };

    protected:
        std::string Name;
        std::string Coder;
        std::string Version;
        std::string Releasedate;
        std::string ShortDescription;
        std::string LongDescription;
        std::vector<std::string> Arguments;
};

#endif
