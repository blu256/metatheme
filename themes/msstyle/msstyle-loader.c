/**
 * This file is part of MetaTheme.
 * Copyright (c) 2005 Petr Jelinek <pjmodos@centrum.cz>
 *
 * MetaTheme is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MetaTheme is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MetaTheme; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>
#include "msstyle-loader.h"
#include "structures.h"
#include "msstyle-ini.h"

static uint32_t dwResourceSectionVA;
static int lastres;

void get_resources(ms_theme_t *ms, PIMAGE_RESOURCE_DIRECTORY ridRoot, PIMAGE_RESOURCE_DIRECTORY ridScan, int level, uint32_t restype, const char *parentname);
void bitfield_to_image(MT_ENGINE *engine, unsigned char *src, MT_IMAGE *img, uint16_t redmask, uint16_t greenmask, uint16_t bluemask, uint32_t width, uint32_t height, unsigned char bpp);


gboolean free_bitmap_entry(gpointer key, gpointer val, gpointer user_data)
{
	bitmap_entry *be = (bitmap_entry*)val;
	g_free(key);
	if (be->image)
		free(be->image);
	g_free(be);
	return (TRUE);
}


char *unicode_to_ascii(const char *str, int len)
{
	int i;
	char *ret = malloc(len+1);

	for (i = 0; i < len; i++)
	{
		ret[i] = str[i*2];
	}

	ret[i] = 0;

	return ret;
}


ms_theme_t *ms_open_theme(const char *filename)
{
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS ntHeaders;
	PIMAGE_SECTION_HEADER sectionHeaders;
	PIMAGE_RESOURCE_DIRECTORY resourceDir;
	uint32_t dwResourceSectionIndex = 0xFFFFFFFF;
	struct stat statbuf;
	int i;
	ms_theme_t *ms = calloc(1, sizeof(ms_theme_t));

	if ((ms->themefd = open (filename, O_RDONLY)) < 0)
	{
		fprintf(stderr, "Can't open theme file %s\n", filename);
		free(ms);
		return NULL;
	}

	if (fstat (ms->themefd, &statbuf) < 0)
	{
		fprintf(stderr, "fstat error\n");
		close(ms->themefd);
		free(ms);
		return NULL;
	}

	ms->pedatasize = statbuf.st_size;

	ms->pedata = mmap (0, ms->pedatasize, PROT_READ, MAP_SHARED, ms->themefd, 0);
	if (ms->pedata == MAP_FAILED)
	{
		close(ms->themefd);
		free(ms);
		fprintf(stderr, "Error mmaping theme file\n");
		return NULL;
	}

	dosHeader = (PIMAGE_DOS_HEADER)ms->pedata;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		close(ms->themefd);
		free(ms);
		fprintf(stderr, "Not valid DOS header\n");
		return NULL;
	}

	ntHeaders = (PIMAGE_NT_HEADERS)(ms->pedata + dosHeader->e_lfanew);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)	
	{
		close(ms->themefd);
		free(ms);
		fprintf(stderr, "Not valid NT headers\n");
		return NULL;
	}

	// Get resource section virtual address
	dwResourceSectionVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	// Pointer to the sections headers array
	sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);

	// Find resource section index in the array
	for (i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (dwResourceSectionVA == sectionHeaders[i].VirtualAddress)
		{
			dwResourceSectionIndex = i;

	  		if (!sectionHeaders[i].PointerToRawData)
			{
				close(ms->themefd);
				fprintf(stderr, "Error searching for resource section\n");
				return NULL;
			}
		}

		if (sectionHeaders[i].PointerToRawData > ms->pedatasize)
		{
			close(ms->themefd);
			fprintf(stderr, "Invalid section pointer\n");
			return NULL;
		}
	}

	if (dwResourceSectionIndex == ntHeaders->FileHeader.NumberOfSections)
	{
		close(ms->themefd);
		fprintf(stderr, "No resource section in this file\n");
		return NULL;
	}

	resourceDir = (PIMAGE_RESOURCE_DIRECTORY)(ms->pedata + sectionHeaders[dwResourceSectionIndex].PointerToRawData);

	ms->bitmaps = g_hash_table_new(g_str_hash, g_str_equal);
	ms->ini_files = msstyle_ini_files_new();
	ms->ms_theme_info = calloc(1, sizeof(ms_theme_info_t));

	lastres = 0;
	get_resources(ms, resourceDir, resourceDir, 0, 0, ".rsrc");
	
	return ms;
}

void ms_close_theme(ms_theme_t *ms)
{
	g_hash_table_foreach_remove(ms->bitmaps, free_bitmap_entry, NULL);
	g_hash_table_destroy(ms->bitmaps);
	g_hash_table_destroy(ms->ini_files);
	munmap(ms->pedata, ms->pedatasize);
	close(ms->themefd);
	free(ms->ms_theme_info);
}

MT_IMAGE *ms_load_widget(ms_theme_t *ms, MT_ENGINE *engine, char *name, int index)
{
	bitmap_entry *be = (bitmap_entry *)g_hash_table_lookup(ms->bitmaps, name);

	if (be->image == NULL)
	{
		//BITMAPFILEHEADER *bf = (BITMAPFILEHEADER *)be->PointerToRawData;
		BITMAPINFOHEADER *bi = (BITMAPINFOHEADER *)be->PointerToRawData; //+sizeof(BITMAPFILEHEADER);
		BITMAPCOREHEADER bc;
		int offset = 0; //sizeof(BITMAPFILEHEADER);
		int isold = 0;
		int isflipped = 0;
		int i, numcolors, align;
		MT_COLOR *palette = NULL;
		BITMAPINFOHEADER *_bi = (BITMAPINFOHEADER *)be->PointerToRawData;

		switch (bi->biSize)
		{
			case sizeof(BITMAPINFOHEADER):
				offset += sizeof(BITMAPINFOHEADER);
				break;

			case sizeof(BITMAPCOREHEADER):
				bc = *(BITMAPCOREHEADER*)bi;
				bi->biSize          = bc.bcSize;
				bi->biWidth         = (uint32_t)bc.bcWidth;
				bi->biHeight        = (uint32_t)bc.bcHeight;
				bi->biPlanes        = bc.bcPlanes;
				bi->biBitCount      = bc.bcBitCount;
				bi->biCompression   = BI_RGB;
				bi->biSizeImage     = 0;
				bi->biXPelsPerMeter = 0;
				bi->biYPelsPerMeter = 0;
				bi->biClrUsed       = 0;
				bi->biClrImportant  = 0;

				isold = 1;
				offset += sizeof(BITMAPCOREHEADER);
				break;

			default:
				return NULL;
		}

		if (bi->biHeight < 0)
		{
			isflipped = 1;
			bi->biHeight =- bi->biHeight;
		}

      /*
		if (bi->biSizeImage == 0)
			bi->biSizeImage = DibSizeImage(bi);
		if (bi->biClrUsed == 0)
			bi->biClrUsed = DibNumColors(bi);
		*/

		numcolors = (bi->biClrUsed == 0 && bi->biBitCount <= 8 ? (int)(1 << (int)bi->biBitCount) : (int)bi->biClrUsed);
		if (numcolors > 0) {
			palette = calloc(sizeof(MT_COLOR)*numcolors, 1);
		}

		if (isold)
		{
			for (i = numcolors-1; i >= 0; i--)
			{
				palette[i].b = *(((unsigned char*)be->PointerToRawData)+offset+i*3+0);
				palette[i].g = *(((unsigned char*)be->PointerToRawData)+offset+i*3+1);
				palette[i].r = *(((unsigned char*)be->PointerToRawData)+offset+i*3+2);
			}
			offset += numcolors*3;
		}
		else
		{
			for (i = 0; i<numcolors; i++)
			{
				palette[i].b = *(((unsigned char*)be->PointerToRawData)+offset+i*4+0);
				palette[i].g = *(((unsigned char*)be->PointerToRawData)+offset+i*4+1);
				palette[i].r = *(((unsigned char*)be->PointerToRawData)+offset+i*4+2);
				//palette[i].a = *(unsigned char*)be->PointerToRawData+offset+i*4+3;
				palette[i].a = 255;
				if (palette[i].r == 255 && palette[i].g == 0 && palette[i].b == 255) palette[i].a = 0;
			}
			offset += numcolors*4;
		}
		
		align = (4-((bi->biWidth)%4)) & 3;

		if (bi->biWidth < 1 || bi->biHeight < 1) {
			free(palette);
			return NULL;
		}
		
		be->image = mt_image_new(bi->biWidth, bi->biHeight);
		switch (bi->biBitCount)
		{
			uint32_t bfmask[3];

			case 32:
				if (bi->biCompression == BI_BITFIELDS || bi->biCompression == BI_RGB)
				{
					bitfield_to_image(engine, (unsigned char*)be->PointerToRawData+offset, be->image, 
						0, 0, 0, bi->biWidth, bi->biHeight, 32);
				}
				break;

			case 24:
				if (bi->biCompression == BI_RGB)
				{
					bitfield_to_image(engine, (unsigned char*)be->PointerToRawData+offset, be->image, 
						0, 0, 0, bi->biWidth, bi->biHeight, 24);
				}
				else
					return NULL;
				break;

			case 16:
				fprintf(stderr, "TODO\n");
				return NULL;

				/*
				if (bi->biCompression == BI_BITFIELDS)
				{
					bfmask[0] = *(uint32_t*)be->PointerToRawData+offset;
					bfmask[1] = *(uint32_t*)be->PointerToRawData+offset+4;
					bfmask[2] = *(uint32_t*)be->PointerToRawData+offset+8;
					offset += 12;
				}
				else
				{
					bfmask[0] = 0x7C00; bfmask[1] = 0x3E0; bfmask[2] = 0x1F; //RGB555
				}

				bitfield_to_image(engine, (unsigned char*)be->PointerToRawData+offset, be->image, 
					bfmask[0], bfmask[1], bfmask[2], bi->biWidth, bi->biHeight, 16);
				*/
				break;

			case 8:
				switch (bi->biCompression)
				{
					int x, y;
					MT_COLOR col;

					case BI_RGB:
					{
						for (y = bi->biHeight-1; y >= 0; y--)
						{
							for (x = bi->biWidth-1; x >= 0; x--)
							{
								int p = *((unsigned char*)(be->PointerToRawData) + offset + y*(bi->biWidth+align) +x);
								mt_image_set_pixel(be->image, x, (!isflipped)? (bi->biHeight - y - 1) : y, &palette[p]);
							}
						}
						break;
					}

					case BI_RLE8:
						return NULL;
						break;

					default:
						return NULL;
				}
				break;

			case 4:
				switch (bi->biCompression)
				{
					int x, y;
					MT_COLOR col;

					case BI_RGB:
					case BI_RLE4:
						return NULL;
						break;
				}

			case 1:
				return NULL;
				break;
		}

		free(palette);
	}

	return be->image;
}


