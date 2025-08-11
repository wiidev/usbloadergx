#include "haxx.h"
#include "riivolution.h"
#include "riivolution_config.h"
#include "launcher.h"

#include <sys/param.h>
#include <unistd.h>
#include <malloc.h>

#include "files.h"
#include "wdvd.h"

using std::string;
using std::map;
using std::vector;

#define OPEN_MODE_BYPASS 0x80

static int fd = -1;
static DiscNode* fst = NULL;
static bool shiftfiles = false;
static u64 shift = 0;
static u32 fstsize;
map<int, bool> UsedFilesystems;

namespace Ioctl { enum Enum {
	AddFile			= 0xC1,
	AddPatch		= 0xC2,
	AddShift		= 0xC3,
	SetClusters		= 0xC4,
	Allocate		= 0xC5,
	AddEmu			= 0xC6,
	SetFileProvider	= 0xC7,
	SetShiftBase	= 0xC8,
	BanTitle		= 0xC9,
	DLC				= 0xCA
}; }

static u32 ioctlbuffer[0x08] ATTRIBUTE_ALIGN(32);

DiscNode* DiscNode::GetParent()
{
	u32 offset = this - fst;
	for (DiscNode* node = this - 1; node >= fst; node--) {
		if (node->Type && node->Size > offset)
			return node;
	}

	return NULL;
}

int RVL_Initialize()
{
	if (fd < 0)
		fd = IOS_Open("/dev/do", OPEN_MODE_BYPASS);
	shift = 0x200000000ULL; // 8 GB shift
	return fd;
}

void RVL_Close()
{
	if (fd < 0)
		return;
	IOS_Close(fd);
}

void* RVL_GetFST()
{
	return fst;
}

void RVL_SetFST(void* address, u32 size)
{
	free(fst);
	if (!size || !address) {
		fst = NULL;
		size = 0;
		return;
	}
	fst = (DiscNode*)memalign(32, size);
	if (!fst)
		return;
	fstsize = size;
	memcpy(fst, address, size);

	const void* nametable = (const void*)(fst + fst->Size);
	DiscNode* largest = NULL;
	for (DiscNode* node = fst; node < nametable; node++) {
		if (!node->Type && (!largest || largest->DataOffset < node->DataOffset))
			largest = node;
	}
	if (largest) {
		shift = (u64)largest->DataOffset << 2;
		RVL_GetShiftOffset(largest->Size); // Round up

		RVL_SetShiftBase(shift);
	}
}

u32 RVL_GetFSTSize()
{
	return fstsize;
}

int RVL_SetFileProvider(const char* filename)
{
	return IOS_Ioctl(fd, Ioctl::SetFileProvider, (void*)filename, strlen(filename + 1), NULL, 0);
}

int RVL_SetClusters(bool clusters)
{
	ioctlbuffer[0] = clusters;
	return IOS_Ioctl(fd, Ioctl::SetClusters, ioctlbuffer, 4, NULL, 0);
}

int RVL_SetShiftBase(u64 shift)
{
	ioctlbuffer[0] = shift >> 32;
	ioctlbuffer[1] = shift;
	return IOS_Ioctl(fd, Ioctl::SetShiftBase, ioctlbuffer, 8, NULL, 0);
}

void RVL_SetAlwaysShift(bool shift)
{
	shiftfiles = shift;
}

int RVL_Allocate(PatchType::Enum type, int num)
{
	ioctlbuffer[0] = type;
	ioctlbuffer[1] = num;
	return IOS_Ioctl(fd, Ioctl::Allocate, ioctlbuffer, 8, NULL, 0);
}

int RVL_AddFile(const char* filename)
{
	return IOS_Ioctl(fd, Ioctl::AddFile, (void*)filename, strlen(filename) + 1, NULL, 0);
}

int RVL_AddFile(const char* filename, u64 identifier)
{
	return IOS_Ioctl(fd, Ioctl::AddFile, (void*)filename, strlen(filename) + 1, &identifier, 8);
}

int RVL_AddShift(u64 original, u64 offset, u32 length)
{
	ioctlbuffer[0] = length;
	ioctlbuffer[1] = original >> 32;
	ioctlbuffer[2] = original;
	ioctlbuffer[3] = offset >> 32;
	ioctlbuffer[4] = offset;
	return IOS_Ioctl(fd, Ioctl::AddShift, ioctlbuffer, 0x20, NULL, 0);
}

