#ifndef WDMFILE_HPP_
#define WDMFILE_HPP_

#include <stdio.h>
#include <gctypes.h>
#include <string>
#include <vector>

using namespace std;

class WDMFile
{
	public:
		WDMFile(const char * filepath);
		const char * GetDolName(int pos) const { if(pos >= 0 && pos < (int) WDMEntries.size()) return WDMEntries[pos].DolName.c_str(); else return NULL; };
		const char * GetReplaceName(int pos) const { if(pos >= 0 && pos < (int) WDMEntries.size()) return WDMEntries[pos].ReplaceName.c_str(); else return NULL; };
		int GetParameter(int pos) const { if(pos >= 0 && pos < (int) WDMEntries.size()) return WDMEntries[pos].Parameter; else return 0; };
		int size() const { return WDMEntries.size(); };
	private:
		struct WDMEntry
		{
			string DolName;
			string ReplaceName;
			int Parameter;
		};

		vector<WDMEntry> WDMEntries;
};

#endif
