/*
 * reader_asset.c
 *
 *  Created on: Dec 11, 2011
 *      Author: Clark Scheff
 *
 * Description: Used to read a module in from an android asset.
 */

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include "mikmod.h"

extern AAsset* asset;
static off_t g_offset = -1;
static int g_eof = 0;

BOOL ASSET_Seek(MREADER* reader, long offset, int whence)
{
	g_offset = AAsset_seek(asset, (off_t)offset, whence);
	if (g_offset != -1)
		return 1;

	return 0;
}

long ASSET_Tell(MREADER* reader)
{
	return g_offset;
}

BOOL ASSET_Read(MREADER* reader, void* buf, size_t count)
{
	g_eof = AAsset_read(asset, buf, count);
	if (g_eof == -1)
		return 0;
	g_offset += g_eof;
	return 1;
}

int ASSET_Get(MREADER* reader)
{
	UBYTE data;
	g_eof = AAsset_read(asset, &data, 1);
	if (g_eof != -1)
		g_offset += g_eof;
	return (int)data;
}

BOOL ASSET_Eof(MREADER* reader)
{
	return (g_eof <= 0);
}

MIKMODAPI MREADER reader_asset = {
	ASSET_Seek,
	ASSET_Tell,
	ASSET_Read,
	ASSET_Get,
	ASSET_Eof
};
