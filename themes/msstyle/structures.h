#ifndef _structures_h_
#define _structures_h_


/* 
 * PE file
 * Most of this is taken from win32api for mingw which is not copyrighted
 */

/* file header */
typedef struct _IMAGE_FILE_HEADER {
	uint16_t Machine;
	uint16_t NumberOfSections;
	uint32_t TimeDateStamp;
	uint32_t PointerToSymbolTable;
	uint32_t NumberOfSymbols;
	uint16_t SizeOfOptionalHeader;
	uint16_t Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER; 

/* optional header */
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16 

typedef struct _IMAGE_DATA_DIRECTORY {
	uint32_t VirtualAddress;
	uint32_t Size;
} IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY; 

typedef struct _IMAGE_OPTIONAL_HEADER {
	uint16_t Magic;
	unsigned char MajorLinkerVersion;
	unsigned char MinorLinkerVersion;
	uint32_t SizeOfCode;
	uint32_t SizeOfInitializedData;
	uint32_t SizeOfUninitializedData;
	uint32_t AddressOfEntryPoint;
	uint32_t BaseOfCode;
	uint32_t BaseOfData;
	uint32_t ImageBase;
	uint32_t SectionAlignment;
	uint32_t FileAlignment;
	uint16_t MajorOperatingSystemVersion;
	uint16_t MinorOperatingSystemVersion;
	uint16_t MajorImageVersion;
	uint16_t MinorImageVersion;
	uint16_t MajorSubsystemVersion;
	uint16_t MinorSubsystemVersion;
	uint32_t Reserved1;
	uint32_t SizeOfImage;
	uint32_t SizeOfHeaders;
	uint32_t CheckSum;
	uint16_t Subsystem;
	uint16_t DllCharacteristics;
	uint32_t SizeOfStackReserve;
	uint32_t SizeOfStackCommit;
	uint32_t SizeOfHeapReserve;
	uint32_t SizeOfHeapCommit;
	uint32_t LoaderFlags;
	uint32_t NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER,*PIMAGE_OPTIONAL_HEADER; 

/* DOS header */
#define IMAGE_DOS_SIGNATURE 0x5A4D
typedef struct _IMAGE_DOS_HEADER {
	uint16_t e_magic;
	uint16_t e_cblp;
	uint16_t e_cp;
	uint16_t e_crlc;
	uint16_t e_cparhdr;
	uint16_t e_minalloc;
	uint16_t e_maxalloc;
	uint16_t e_ss;
	uint16_t e_sp;
	uint16_t e_csum;
	uint16_t e_ip;
	uint16_t e_cs;
	uint16_t e_lfarlc;
	uint16_t e_ovno;
	uint16_t e_res[4];
	uint16_t e_oemid;
	uint16_t e_oeminfo;
	uint16_t e_res2[10];
	int32_t e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER; 

/* NT headers */
#define IMAGE_NT_SIGNATURE 0x00004550 
typedef struct _IMAGE_NT_HEADERS {
	uint32_t Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS,*PIMAGE_NT_HEADERS; 


/* section header */
#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
	unsigned char Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		uint32_t PhysicalAddress;
		uint32_t VirtualSize;
	} Misc;
	uint32_t VirtualAddress;
	uint32_t SizeOfRawData;
	uint32_t PointerToRawData;
	uint32_t PointerToRelocations;
	uint32_t PointerToLinenumbers;
	uint16_t NumberOfRelocations;
	uint16_t NumberOfLinenumbers;
	uint32_t Characteristics;
} IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER; 

/* machine codes (only those we really need) */
#define IMAGE_FILE_MACHINE_UNKNOWN	0
#define IMAGE_FILE_MACHINE_I386	332 
#define IMAGE_FILE_MACHINE_AMD64 0x8664 

/* ... */
#define IMAGE_SIZEOF_FILE_HEADER	20
#define IMAGE_FILE_RELOCS_STRIPPED	1
#define IMAGE_FILE_EXECUTABLE_IMAGE	2
#define IMAGE_FILE_LINE_NUMS_STRIPPED	4
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED	8
#define IMAGE_FILE_AGGRESIVE_WS_TRIM 	16
#define IMAGE_FILE_LARGE_ADDRESS_AWARE	32
#define IMAGE_FILE_BYTES_REVERSED_LO	128
#define IMAGE_FILE_32BIT_MACHINE	256
#define IMAGE_FILE_DEBUG_STRIPPED	512
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP	1024
#define IMAGE_FILE_NET_RUN_FROM_SWAP	2048
#define IMAGE_FILE_SYSTEM	4096
#define IMAGE_FILE_DLL	8192
#define IMAGE_FILE_UP_SYSTEM_ONLY	16384
#define IMAGE_FILE_BYTES_REVERSED_HI	32768 


/* image directory */
#define FIELD_OFFSET(t,f) ((long)&(((t*)0)->f))
#define IMAGE_FIRST_SECTION(h) ((PIMAGE_SECTION_HEADER) ((long)h+FIELD_OFFSET(IMAGE_NT_HEADERS,OptionalHeader)+((PIMAGE_NT_HEADERS)(h))->FileHeader.SizeOfOptionalHeader)) 
#define IMAGE_DIRECTORY_ENTRY_EXPORT	0
#define IMAGE_DIRECTORY_ENTRY_IMPORT	1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE	2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION	3
#define IMAGE_DIRECTORY_ENTRY_SECURITY	4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC	5
#define IMAGE_DIRECTORY_ENTRY_DEBUG	6
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT	7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR	8
#define IMAGE_DIRECTORY_ENTRY_TLS	9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11
#define IMAGE_DIRECTORY_ENTRY_IAT	12 
#define IMAGE_RESOURCE_NAME_IS_STRING 0x80000000 

/* image resource directory */
#define _ANONYMOUS_UNION __extension__
#define _ANONYMOUS_STRUCT 

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            uint32_t NameOffset:31;
            uint32_t NameIsString:1;
        };
        uint32_t   Name;
        uint16_t    Id;
    };
    union {
        uint32_t   OffsetToData;
        struct {
            uint32_t   OffsetToDirectory:31;
            uint32_t   DataIsDirectory:1;
        };
    };
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;
 