int RVL_AddPatch(int file, u64 offset, u32 fileoffset, u32 length)
{
	ioctlbuffer[0] = file;
	ioctlbuffer[1] = fileoffset;
	ioctlbuffer[2] = offset >> 32;
	ioctlbuffer[3] = offset;
	ioctlbuffer[4] = length;
	return IOS_Ioctl(fd, Ioctl::AddPatch, ioctlbuffer, 0x20, NULL, 0);
}

int RVL_AddEmu(const char* nandpath, const char* external, int clone)
{
	ioctlv vec[3];
	vec[0].data = (void*)nandpath;
	vec[0].len = strlen(nandpath)+1;
	vec[1].data = (void*)external;
	vec[1].len = strlen(external)+1;
	vec[2].data = &clone;
	vec[2].len = sizeof(int);
	return IOS_Ioctlv(fd, Ioctl::AddEmu, 3, 0, vec);
}

int RVL_DLC(const char* path)
{
	return IOS_Ioctl(fd, Ioctl::DLC, (void*)path, strlen(path) + 1, NULL, 0);
}

u64 RVL_GetShiftOffset(u32 length)
{
	u64 ret = shift;
	shift = ROUND_UP(shift + length, 0x40);
	return ret;
}

#define REFRESH_PATH() { \
	strcpy(curpath, "/"); \
	if (nodes.size() > 1) \
		for (DiscNode** iter = nodes.begin() + 1; iter != nodes.end(); iter++) { \
			strcat(curpath, nametable + (*iter)->GetNameOffset()); \
			strcat(curpath, "/"); \
		} \
	pos = strlen(curpath); \
}

static DiscNode* RVL_FindNode(DiscNode* root, const char* name, bool recursive = false)
{
	const char* nametable = (const char*)(fst + fst->Size);
	DiscNode* node = root + 1;
	while ((void*)node < (void*)(fst + root->Size)) {
		if (!strcasecmp(nametable + node->GetNameOffset(), name))
			return node;

		if (recursive || !node->Type)
			node++;
		else
			node = fst + node->Size;
	}

	return NULL;
}

DiscNode* RVL_FindNode(const char* fstname)
{
	if (fstname[0] != '/') {
		if (!strcasecmp(fstname, "main.dol")) {
			static DiscNode maindol;
			maindol.Size = 0xFFFFFFFF;
			maindol.Type = 0;
			maindol.SetNameOffset(0);
			maindol.DataOffset = Launcher_GetFstData()[0];
			return &maindol;
		}

		return RVL_FindNode(fst, fstname, true);
	}

	char namebuffer[MAXPATHLEN];
	char* name = namebuffer;
	strcpy(name, fstname + 1);

	DiscNode* root = fst;

	while (root) {
		char* slash = strchr(name, '/');
		if (!slash)
			return RVL_FindNode(root, name);

		*slash = '\0';
		root = RVL_FindNode(root, name);
		name = slash + 1;
	}

	return NULL;
}

static void ShiftFST(DiscNode* node)
{
	node->DataOffset = (u32)(RVL_GetShiftOffset(node->Size) >> 2);
}

static void ResizeFST(DiscNode* node, u32 length, bool telldip)
{
	u64 oldoffset = (u64)node->DataOffset << 2;
	u32 oldsize = node->Size;
	node->Size = length;

	if (length > oldsize || shiftfiles)
		ShiftFST(node);

	if (telldip)
		RVL_AddShift(oldoffset, (u64)node->DataOffset << 2, oldsize);
}

static void RVL_Patch(RiiShiftPatch* shift)
{
	DiscNode* source = RVL_FindNode(shift->Source.c_str());
	DiscNode* destination = RVL_FindNode(shift->Destination.c_str());

	if (!source || !destination || source->Type || destination->Type)
		return;

	destination->DataOffset = source->DataOffset;
	destination->Size = source->Size;
}

