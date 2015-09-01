
//{{BLOCK(s2)

//======================================================================
//
//	s2, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 32768 + 2048 = 35328
//
//	Time-stamp: 2013-04-22, 02:05:41
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_S2_H
#define GRIT_S2_H

#define s2TilesLen 32768
extern const unsigned short s2Tiles[16384];

#define s2MapLen 2048
extern const unsigned short s2Map[1024];

#define s2PalLen 512
extern const unsigned short s2Pal[256];

#endif // GRIT_S2_H

//}}BLOCK(s2)