void bitfield_to_image(MT_ENGINE *engine, unsigned char *src, MT_IMAGE *img, uint16_t redmask, uint16_t greenmask, uint16_t bluemask, uint32_t width, uint32_t height, unsigned char bpp)
{
	int i;
	long x, x2, y, y2, effwidth;
	MT_COLOR col;

	col.a = 255;

	switch (bpp)
	{
		case 16:
		{
			uint32_t ns[3] = {0,0,0};
			uint16_t w;
			effwidth = (((width + 1) / 2) * 4);

			// compute the number of shift for each mask
			for (i = 0; i < 16; i++)
			{
				if ((redmask>>i)&0x01) ns[0]++;
				if ((greenmask>>i)&0x01) ns[1]++;
				if ((bluemask>>i)&0x01) ns[2]++;
			}
			ns[1]+=ns[0]; ns[2]+=ns[1];	ns[0]=8-ns[0]; ns[1]-=8; ns[2]-=8;

			for (y = height-1; y >= 0; y--)
			{
				y2 = effwidth*y;
				for (x = width-1; x >= 0; x--)
				{
					x2 = 2*x+y2;
					w = (uint16_t)(src[x2]+256*src[1+x2]);
					col.b = (unsigned char)((w & bluemask)<<ns[0]);
					col.g = (unsigned char)((w & greenmask)>>ns[1]);
					col.r = (unsigned char)((w & redmask)>>ns[2]);
					mt_image_set_pixel(img, x, y, &col);
				}
			}
			break;
		}
		case 24:
		{
			effwidth = width * 3 + (width%4);

			for (y = height-1; y >= 0; y--)
			{
				y2=effwidth*y;
				for (x = width-1; x >= 0; x--)
				{
					x2 = 3*x+y2;
					col.b = src[x2];
					col.g = src[x2+1];
					col.r = src[x2+2];
					col.a = 255;
					if (col.r == 255 && col.g == 0 && col.b == 255) col.a = 0;
					mt_image_set_pixel(img, x, height - y - 1, &col);
				}
			}

			break;
		}
		case 32:
		{
			effwidth = width * 4;

			for (y = height-1; y >= 0; y--)
			{
				y2=effwidth*y;
				for (x = width-1; x >= 0; x--)
				{
					x2 = 4*x+y2;
					col.b = src[x2];
					col.g = src[x2+1];
					col.r = src[x2+2];
					col.a = src[x2+3];
					if (col.r == 255 && col.g == 0 && col.b == 255) col.a = 0;
					mt_image_set_pixel(img, x, height - y - 1, &col);
				}
			}
			break;
		}
	}

	return;
}