static DiscNode* RVL_CreateFileNode(DiscNode* root, const char* name, u32 size = 0)
{
	u32 rootoffset = root - fst;
	char* nametable = (char*)(fst + fst->Size);

	// Place the new node alphabetically within the parent
	DiscNode* node = root + 1;
	for (node = root + 1; node < fst + root->Size && strcasecmp(name, nametable + node->GetNameOffset()) > 0; node = (node->Type ? (fst + node->Size) : (node + 1)))
		;
	u32 nodeoffset = node - fst;
	u32 oldfstsize = fstsize;

	// Resize fstsize down to the second-last null byte
	for (; !((u8*)fst)[fstsize - 1]; fstsize--)
		;
	fstsize++;

	u32 nametablesize = fstsize - sizeof(DiscNode) * fst->Size;

	u32 newfstsize = fstsize + sizeof(DiscNode) + strlen(name) + 1;
	if (newfstsize > oldfstsize) {
		newfstsize = ROUND_UP(newfstsize, 0x100);
		DiscNode* newfst = (DiscNode*)memalign(32, newfstsize);
		if (!newfst)
			return NULL;

		memset(newfst, 0, newfstsize);

		memcpy(newfst, fst, nodeoffset * sizeof(DiscNode));
		memcpy(newfst + nodeoffset + 1, node, fstsize - nodeoffset * sizeof(DiscNode));
		free(fst);
		fst = newfst;
		root = fst + rootoffset;
		node = fst + nodeoffset;
		fstsize = newfstsize;
	} else {
		memmove(node + 1, node, fstsize - nodeoffset * sizeof(DiscNode));
		fstsize = oldfstsize;
	}

	nametable = (char*)(fst + fst->Size + 1);

	node->Size = size;
	node->Type = 0;
	node->DataOffset = RVL_GetShiftOffset(size) >> 2;
	node->SetNameOffset(nametablesize);
	strcpy(nametable + nametablesize, name);

	for (DiscNode* parent = root; parent; parent = parent->GetParent())
		parent->Size++;

	for (DiscNode* folder = node + 1; folder < fst + fst->Size; folder++)
		if (folder->Type)
			folder->Size++;

	return node;
}

static DiscNode* RVL_CreateDirectoryNode(DiscNode* root, const char* name)
{
	DiscNode* node = RVL_CreateFileNode(root, name);
	if (node != NULL) {
		node->Type = 1;
		node->DataOffset = node->GetParent()->DataOffset + 1;
		node->Size = node - fst + 1; // Always create an empty directory
	}
	return node;
}

static DiscNode* RVL_CreateNode(string path, u32 length)
{
	if (!path.size() || path[0] != '/')
		return NULL;

	char namebuffer[MAXPATHLEN];
	char* name = namebuffer;
	strcpy(name, path.c_str() + 1);

	DiscNode* root = fst;
	while (root) {
		char* slash = strchr(name, '/');
		if (!slash) {
			if (strlen(name))
				return RVL_CreateFileNode(root, name, length);
			break;
		}

		*slash = '\0';
		DiscNode* newroot = RVL_FindNode(root, name);
		if (newroot)
			root = newroot;
		else
			root = RVL_CreateDirectoryNode(root, name);
		name = slash + 1;
	}

	return root;
}

static DiscNode ZeroNode;
static void RVL_Patch(RiiFilePatch* file, string commonfs, bool stat=false, u64 externalid=0)
{
	DiscNode* node;
	if (file->Disc.size())
		node = RVL_FindNode(file->Disc.c_str());
	else {
		ZeroNode.Type = 0;
		ZeroNode.NameOffsetMSB = 0;
		ZeroNode.NameOffset = 0;
		ZeroNode.DataOffset = 0;
		ZeroNode.Size = 0xFFFFFFFF;
		node = &ZeroNode;
	}

	if (!node) {
		if (file->Create) {
			node = RVL_CreateNode(file->Disc, file->Length);
			if (!node)
				return;
		} else
			return;
	}

	if (node->Type)
		return; // Patching a directory will not end well.

	string external = file->External;
	if (commonfs.size() && !external.compare(0, commonfs.size(), commonfs, 0, commonfs.size()))
		external = external.substr(commonfs.size());

	if (!stat && file->Length == 0) {
		Stats st;
		if (!File_Stat(external.c_str(), &st) && !(st.Mode & S_IFDIR)) {
			file->Length = st.Size - file->FileOffset;
			stat = true;
			externalid = st.Identifier;
		} else
			return;
	}

	int fd;
	if (stat)
		fd = RVL_AddFile(external.c_str(), externalid);
	else
		fd = RVL_AddFile(external.c_str());

	if (fd >= 0) {
		bool shifted = false;

		if (file->Resize) {
			if (file->Length + file->Offset > node->Size) {
				ResizeFST(node, file->Length + file->Offset, file->Offset > 0);
				shifted = true;
			} else
				node->Size = file->Length + file->Offset;
		}

		if (!shifted && shiftfiles)
			ShiftFST(node);

		RVL_AddPatch(fd, ((u64)node->DataOffset << 2) + file->Offset, file->FileOffset, file->Length);
	}
}

