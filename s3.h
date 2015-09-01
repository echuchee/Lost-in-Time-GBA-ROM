
//{{BLOCK(s3)

//======================================================================
//
//	s3, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 32768 + 2048 = 35328
//
//	Time-stamp: 2013-04-22, 07:51:17
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_S3_H
#define GRIT_S3_H

#define s3TilesLen 32768
extern const unsigned short s3Tiles[16384];

#define s3MapLen 2048
extern const unsigned short s3Map[1024];

#define s3PalLen 512
extern const unsigned short s3Pal[256];

#endif // GRIT_S3_H

//}}BLOCK(s3)
