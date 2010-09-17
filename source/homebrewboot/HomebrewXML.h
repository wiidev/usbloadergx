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
        //!Constructor
        //!\param path Path for the xml file
        HomebrewXML() { };
        //!Destructor
        ~HomebrewXML() { };
        //!\param filename Filepath of the XML file
        int LoadHomebrewXMLData(const char* filename);
        //! Get name
        const char * GetName() { return Name.c_str(); };
        //! Get coder
        const char * GetCoder() { return Coder.c_str(); };
        //! Get version
        const char * GetVersion() { return Version.c_str(); };
        //! Get releasedate
        const char * GetReleasedate() { return Releasedate.c_str(); };
        //! Get shortdescription
        const char * GetShortDescription() { return ShortDescription.c_str(); };
        //! Get longdescription
        const char * GetLongDescription() { return LongDescription.c_str(); };
        //! Set Name
        void SetName(char * newName) { Name = newName; };
    protected:
        std::string Name;
        std::string Coder;
        std::string Version;
        std::string Releasedate;
        std::string ShortDescription;
        std::string LongDescription;
};

#endif
