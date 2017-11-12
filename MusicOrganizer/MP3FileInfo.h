#ifndef MP3FILEINFO_H
#define MP3FILEINFO_H
#define ID3LIB_LINKOPTION 3
#include "id3/tag.h"
#pragma comment(lib, "id3lib.lib")

  class MP3FileInfo
   {
    public:
      inline MP3FileInfo(void) :
        m_bOK(true),
        m_id3tag(0),
        m_mp3header(0),

          // Fileinfo
        szFilename(0), nFileSize(-1),
        bHasLyrics(false), bHasV1Tag(false), bHasV2Tag(false),

          // Headerinfo
        nBitRate(0), nCbrBitRate(0), nVbrBitRate(0), nSampleRate(0), nLength(0),
        szBitRate(0), szMpegLayer(0), szMpegVersion(0), szChannelMode(0),

          // ID3V1 Tag elements
        szArtist(0), szAlbum(0), szGenre(0), szTitle(0), szYear(0), szComment(0), szTrack(0),

          // ID3V2 Tag elements
        szComposer(0), szCopyright(0), szEncodedBy(0), szOriginalArtist(0), szURL(0)
       {
        // nothing to do; use Init() instead
       }

      virtual ~MP3FileInfo(void) { Free(); }


      bool Init(const char* cszFilename);
	  bool Change(const char* cszFilename, char *, char *, char *, char *, char*);
      void Free(void);
      inline bool isOK(void) const { return m_bOK; }


      /* Fileinfo */
      char* szFilename;
      bool  bHasLyrics, bHasV1Tag, bHasV2Tag;
      int   nFileSize;

      /* Headerinfo */
      int nCbrBitRate, nVbrBitRate, nBitRate, nSampleRate, nLength;
      char* szBitRate;
      char* szMpegLayer;
      char* szMpegVersion;
      char* szChannelMode;

      /* V1 up */
      char* szArtist;
      char* szTitle;
      char* szAlbum;
      char* szComment;
      char* szTrack; int nTrack;
      char* szYear; int nYear;
      char* szGenre;

      /* V2 only */
      char* szComposer;
      char* szCopyright;
      char* szEncodedBy;
      char* szOriginalArtist;
      char* szURL;

      void  ShowAllFrames(void);

    protected:
      bool m_bOK;

      ID3_Tag* m_id3tag;
      const Mp3_Headerinfo* m_mp3header;

      void m_parseHeader(void);
      void m_parseGenre(void);

      char* m_getNewString(ID3_FrameID fid);
   };



#endif // MP3FILEINFO_H