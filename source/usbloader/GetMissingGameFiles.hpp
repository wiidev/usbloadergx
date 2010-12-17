#ifndef GETMISSINGGAMEFILES_HPP_
#define GETMISSINGGAMEFILES_HPP_

/**************************************************************************************
 * FindMissingFiles
 * This function can be used for any files that are game related: .png, .wip, .gct, ...
 * Inputs:
 * path - Path to search in with. example "SD:/covers/"
 * fileext - the file extension. example ".png"
 * List - string vector where the IDs of missing game files will be put in.
 **************************************************************************************/
int GetMissingGameFiles(const char * path, const char * fileext, std::vector<std::string> & List);

#endif
