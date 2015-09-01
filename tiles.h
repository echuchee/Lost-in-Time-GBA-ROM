
//{{BLOCK(tiles)

//======================================================================
//
//	tiles, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 32768 + 2048 = 35328
//
//	Time-stamp: 2013-04-11, 07:34:11
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TILES_H
#define GRIT_TILES_H

#define tilesTilesLen 32768
extern const unsigned short tilesTiles[16384];

#define tilesMapLen 2048
extern const unsigned short tilesMap[1024];

#define tilesPalLen 512
extern const unsigned short tilesPal[256];

#endif // GRIT_TILES_H

//}}BLOCK(tiles)
