/****************************************************************************
 * HomebrewXML Class
 * for USB Loader GX
 ***************************************************************************/
#ifndef ___HOMEBREWXML_H_
#define ___HOMEBREWXML_H_

class HomebrewXML
{
    public:
        //!Constructor
        //!\param path Path for the xml file
        HomebrewXML();
        //!Destructor
        ~HomebrewXML();
        //!\param filename Filepath of the XML file
        int LoadHomebrewXMLData(const char* filename);
        //! Get name
        char * GetName() { return name; }
        //! Get coder
        char * GetCoder() { return coder; }
        //! Get version
        char * GetVersion() { return version; }
        //! Get releasedate
        char * GetReleasedate() { return releasedate; }
        //! Get shortdescription
        char * GetShortDescription() { return shortdescription; }
        //! Get longdescription
        char * GetLongDescription() { return longdescription; }
        //! Set Name
        void SetName(char * path) { strncpy(name, path, sizeof(name)); }
    protected:
        char name[50];
        char coder[100];
        char version[30];
        char releasedate[30];
        char shortdescription[150];
        char longdescription[500];
};

#endif
