#ifndef _FILESYS_INT_H
#define _FILESYS_INT_H

#include "kwrap/type.h"

#define FS_SXCMD_ENABLE     0   //default: 0 (disable), 1 (for debug)

void xFileSys_InstallCmd(void);

typedef INT32 FILESYS_OP_OPEN(CHAR Drive, FS_HANDLE TarStrgDXH, FILE_TSK_INIT_PARAM *pInitParam);
typedef INT32 FILESYS_OP_CLOSE(CHAR Drive, UINT32 TimeOut);
typedef INT32 FILESYS_OP_GETSTRGOBJ(CHAR Drive, FS_HANDLE *pStrgDXH);
typedef INT32 FILESYS_OP_CHANGEDISK(CHAR Drive, FS_HANDLE StrgDXH);
typedef UINT32 FILESYS_OP_GETPARAM(CHAR Drive, FST_PARM_ID param_id, UINT32 parm1);
typedef INT32 FILESYS_OP_FORMATANDLABEL(CHAR Drive, FS_HANDLE StrgDXH, BOOL ChgDisk, CHAR *pLabelName);
typedef INT32 FILESYS_OP_GETLABEL(CHAR Drive, CHAR *pLabel);
typedef UINT64 FILESYS_OP_GETDISKINFO(CHAR Drive, FST_INFO_ID info_id);
typedef INT32 FILESYS_OP_GETLONGNAME(CHAR *pPath, UINT16 *wFileName);
typedef ER FILESYS_OP_SETPARAM(CHAR Drive, FST_PARM_ID param_id, UINT32 value);
typedef INT32 FILESYS_OP_WAITFINISH(CHAR Drive);
typedef INT32 FILESYS_OP_MAKEDIR(CHAR *pPath);
typedef INT32 FILESYS_OP_SCANDIR(CHAR *pPath, FileSys_ScanDirCB DirCB, BOOL bGetlong, UINT32 CBArg);
typedef INT32 FILESYS_OP_DELDIRFILES(CHAR *pPath, FileSys_DelDirCB CB);
typedef INT32 FILESYS_OP_LOCKDIRFILES(CHAR *pPath, BOOL isLock, FileSys_LockDirCB CB);
typedef INT32 FILESYS_OP_GETATTRIB(CHAR *pPath, UINT8 *pAttrib);
typedef INT32 FILESYS_OP_SETATTRIB(CHAR *pPath, UINT8 Attrib, BOOL bSet);
typedef INT32 FILESYS_OP_GETDATETIME(CHAR *pPath, UINT32 creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32 modDateTime[FST_FILE_DATETIME_MAX_ID]);
typedef INT32 FILESYS_OP_SETDATETIME(CHAR *pPath, UINT32 creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32 modDateTime[FST_FILE_DATETIME_MAX_ID]);
typedef INT32 FILESYS_OP_DELETEFILE(CHAR *pPath);
typedef INT32 FILESYS_OP_DELETEDIR(CHAR *pPath);
typedef INT32 FILESYS_OP_RENAMEFILE(CHAR *pNewname, CHAR *pPath, BOOL bIsOverwrite);
typedef INT32 FILESYS_OP_RENAMEDIR(CHAR *pNewname, CHAR *pPath, BOOL bIsOverwrite);
typedef INT32 FILESYS_OP_MOVEFILE(CHAR *pSrcPath, CHAR *pDstPath);
typedef UINT64 FILESYS_OP_GETFILELEN(CHAR *pPath);
typedef VOID FILESYS_OP_REGISTERCB(CHAR Drive, FileSys_CB CB);
typedef FST_FILE FILESYS_OP_OPENFILE(CHAR *pPath, UINT32 Flag);
typedef INT32 FILESYS_OP_CLOSEFILE(FST_FILE pFile);
typedef INT32 FILESYS_OP_READFILE(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
typedef INT32 FILESYS_OP_WRITEFILE(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
typedef INT32 FILESYS_OP_SEEKFILE(FST_FILE pFile, UINT64 offset, FST_SEEK_CMD fromwhere);
typedef UINT64 FILESYS_OP_TELLFILE(FST_FILE pFile);
typedef INT32 FILESYS_OP_FLUSHFILE(FST_FILE pFile);
typedef INT32 FILESYS_OP_STATFILE(FST_FILE pFile, FST_FILE_STATUS *pFileStat);
typedef INT32 FILESYS_OP_TRUNCFILE(FST_FILE pFile, UINT64 NewSize);
typedef INT32 FILESYS_OP_ALLOCFILE(FST_FILE pFile, UINT64 NewSize, UINT32 Reserved1, UINT32 Reserved2);
typedef INT32 FILESYS_OP_COPYTOBYNAME(COPYTO_BYNAME_INFO *pCopyInfo);
typedef INT32 FILESYS_OP_BENCHMARK(CHAR Drive, FS_HANDLE StrgDXH, UINT8 *pBuf, UINT32 BufSize);
typedef VOID FILESYS_OP_INSTALLID(VOID);

typedef CHAR *FILESYS_OP_GETCWD(VOID);
typedef VOID FILESYS_OP_SETCWD(const CHAR *pPath);
typedef int FILESYS_OP_GETPARENTDIR(const CHAR *pPath, CHAR *parentDir);
typedef int FILESYS_OP_CHDIR(CHAR *pPath);

typedef FS_SEARCH_HDL FILESYS_OP_SEARCHFILEOPEN(CHAR *pPath, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
typedef int FILESYS_OP_SEARCHFILE(FS_SEARCH_HDL pSearch, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
typedef int FILESYS_OP_SEARCHFILECLOSE(FS_SEARCH_HDL pSearch);
typedef int FILESYS_OP_SEARCHFILEREWIND(FS_SEARCH_HDL pSearch);

typedef INT32 FILESYS_OP_DELETEMULTIFILES(PFST_MULTI_FILES pMultiFiles);

typedef struct _FILESYS_OPS {
	FST_FS_TYPE                 FsType;
	FILESYS_OP_OPEN             *Open;
	FILESYS_OP_CLOSE            *Close;
	FILESYS_OP_GETSTRGOBJ       *GetStrgObj;
	FILESYS_OP_CHANGEDISK       *ChangeDisk;
	FILESYS_OP_GETPARAM         *GetParam;
	FILESYS_OP_SETPARAM         *SetParam;
	FILESYS_OP_FORMATANDLABEL   *FormatAndLabel;
	FILESYS_OP_GETDISKINFO      *GetDiskInfo;
	FILESYS_OP_GETLONGNAME      *GetLongName;
	FILESYS_OP_WAITFINISH       *WaitFinish;
	FILESYS_OP_MAKEDIR          *MakeDir;
	FILESYS_OP_SCANDIR          *ScanDir;
	FILESYS_OP_DELDIRFILES      *DelDirFiles;
	FILESYS_OP_LOCKDIRFILES     *LockDirFiles;
	FILESYS_OP_GETATTRIB        *GetAttrib;
	FILESYS_OP_SETATTRIB        *SetAttrib;
	FILESYS_OP_GETDATETIME      *GetDateTime;
	FILESYS_OP_SETDATETIME      *SetDateTime;
	FILESYS_OP_DELETEFILE       *DeleteFile;
	FILESYS_OP_DELETEDIR        *DeleteDir;
	FILESYS_OP_RENAMEFILE       *RenameFile;
	FILESYS_OP_RENAMEDIR        *RenameDir;
	FILESYS_OP_MOVEFILE         *MoveFile;
	FILESYS_OP_GETFILELEN       *GetFileLen;
	FILESYS_OP_REGISTERCB       *RegisterCB;
	FILESYS_OP_OPENFILE         *OpenFile;
	FILESYS_OP_CLOSEFILE        *CloseFile;
	FILESYS_OP_READFILE         *ReadFile;
	FILESYS_OP_WRITEFILE        *WriteFile;
	FILESYS_OP_SEEKFILE         *SeekFile;
	FILESYS_OP_TELLFILE         *TellFile;
	FILESYS_OP_FLUSHFILE        *FlushFile;
	FILESYS_OP_STATFILE         *StatFile;
	FILESYS_OP_TRUNCFILE        *TruncFile;
	FILESYS_OP_COPYTOBYNAME     *CopyToByName;
	FILESYS_OP_BENCHMARK        *Benchmark;
	FILESYS_OP_INSTALLID        *InstallID;

	FILESYS_OP_GETCWD           *GetCwd;
	FILESYS_OP_SETCWD           *SetCwd;
	FILESYS_OP_GETPARENTDIR     *GetParentDir;
	FILESYS_OP_CHDIR            *Chdir;

	FILESYS_OP_SEARCHFILEOPEN   *SearchFileOpen;
	FILESYS_OP_SEARCHFILE       *SearchFile;
	FILESYS_OP_SEARCHFILECLOSE  *SearchFileClose;
	FILESYS_OP_SEARCHFILEREWIND *SearchFileRewind;

	FILESYS_OP_ALLOCFILE        *AllocFile;
	FILESYS_OP_GETLABEL         *GetLabel;
	FILESYS_OP_DELETEMULTIFILES *DeleteMultiFiles;
} FILESYS_OPS;

#endif
