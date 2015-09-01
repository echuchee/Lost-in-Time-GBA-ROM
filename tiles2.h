
//{{BLOCK(tiles2)

//======================================================================
//
//	tiles2, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 32768 + 2048 = 35328
//
//	Time-stamp: 2013-04-22, 08:16:26
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TILES2_H
#define GRIT_TILES2_H

#define tiles2TilesLen 32768
extern const unsigned short tiles2Tiles[16384];

#define tiles2MapLen 2048
extern const unsigned short tiles2Map[1024];

#define tiles2PalLen 512
extern const unsigned short tiles2Pal[256];

#endif // GRIT_TILES2_H

//}}BLOCK(tiles2)
