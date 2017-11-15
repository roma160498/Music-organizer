#include "stdafx.h"
#include "MP3FileInfo.h"
#include <assert.h>
#include <stdio.h>
#include <vector>

  bool MP3FileInfo::Init(const char* cszFilename)
   {
    Free();
    m_bOK = true;

    szFilename = new char[strlen(cszFilename)+1];
    sprintf(szFilename, cszFilename);

    m_id3tag = new ID3_Tag;
    m_id3tag->Link(szFilename);

    /* Fileinfo */
    bHasLyrics = m_id3tag->HasLyrics();
    bHasV1Tag  = m_id3tag->HasV1Tag();
    bHasV2Tag  = m_id3tag->HasV2Tag();
    nFileSize  = (int)m_id3tag->GetFileSize();

    /* Headerinfo */
    m_parseHeader();
    if(!m_bOK) { Free(); return m_bOK; }

    /* V1 up */
    szAlbum   = m_getNewString(ID3FID_ALBUM);
    szArtist  = m_getNewString(ID3FID_LEADARTIST);
    szComment = m_getNewString(ID3FID_COMMENT);
    szTitle   = m_getNewString(ID3FID_TITLE);
    szTrack   = m_getNewString(ID3FID_TRACKNUM);
    szYear    = m_getNewString(ID3FID_YEAR);

    /* V2 */
    szComposer        = m_getNewString(ID3FID_COMPOSER);
    szCopyright       = m_getNewString(ID3FID_COPYRIGHT);
    szEncodedBy       = m_getNewString(ID3FID_ENCODEDBY);
    szOriginalArtist  = m_getNewString(ID3FID_ORIGARTIST);
    szURL             = m_getNewString(ID3FID_WWWUSER);

    //m_parseGenre();
    if(szYear)  sscanf(szYear,  "%i", &nYear);
    if(szTrack) sscanf(szTrack, "%i", &nTrack);

    return m_bOK;
   }
  /// parses header infomation
  void MP3FileInfo::m_parseHeader(void)
  {
	  assert(m_bOK);

	  m_mp3header = m_id3tag->GetMp3HeaderInfo();
	  if (!m_mp3header)
	  {
		  //printf("\n[Error]:\nGetMp3HeaderInfo() failed!\n(file: %s)\n\n", szFilename);
		  Free();
		  return;
	  }

	  nCbrBitRate = m_mp3header->bitrate;
	  nVbrBitRate = m_mp3header->vbr_bitrate;
	  nSampleRate = m_mp3header->frequency;
	  nLength = m_mp3header->time;

	  szBitRate = new char[20];
	  if (nVbrBitRate>0)
	  {
		  sprintf(szBitRate, "%i kbit/s (VBR)", nVbrBitRate / 1000);
		  nBitRate = nVbrBitRate;
	  }
	  else
	  {
		  sprintf(szBitRate, "%i kbit/s", nCbrBitRate / 1000);
		  nBitRate = nCbrBitRate;
	  }


	  // MPEG Layer
	  szMpegLayer = new char[10];
	  switch (m_mp3header->layer)
	  {
	  case MPEGLAYER_UNDEFINED: sprintf(szMpegLayer, "UNDEFINED"); break;
	  case MPEGLAYER_III:       sprintf(szMpegLayer, "Layer III"); break;
	  case MPEGLAYER_II:        sprintf(szMpegLayer, "Layer II");  break;
	  case MPEGLAYER_I:         sprintf(szMpegLayer, "Layer I");   break;
	  default:                  szMpegLayer = 0;
	  }

	  // MPEG Version
	  szMpegVersion = new char[10];
	  switch (m_mp3header->version)
	  {
	  case MPEGVERSION_2_5: sprintf(szMpegVersion, "MPEG 2.5"); break;
	  case MPEGVERSION_2:   sprintf(szMpegVersion, "MPEG 2");   break;
	  case MPEGVERSION_1:   sprintf(szMpegVersion, "MPEG 1");   break;
	  default:              szMpegVersion = 0;
	  }

	  // Channel Mode
	  szChannelMode = new char[15];
	  switch (m_mp3header->channelmode)
	  {
	  case MP3CHANNELMODE_STEREO:         sprintf(szChannelMode, "Stereo");         break;
	  case MP3CHANNELMODE_JOINT_STEREO:   sprintf(szChannelMode, "Joint Stereo");   break;
	  case MP3CHANNELMODE_DUAL_CHANNEL:   sprintf(szChannelMode, "Dual Channel");   break;
	  case MP3CHANNELMODE_SINGLE_CHANNEL: sprintf(szChannelMode, "Single Channel"); break;
	  default:                            szChannelMode = 0;
	  }
  }
 
  bool MP3FileInfo::Change(const char* cszFilename, char * title, char * album, char * artist, char * year, char * comment)
  {
	  Free();
	  m_bOK = true;

	  // copy filename
	  szFilename = new char[strlen(cszFilename) + 1];
	  sprintf(szFilename, cszFilename);

	  m_id3tag = new ID3_Tag;
	  m_id3tag->Link(szFilename);

	  /* Fileinfo */
	  bHasLyrics = m_id3tag->HasLyrics();
	  bHasV1Tag = m_id3tag->HasV1Tag();
	  bHasV2Tag = m_id3tag->HasV2Tag();
	  nFileSize = (int)m_id3tag->GetFileSize();

	  /* Headerinfo */
	  m_parseHeader();

	  if (!m_bOK) { Free(); return m_bOK; }
	  std::vector<ID3_FrameID> idVector;
	  std::vector<char *> attributes;
	  idVector.push_back(ID3FID_TITLE);
	  idVector.push_back(ID3FID_ALBUM);
	  idVector.push_back(ID3FID_LEADARTIST);
	  idVector.push_back(ID3FID_YEAR);
	  idVector.push_back(ID3FID_COMMENT);
	  attributes.push_back(title);
	  attributes.push_back(album);
	  attributes.push_back(artist);
	  attributes.push_back(year);
	  attributes.push_back(comment);
	  ID3_Frame* frame = NULL;
	  ID3_Frame* tempFrame = NULL;
	  ID3_FrameID frameID;
	  ID3_Field* field = 0;
	  for (int i = 0; i < 5; i++)
	  {
		  frameID = idVector[i];
		  frame = m_id3tag->Find(frameID);
		  if (frame == NULL)
		  {
			  tempFrame = new ID3_Frame(idVector[i]);
			  m_id3tag->AddFrame(tempFrame);
			  frame = m_id3tag->Find(frameID);
		  }
		  if (frame->Contains(ID3FN_TEXT))
		  {
			  field = frame->GetField(ID3FN_TEXT);
			  field->SetEncoding(ID3TE_ISO8859_1);
			  field->Set(attributes[i]);
		  }
	  }

	  m_id3tag->Update();
	  return m_bOK;
  }

  void MP3FileInfo::Free(void)
   {
    if(m_bOK) // only free if OK
     {
      m_bOK = false;

      if(m_id3tag!=0)
       {
          // in rare cases (e.g. if file is corrupt)
          // "delete m_id3tag" fails ...
        try { delete m_id3tag; } catch(...) {}
        m_id3tag = 0;
       }

      /* fileinfo */
      delete[] szFilename; szFilename = 0; nFileSize = -1;
      bHasLyrics = false; bHasV1Tag = false; bHasV2Tag = false;

      /* headerinfo */
      m_mp3header = 0;
      nVbrBitRate = 0; nBitRate = 0; nCbrBitRate = 0;
      nSampleRate = 0; nLength  = 0;
      delete[] szBitRate;     szBitRate = 0;
      delete[] szMpegLayer;   szMpegLayer = 0;
      delete[] szMpegVersion; szMpegVersion = 0;
      delete[] szChannelMode; szChannelMode = 0;

      /* ID3V1 Tag elements */
      nTrack = 0; nYear = 0;
      delete[] szArtist;  szArtist  = 0;
      delete[] szTitle;   szTitle   = 0;
      delete[] szAlbum;   szAlbum   = 0;
      delete[] szComment; szComment = 0;
      delete[] szTrack;   szTrack   = 0;
      delete[] szYear;    szYear    = 0;
      delete[] szGenre;   szGenre   = 0;

      /* V2 up */
      delete[] szComposer;        szComposer= 0;
      delete[] szCopyright;       szCopyright= 0;
      delete[] szEncodedBy;       szEncodedBy= 0;
      delete[] szOriginalArtist;  szOriginalArtist= 0;
      delete[] szURL;             szURL= 0;
     }
   }

    /// extracts a string
  
  char* MP3FileInfo::m_getNewString(ID3_FrameID fid)
   {
    assert(m_bOK);

    ID3_Frame* frame = m_id3tag->Find(fid); // find frame
    if(frame==0) { return 0; }

    if(!frame->Contains(ID3FN_TEXTENC))
     { delete frame; return 0; } // frame contains no text

      // look for field type
    ID3_Field* field = 0;
    if(frame->Contains(ID3FN_TEXT)) // Text field
     { field = frame->GetField(ID3FN_TEXT); }

    if(frame->Contains(ID3FN_URL) && (field==0)) // URL field
     { field = frame->GetField(ID3FN_URL); }

    if(field==0) { delete frame; return 0; } // no field found

    field->SetEncoding(ID3TE_ISO8859_1); // use Latin-1 charset

    const char* res = field->GetRawText(); // TODO: GetRawUnicodeText
    char* buf = new char[strlen(res)+1]; // new string
    sprintf(buf, res); // copy

    return buf;
   }