static void RVL_Patch(RiiFolderPatch* folder, string commonfs)
{
	string external = folder->External;
	if (commonfs.size() && !external.compare(0, commonfs.size(), commonfs, 0, commonfs.size()))
		external = external.substr(commonfs.size());

	char fdirname[MAXPATHLEN];
	int fdir = File_OpenDir(external.c_str());
	if (fdir < 0)
		return;
	Stats stats;
	while (!File_NextDir(fdir, fdirname, &stats)) {
		if (fdirname[0] == '.')
			continue;
		if (stats.Mode & S_IFDIR) {
			if (folder->Recursive){
				RiiFolderPatch newfolder = *folder;
				newfolder.Disc = PathCombine(newfolder.Disc, fdirname);
				newfolder.External = PathCombine(external, fdirname);
				RVL_Patch(&newfolder, commonfs);
			}
		} else {
			RiiFilePatch file;
			file.Create = folder->Create;
			file.Resize = folder->Resize;
			file.Offset = 0;
			file.FileOffset = 0;
			file.Length = folder->Length ? folder->Length : stats.Size;
			file.Disc = PathCombine(folder->Disc, fdirname);
			file.External = PathCombine(external, fdirname);
			RVL_Patch(&file, commonfs, true, stats.Identifier);
		}
	}
	File_CloseDir(fdir);
}

static void RVL_Patch(RiiSavegamePatch* save, string commonfs)
{
	string external = save->External;
	if (commonfs.size() && !commonfs.compare(0, commonfs.size(), external))
		external = external.substr(commonfs.size());

	char nandpath[0x40];
	snprintf(nandpath, 0x40, "/title/%08x/%08x/data", (int)(WDVD_GetTMD()->title_id >> 32), (int)WDVD_GetTMD()->title_id);
	RVL_AddEmu(nandpath, external.c_str(), save->Clone);
}

static void RVL_Patch(RiiDLCPatch* DLC, string commonfs)
{
	string external = DLC->External;
	if (commonfs.size() && !commonfs.compare(0, commonfs.size(), external))
		external = external.substr(commonfs.size());

	RVL_DLC(external.c_str());
}

static void ApplyParams(std::string* str, map<string, string>* params)
{
	string::size_type pos;
	while ((pos = str->find("{$")) != string::npos) {
		string::size_type pend = str->find("}", pos);
		string paramname = str->substr(pos + 2, pend - pos - 2);
		map<string, string>::iterator param = params->find(paramname);
		if (param == params->end())
			paramname = "";
		else
			paramname = param->second;
		*str = str->substr(0, pos) + paramname + str->substr(pend + 1);
	}
}

static void RVL_Patch(RiiPatch* patch, map<string, string>* params, string commonfs)
{
	for (vector<RiiFilePatch>::iterator file = patch->Files.begin(); file != patch->Files.end(); file++) {
		RiiFilePatch temp = *file;
		ApplyParams(&temp.External, params);
		ApplyParams(&temp.Disc, params);
		RVL_Patch(&temp, commonfs);
	}
	for (vector<RiiFolderPatch>::iterator folder = patch->Folders.begin(); folder != patch->Folders.end(); folder++) {
		RiiFolderPatch temp = *folder;
		ApplyParams(&temp.External, params);
		ApplyParams(&temp.Disc, params);
		RVL_Patch(&temp, commonfs);
	}
	for (vector<RiiShiftPatch>::iterator shift = patch->Shifts.begin(); shift != patch->Shifts.end(); shift++) {
		RVL_Patch(&*shift);
	}
	if (patch->Savegame.External.size()) {
		ApplyParams(&patch->Savegame.External, params);
		RVL_Patch(&patch->Savegame, commonfs);
	}
	if (patch->DLC.External.size()) {
		ApplyParams(&patch->DLC.External, params);
		RVL_Patch(&patch->DLC, commonfs);
	}
}

#define ADD_DEFAULT_PARAMS(params) { \
	char sng_id[9]; \
	sprintf(sng_id, "%08X", otp.ng_id); \
	params["__ngid"] = string(sng_id); \
	params["__gameid"] = string((char*)MEM_BASE, 3); \
	params["__region"] = string((char*)MEM_BASE + 3, 1); \
	params["__maker"] = string((char*)MEM_BASE + 4, 2); \
}

