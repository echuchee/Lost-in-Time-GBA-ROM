
//{{BLOCK(s1)

//======================================================================
//
//	s1, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 32768 + 2048 = 35328
//
//	Time-stamp: 2013-04-22, 02:01:57
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_S1_H
#define GRIT_S1_H

#define s1TilesLen 32768
extern const unsigned short s1Tiles[16384];

#define s1MapLen 2048
extern const unsigned short s1Map[1024];

#define s1PalLen 512
extern const unsigned short s1Pal[256];

#endif // GRIT_S1_H

//}}BLOCK(s1)