typedef struct _IMAGE_RESOURCE_DIRECTORY {
	uint32_t Characteristics;
	uint32_t TimeDateStamp;
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint16_t NumberOfNamedEntries;
	uint16_t NumberOfIdEntries;
	IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[1]; /* not sure how safe is this */
} IMAGE_RESOURCE_DIRECTORY,*PIMAGE_RESOURCE_DIRECTORY; 

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
	uint32_t OffsetToData;
	uint32_t Size;
	uint32_t CodePage;
	uint32_t Reserved;
} IMAGE_RESOURCE_DATA_ENTRY,*PIMAGE_RESOURCE_DATA_ENTRY; 

typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
	uint16_t Length;
	char NameString[1];
} IMAGE_RESOURCE_DIR_STRING_U,*PIMAGE_RESOURCE_DIR_STRING_U; 

/* PE file end */


/* resource types */
#define RT_CURSOR 1
#define RT_BITMAP 2
#define RT_ICON 3
#define RT_MENU 4
#define RT_DIALOG 5
#define RT_STRING 6
#define RT_FONTDIR 7
#define RT_FONT 8
#define RT_ACCELERATOR 9
#define RT_RCDATA 10
#define RT_MESSAGETABLE 11
#define RT_GROUP_CURSOR 12
#define RT_GROUP_ICON 14
#define RT_VERSION 16
#define RT_DLGINCLUDE 17
#define RT_PLUGPLAY 19
#define RT_VXD 20
#define RT_ANICURSOR 21
#define RT_ANIICON 22
#define RT_HTML 23
#define RT_MANIFEST 24

/* bitmap */
typedef struct tagBITMAPINFOHEADER{
	uint32_t   biSize;
	int32_t    biWidth;
	int32_t    biHeight;
	uint16_t   biPlanes;
	uint16_t   biBitCount;
	uint32_t   biCompression;
	uint32_t   biSizeImage;
	int32_t    biXPelsPerMeter;
	int32_t    biYPelsPerMeter;
	uint32_t   biClrUsed;
	uint32_t   biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPCOREHEADER {
	uint32_t	bcSize;
	uint16_t	bcWidth;
	uint16_t	bcHeight;
	uint16_t	bcPlanes;
	uint16_t	bcBitCount;
} BITMAPCOREHEADER,*LPBITMAPCOREHEADER,*PBITMAPCOREHEADER; 

#define BFT_BITMAP 0x4d42   /* 'BM' */

typedef struct tagRGBQUAD {
	unsigned char	rgbBlue;
	unsigned char	rgbGreen;
	unsigned char	rgbRed;
	unsigned char	rgbReserved;
} RGBQUAD,*LPRGBQUAD; 

typedef struct tagRGBTRIPLE {
	unsigned char rgbtBlue;
	unsigned char rgbtGreen;
	unsigned char rgbtRed;
} RGBTRIPLE,*LPRGBTRIPLE; 

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3 

#ifndef WIDTHBYTES
#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)  /* ULONG aligned ! */
#endif

#define DibWidthBytesN(lpbi, n) (uint32_t)WIDTHBYTES((uint32_t)(lpbi)->biWidth * (uint32_t)(n))
#define DibWidthBytes(lpbi)     DibWidthBytesN(lpbi, (lpbi)->biBitCount)

#define DibSizeImage(lpbi)      ((lpbi)->biSizeImage == 0 \
                                    ? ((uint32_t)DibWidthBytes(lpbi) * (uint32_t)(lpbi)->biHeight) \
                                    : (lpbi)->biSizeImage)

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#endif