void RVL_Patch(RiiDisc* disc)
{
	// Search for a common filesystem so we can optimize memory
	string filesystem;

	for (vector<RiiSection>::iterator section = disc->Sections.begin(); section != disc->Sections.end(); section++) {
		for (vector<RiiOption>::iterator option = section->Options.begin(); option != section->Options.end(); option++) {
			if (!option->Default)
				continue;
			RiiChoice* choice = &option->Choices[option->Default - 1];

			for (vector<RiiChoice::Patch>::iterator patch = choice->Patches.begin(); patch != choice->Patches.end(); patch++) {
				map<string, RiiPatch>::iterator currentpatch = disc->Patches.find(patch->ID);
				if (currentpatch->second.Files.size() || currentpatch->second.Folders.size() ||
					currentpatch->second.Savegame.External.size() || currentpatch->second.DLC.External.size()) {
					// Only keep the filesystem mounted if a patch that needs to be running during the game is using it
					UsedFilesystems[choice->Filesystem] = true;
					break;
				}
			}
		}
	}

	if (UsedFilesystems.size() == 1) {
		char mountpoint[MAXPATHLEN];
		if (File_GetMountPoint(UsedFilesystems.begin()->first, mountpoint, sizeof(mountpoint)) >= 0) {
			filesystem = mountpoint;
			File_SetDefaultPath(mountpoint);
			RVL_SetClusters(true);
		}
	}

	for (vector<RiiSection>::iterator section = disc->Sections.begin(); section != disc->Sections.end(); section++) {
		for (vector<RiiOption>::iterator option = section->Options.begin(); option != section->Options.end(); option++) {
			if (option->Default == 0)
				continue;
			map<string, string> params;
			ADD_DEFAULT_PARAMS(params);
			params.insert(option->Params.begin(), option->Params.end());
			RiiChoice* choice = &option->Choices[option->Default - 1];
			params.insert(choice->Params.begin(), choice->Params.end());
			u32 end = params.size();
			for (vector<RiiChoice::Patch>::iterator patch = choice->Patches.begin(); patch != choice->Patches.end(); patch++) {
				map<string, RiiPatch>::iterator currentpatch = disc->Patches.find(patch->ID);
				if (currentpatch != disc->Patches.end()) {
					params.insert(patch->Params.begin(), patch->Params.end());

					RVL_Patch(&currentpatch->second, &params, filesystem);

					if (patch->Params.size()) {
						map<string, string>::iterator endi = params.begin();
						for (u32 i = 0; i < end; i++) // Fucking iterator needs operator+()
							endi++;
						params.erase(endi, patch->Params.end());
					}
				}
			}
		}
	}
}

void RVL_Unmount()
{
	static u32 tik_data[STD_SIGNED_TIK_SIZE/4] ATTRIBUTE_ALIGN(32);
	char _tik_path[MAXPATHLEN];
	strcpy(_tik_path + 2, "/mnt/isfs/ticket/00010005/");
	char *tik_path = _tik_path+2;
	s32 tik_dir = File_OpenDir(tik_path);
	if (tik_dir>=0) {
		Stats st;
		while (File_NextDir(tik_dir, tik_path+26, &st)>=0) {
			if (st.Mode & S_IFDIR)
				continue;
			s32 tik_file = File_Open(tik_path, O_RDONLY);
			if (tik_file>=0) {
				u32 game_id;
				memcpy(&game_id, MEM_BASE, 4);
				File_Read(tik_file, tik_data, STD_SIGNED_TIK_SIZE);
				File_Close(tik_file);
			}
		}
		File_CloseDir(tik_dir);
	}

	for (vector<int>::iterator mount = Mounted.begin(); mount != Mounted.end(); mount++) {
		bool found = false;
		for (map<int, bool>::iterator used = UsedFilesystems.begin(); used != UsedFilesystems.end(); used++) {
			if (used->first == *mount) {
				found = true;
				break;
			}
		}

		if (!found && File_GetLogFS() != *mount) {
			File_Unmount(*mount);
			for (vector<int>::iterator tomount = ToMount.begin(); tomount != ToMount.end(); tomount++) {
				if (*tomount == *mount) {
					ToMount.erase(tomount);
					break;
				}
			}
		}
	}

	if (!ToMount.size()) { // Deinit all network if we're not using it.
		static u32 buffer[0x10] ATTRIBUTE_ALIGN(32);
		s32 fd = IOS_Open("/dev/net/kd/request", 0);
		if (fd >= 0) {
			IOS_IoctlAsync(fd, 0x28, buffer, 0x20, buffer + 0x08, 0x20, NULL, NULL);
			IOS_Ioctl(fd, 0x07, NULL, 0, buffer, 0x20);
			IOS_Close(fd);
		}
	}
}