void get_resources(ms_theme_t *ms, PIMAGE_RESOURCE_DIRECTORY ridRoot, PIMAGE_RESOURCE_DIRECTORY ridScan, int level, uint32_t restype, const char *parentname)
{
	char* name;
	bitmap_entry *be = NULL;
	PIMAGE_RESOURCE_DATA_ENTRY rde = NULL;
	PIMAGE_RESOURCE_DIR_STRING_U rdsu = NULL;
	int i;

	if ( level == 1 && !(restype & IMAGE_RESOURCE_NAME_IS_STRING))
	{
		lastres = restype;
	}

	if (!(restype & IMAGE_RESOURCE_NAME_IS_STRING))
	{
		name = calloc(11, 1);
		sprintf(name, "%d", (restype & 0x7FFFFFFF));
	}
	else
	{
		rdsu = (PIMAGE_RESOURCE_DIR_STRING_U)((char*)ridRoot + (restype & 0x7FFFFFFF));
		name = unicode_to_ascii((char*)rdsu->NameString, rdsu->Length);
	}

	for (i = 0; i < ridScan->NumberOfNamedEntries + ridScan->NumberOfIdEntries; i++)
	{
		if (ridScan->DirectoryEntries[i].DataIsDirectory)
		{
			get_resources(ms, ridRoot, (PIMAGE_RESOURCE_DIRECTORY)((char*)ridRoot + ridScan->DirectoryEntries[i].OffsetToDirectory), level+1, ridScan->DirectoryEntries[i].Name, name);
		}
		else
		{
			rde = (PIMAGE_RESOURCE_DATA_ENTRY)(ridScan->DirectoryEntries[i].OffsetToData + (char*)ridRoot);
			switch(lastres)
			{
				case RT_BITMAP:
  					be = (bitmap_entry*)calloc(sizeof(bitmap_entry), 1);
  					be->PointerToRawData = (char*)ridRoot + rde->OffsetToData - dwResourceSectionVA;
					be->sizeOfRawData = rde->Size;
					g_hash_table_insert(ms->bitmaps, name, be);
					break;

				case RT_STRING:
					if (restype == 313)
					{
						int offset = 0;
						for (i = 0; i < 16; i++) //stringtable always has 16 strings
						{
							rdsu = (PIMAGE_RESOURCE_DIR_STRING_U)((char*)ridRoot + rde->OffsetToData - dwResourceSectionVA + offset);

							switch (i)
							{
								case 8: //Theme Name
									ms->ms_theme_info->name = unicode_to_ascii((char*)rdsu->NameString, rdsu->Length);
									break;
								case 10: //Author
									ms->ms_theme_info->author = unicode_to_ascii((char*)rdsu->NameString, rdsu->Length);
									break;
								case 12: //Copyright
									ms->ms_theme_info->copyright = unicode_to_ascii((char*)rdsu->NameString, rdsu->Length);
									break;
								case 13: //Web
									ms->ms_theme_info->website = unicode_to_ascii((char*)rdsu->NameString, rdsu->Length);
									break;
							}
							offset += rdsu->Length*2+sizeof(rdsu->Length); //uhm
						}
					}
					break;

				case 0: // unknown
					if (!strcmp(parentname, "FILERESNAMES"))
					{
						ms->ms_theme_info->defaultini = unicode_to_ascii((char*)ridRoot + rde->OffsetToData - dwResourceSectionVA, rde->Size);
					}
					else if (!strcmp(parentname, "TEXTFILE"))
					{
						INI_FILE *file = g_new(INI_FILE, 1);
						file->start = (char*)ridRoot + rde->OffsetToData - dwResourceSectionVA;
						file->length = rde->Size;
						file->sections = NULL;
						g_hash_table_insert(ms->ini_files, g_strdup(name), file);
					}
					break;
			}
		}
	}
}


INI_FILE *ms_load_ini(ms_theme_t *ms, const char *name)
{
	return msstyle_ini_get(ms->ini_files, name);
}