#define MEM_PHYSICAL_OR_K0(addr) ((addr) | 0x80000000)

static void* FindInBuffer(void* memory, void* end, void* pattern, int patternlength, int align)
{
	for (end = (u8*)end - patternlength; memory < end; memory = (u8*)memory + align) {
		if (!memcmp(memory, pattern, patternlength))
			return memory;
	}

	return NULL;
}
static void RVL_Patch(RiiMemoryPatch* memory, map<string, string>* params)
{
	if (memory->Ocarina || (memory->Search && !memory->Original) || !memory->Offset || !memory->GetLength())
		return;

	memory->Offset = (int)MEM_PHYSICAL_OR_K0(memory->Offset);

	if (memory->Search) {
		// TODO: Searching in MEM2? Too bad.
		void* ret = FindInBuffer((void*)memory->Offset, (void*)0x817FFFFF, memory->Original, memory->Length, memory->Align);
		if (!ret)
			return;
		memory->Offset = (int)ret;
	}

	if (memory->Original && memcmp((void*)memory->Offset, memory->Original, memory->GetLength()))
		return;

	string valuefile = memory->ValueFile;
	ApplyParams(&valuefile, params);
	void* value = memory->GetValue(valuefile);
	if (value) {
		memcpy((void*)memory->Offset, value, memory->GetLength());
		DCFlushRange((void*)memory->Offset, memory->GetLength());
		if (!memory->Value)
			free(value);
	}
}

static void RVL_Patch(RiiMemoryPatch* memory, map<string, string>* params, void* mem, u32 length)
{
	if ((!memory->Ocarina && !memory->Search) || (memory->Search && !memory->Align) || (memory->Ocarina && !memory->Offset) || !memory->GetLength())
		return;

	memory->Offset = (int)MEM_PHYSICAL_OR_K0(memory->Offset);

	string valuefile = memory->ValueFile;
	ApplyParams(&valuefile, params);
	void* value = memory->GetValue(valuefile);
	if (!value)
		return;

	if (memory->Ocarina) {
		void* ocarina = FindInBuffer(mem, (u8*)mem + length, value, memory->GetLength(), 4);
		if (ocarina) {
			u32* blr;
			for (blr = (u32*)ocarina; (u8*)blr < (u8*)mem + length && *blr != 0x4E800020; blr++)
				;
			if ((u8*)blr < (u8*)mem + length)
				*blr = ((memory->Offset - (int)blr) & 0x03FFFFFC) | 0x48000000;
		}
	} else /* if (memory->Search) */ {
		void* ret = FindInBuffer(mem, (u8*)mem + length, memory->Original, memory->Length, memory->Align);
		if (ret)
			memcpy(ret, value, memory->GetLength());
	}

	if (!memory->Value)
		free(value);
}

void RVL_PatchMemory(RiiDisc* disc, void* memory, u32 length)
{
	for (vector<RiiSection>::iterator section = disc->Sections.begin(); section != disc->Sections.end(); section++) {
		for (vector<RiiOption>::iterator option = section->Options.begin(); option != section->Options.end(); option++) {
			if (option->Default == 0)
				continue;
			map<string, string> params;
			ADD_DEFAULT_PARAMS(params);
			params.insert(option->Params.begin(), option->Params.end());
			RiiChoice* choice = &option->Choices[option->Default - 1];
			params.insert(choice->Params.begin(), choice->Params.end());
			u32 end = params.size();

			for (vector<RiiChoice::Patch>::iterator patch = choice->Patches.begin(); patch != choice->Patches.end(); patch++) {
				params.insert(patch->Params.begin(), patch->Params.end());
				RiiPatch* mem = &disc->Patches[patch->ID];
				for (vector<RiiMemoryPatch>::iterator mempatch = mem->Memory.begin(); mempatch != mem->Memory.end(); mempatch++) {
					if (memory)
						RVL_Patch(&*mempatch, &params, memory, length);
					else
						RVL_Patch(&*mempatch, &params);
				}
				if (patch->Params.size()) {
					map<string, string>::iterator endi = params.begin();
					for (u32 i = 0; i < end; i++) // Fucking iterator needs operator+()
						endi++;
					params.erase(endi, patch->Params.end());
				}
			}
		}
	}
}
