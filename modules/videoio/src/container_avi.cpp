// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "opencv2/videoio/container_avi.private.hpp"

namespace cv
{

const unsigned int RIFF_CC = CV_FOURCC('R','I','F','F');
const unsigned int LIST_CC = CV_FOURCC('L','I','S','T');
const unsigned int HDRL_CC = CV_FOURCC('h','d','r','l');
const unsigned int AVIH_CC = CV_FOURCC('a','v','i','h');
const unsigned int STRL_CC = CV_FOURCC('s','t','r','l');
const unsigned int STRH_CC = CV_FOURCC('s','t','r','h');
const unsigned int STRF_CC = CV_FOURCC('s','t','r','f');
const unsigned int VIDS_CC = CV_FOURCC('v','i','d','s');
const unsigned int MJPG_CC = CV_FOURCC('M','J','P','G');
const unsigned int MOVI_CC = CV_FOURCC('m','o','v','i');
const unsigned int IDX1_CC = CV_FOURCC('i','d','x','1');
const unsigned int AVI_CC  = CV_FOURCC('A','V','I',' ');
const unsigned int AVIX_CC = CV_FOURCC('A','V','I','X');
const unsigned int JUNK_CC = CV_FOURCC('J','U','N','K');
const unsigned int INFO_CC = CV_FOURCC('I','N','F','O');
const unsigned int ODML_CC = CV_FOURCC('o','d','m','l');
const unsigned int DMLH_CC = CV_FOURCC('d','m','l','h');

String fourccToString(unsigned int fourcc);

#ifndef DWORD
typedef unsigned int DWORD;
#endif
#ifndef WORD
typedef unsigned short int WORD;
#endif
#ifndef LONG
typedef int  LONG;
#endif

#pragma pack(push, 1)
struct AviMainHeader
{
    DWORD dwMicroSecPerFrame;    //  The period between video frames
    DWORD dwMaxBytesPerSec;      //  Maximum data rate of the file
    DWORD dwReserved1;           // 0
    DWORD dwFlags;               //  0x10 AVIF_HASINDEX: The AVI file has an idx1 chunk containing an index at the end of the file.
    DWORD dwTotalFrames;         // Field of the main header specifies the total number of frames of data in file.
    DWORD dwInitialFrames;       // Is used for interleaved files
    DWORD dwStreams;             // Specifies the number of streams in the file.
    DWORD dwSuggestedBufferSize; // Field specifies the suggested buffer size forreading the file
    DWORD dwWidth;               // Fields specify the width of the AVIfile in pixels.
    DWORD dwHeight;              // Fields specify the height of the AVIfile in pixels.
    DWORD dwReserved[4];         // 0, 0, 0, 0
};

struct AviStreamHeader
{
    unsigned int fccType;              // 'vids', 'auds', 'txts'...
    unsigned int fccHandler;           // "cvid", "DIB "
    DWORD dwFlags;               // 0
    DWORD dwPriority;            // 0
    DWORD dwInitialFrames;       // 0
    DWORD dwScale;               // 1
    DWORD dwRate;                // Fps (dwRate - frame rate for video streams)
    DWORD dwStart;               // 0
    DWORD dwLength;              // Frames number (playing time of AVI file as defined by scale and rate)
    DWORD dwSuggestedBufferSize; // For reading the stream
    DWORD dwQuality;             // -1 (encoding quality. If set to -1, drivers use the default quality value)
    DWORD dwSampleSize;          // 0 means that each frame is in its own chunk
    struct {
        short int left;
        short int top;
        short int right;
        short int bottom;
    } rcFrame;                // If stream has a different size than dwWidth*dwHeight(unused)
};

struct AviIndex
{
    DWORD ckid;
    DWORD dwFlags;
    DWORD dwChunkOffset;
    DWORD dwChunkLength;
};

struct BitmapInfoHeader
{
    DWORD biSize;                // Write header size of BITMAPINFO header structure
    LONG  biWidth;               // width in pixels
    LONG  biHeight;              // height in pixels
    WORD  biPlanes;              // Number of color planes in which the data is stored
    WORD  biBitCount;            // Number of bits per pixel
    DWORD biCompression;         // Type of compression used (uncompressed: NO_COMPRESSION=0)
    DWORD biSizeImage;           // Image Buffer. Quicktime needs 3 bytes also for 8-bit png
                                 //   (biCompression==NO_COMPRESSION)?0:xDim*yDim*bytesPerPixel;
    LONG  biXPelsPerMeter;       // Horizontal resolution in pixels per meter
    LONG  biYPelsPerMeter;       // Vertical resolution in pixels per meter
    DWORD biClrUsed;             // 256 (color table size; for 8-bit only)
    DWORD biClrImportant;        // Specifies that the first x colors of the color table. Are important to the DIB.
};

struct RiffChunk
{
    unsigned int m_four_cc;
    unsigned int m_size;
};

struct RiffList
{
    unsigned int m_riff_or_list_cc;
    unsigned int m_size;
    unsigned int m_list_type_cc;
};

class VideoInputStream
{
public:
    VideoInputStream();
    VideoInputStream(const String& filename);
    ~VideoInputStream();
    VideoInputStream& read(char*, unsigned long long int);
    VideoInputStream& seekg(unsigned long long int);
    unsigned long long int tellg();
    bool isOpened() const;
    bool open(const String& filename);
    void close();
    operator bool();
    VideoInputStream& operator=(const VideoInputStream& stream);

private:
    bool    m_is_valid;
    String  m_fname;
    FILE*   m_f;
};

#pragma pack(pop)

inline VideoInputStream& operator >> (VideoInputStream& is, AviMainHeader& avih)
{
    is.read((char*)(&avih), sizeof(AviMainHeader));
    return is;
}
inline VideoInputStream& operator >> (VideoInputStream& is, AviStreamHeader& strh)
{
    is.read((char*)(&strh), sizeof(AviStreamHeader));
    return is;
}
inline VideoInputStream& operator >> (VideoInputStream& is, BitmapInfoHeader& bmph)
{
    is.read((char*)(&bmph), sizeof(BitmapInfoHeader));
    return is;
}
inline VideoInputStream& operator >> (VideoInputStream& is, AviIndex& idx1)
{
    is.read((char*)(&idx1), sizeof(idx1));
    return is;
}

inline VideoInputStream& operator >> (VideoInputStream& is, RiffChunk& riff_chunk)
{
    is.read((char*)(&riff_chunk), sizeof(riff_chunk));
    return is;
}

inline VideoInputStream& operator >> (VideoInputStream& is, RiffList& riff_list)
{
    is.read((char*)(&riff_list), sizeof(riff_list));
    return is;
}

static const int AVIH_STRH_SIZE = 56;
static const int STRF_SIZE = 40;
static const int AVI_DWFLAG = 0x00000910;
static const int AVI_DWSCALE = 1;
static const int AVI_DWQUALITY = -1;
static const int JUNK_SEEK = 4096;
static const int AVIIF_KEYFRAME = 0x10;
static const int MAX_BYTES_PER_SEC = 99999999;
static const int SUG_BUFFER_SIZE = 1048576;

String fourccToString(unsigned int fourcc)
{
    return format("%c%c%c%c", fourcc & 255, (fourcc >> 8) & 255, (fourcc >> 16) & 255, (fourcc >> 24) & 255);
}

VideoInputStream::VideoInputStream(): m_is_valid(false), m_f(0)
{
    m_fname = String();
}

VideoInputStream::VideoInputStream(const String& filename): m_is_valid(false), m_f(0)
{
    m_fname = filename;
    open(filename);
}

bool VideoInputStream::isOpened() const
{
    return m_f != 0;
}

bool VideoInputStream::open(const String& filename)
{
    close();

    m_f = fopen(filename.c_str(), "rb");

    m_is_valid = isOpened();

    return m_is_valid;
}

void VideoInputStream::close()
{
    if(isOpened())
    {
        m_is_valid = false;

        fclose(m_f);
        m_f = 0;
    }
}

VideoInputStream& VideoInputStream::read(char* buf, unsigned long long int count)
{
    if(isOpened())
    {
        m_is_valid = (count == fread((void*)buf, 1, (size_t)count, m_f));
    }

    return *this;
}

VideoInputStream& VideoInputStream::seekg(unsigned long long int pos)
{
    m_is_valid = (fseek(m_f, (long)pos, SEEK_SET) == 0);

    return *this;
}

unsigned long long int VideoInputStream::tellg()
{
    return ftell(m_f);
}

VideoInputStream::operator bool()
{
    return m_is_valid;
}

VideoInputStream& VideoInputStream::operator=(const VideoInputStream& stream)
{
    if (this != &stream) {
        m_fname = stream.m_fname;
        // m_f = stream.m_f;
        open(m_fname);
    }
    return *this;
}

VideoInputStream::~VideoInputStream()
{
    close();
}

AVIReadContainer::AVIReadContainer(): m_stream_id(0), m_movi_start(0), m_movi_end(0), m_width(0), m_height(0), m_fps(0), m_is_indx_present(false)
{
    m_file_stream = makePtr<VideoInputStream>();
}

void AVIReadContainer::initStream(const String &filename)
{
    m_file_stream = makePtr<VideoInputStream>(filename);
}

void AVIReadContainer::initStream(Ptr<VideoInputStream> m_file_stream_)
{
    m_file_stream = m_file_stream_;
}

void AVIReadContainer::close()
{
    m_file_stream->close();
}

bool AVIReadContainer::parseIndex(unsigned int index_size, frame_list& in_frame_list)
{
    unsigned long long int index_end = m_file_stream->tellg();
    index_end += index_size;
    bool result = false;

    while(m_file_stream && (m_file_stream->tellg() < index_end))
    {
        AviIndex idx1;
        *m_file_stream >> idx1;

        if(idx1.ckid == m_stream_id)
        {
            unsigned long long int absolute_pos = m_movi_start + idx1.dwChunkOffset;

            if(absolute_pos < m_movi_end)
            {
                in_frame_list.push_back(std::make_pair(absolute_pos, idx1.dwChunkLength));
            }
            else
            {
                //unsupported case
                fprintf(stderr, "Frame offset points outside movi section.\n");
            }
        }

        result = true;
    }

    return result;
}

bool AVIReadContainer::parseStrl(char stream_id, Codecs codec_)
{
    RiffChunk strh;
    *m_file_stream >> strh;

    if(m_file_stream && strh.m_four_cc == STRH_CC)
    {
        unsigned long long int next_strl_list = m_file_stream->tellg();
        next_strl_list += strh.m_size;

        AviStreamHeader strm_hdr;
        *m_file_stream >> strm_hdr;

        if (codec_ == MJPEG)
        {
            if(strm_hdr.fccType == VIDS_CC && strm_hdr.fccHandler == MJPG_CC)
            {
                char first_digit = (stream_id/10) + '0';
                char second_digit = (stream_id%10) + '0';

                if(m_stream_id == 0)
                {
                    m_stream_id = CV_FOURCC(first_digit, second_digit, 'd', 'c');
                    m_fps = double(strm_hdr.dwRate)/strm_hdr.dwScale;
                }
                else
                {
                    //second mjpeg video stream found which is not supported
                    fprintf(stderr, "More than one video stream found within AVI/AVIX list. Stream %c%cdc would be ignored\n", first_digit, second_digit);
                }

                return true;
            }
        }
    }

    return false;
}

void AVIReadContainer::skipJunk(RiffChunk& chunk)
{
    if(chunk.m_four_cc == JUNK_CC)
    {
        m_file_stream->seekg(m_file_stream->tellg() + chunk.m_size);
        *m_file_stream >> chunk;
    }
}

void AVIReadContainer::skipJunk(RiffList& list)
{
    if(list.m_riff_or_list_cc == JUNK_CC)
    {
        //JUNK chunk is 4 bytes less than LIST
        m_file_stream->seekg(m_file_stream->tellg() + list.m_size - 4);
        *m_file_stream >> list;
    }
}

bool AVIReadContainer::parseHdrlList(Codecs codec_)
{
    bool result = false;

    RiffChunk avih;
    *m_file_stream >> avih;

    if(m_file_stream && avih.m_four_cc == AVIH_CC)
    {
        unsigned long long int next_strl_list = m_file_stream->tellg();
        next_strl_list += avih.m_size;

        AviMainHeader avi_hdr;
        *m_file_stream >> avi_hdr;

        if(m_file_stream)
        {
            m_is_indx_present = ((avi_hdr.dwFlags & 0x10) != 0);
            DWORD number_of_streams = avi_hdr.dwStreams;
            CV_Assert(number_of_streams < 0xFF);
            m_width = avi_hdr.dwWidth;
            m_height = avi_hdr.dwHeight;

            //the number of strl lists must be equal to number of streams specified in main avi header
            for(DWORD i = 0; i < number_of_streams; ++i)
            {
                m_file_stream->seekg(next_strl_list);
                RiffList strl_list;
                *m_file_stream >> strl_list;

                if( m_file_stream && strl_list.m_riff_or_list_cc == LIST_CC && strl_list.m_list_type_cc == STRL_CC )
                {
                    next_strl_list = m_file_stream->tellg();
                    //RiffList::m_size includes fourCC field which we have already read
                    next_strl_list += (strl_list.m_size - 4);

                    result = parseStrl((char)i, codec_);
                }
                else
                {
                    printError(strl_list, STRL_CC);
                }
            }
        }
    }
    else
    {
        printError(avih, AVIH_CC);
    }

    return result;
}

bool AVIReadContainer::parseAviWithFrameList(frame_list& in_frame_list, Codecs codec_)
{
    RiffList hdrl_list;
    *m_file_stream >> hdrl_list;

    if( m_file_stream && hdrl_list.m_riff_or_list_cc == LIST_CC && hdrl_list.m_list_type_cc == HDRL_CC )
    {
        unsigned long long int next_list = m_file_stream->tellg();
        //RiffList::m_size includes fourCC field which we have already read
        next_list += (hdrl_list.m_size - 4);
        //parseHdrlList sets m_is_indx_present flag which would be used later
        if(parseHdrlList(codec_))
        {
            m_file_stream->seekg(next_list);

            RiffList some_list;
            *m_file_stream >> some_list;

            //an optional section INFO
            if(m_file_stream && some_list.m_riff_or_list_cc == LIST_CC && some_list.m_list_type_cc == INFO_CC)
            {
                next_list = m_file_stream->tellg();
                //RiffList::m_size includes fourCC field which we have already read
                next_list += (some_list.m_size - 4);
                parseInfo();

                m_file_stream->seekg(next_list);
                *m_file_stream >> some_list;
            }

            //an optional section JUNK
            skipJunk(some_list);

            //we are expecting to find here movi list. Must present in avi
            if(m_file_stream && some_list.m_riff_or_list_cc == LIST_CC && some_list.m_list_type_cc == MOVI_CC)
            {
                bool is_index_found = false;

                m_movi_start = m_file_stream->tellg();
                m_movi_start -= 4;

                m_movi_end = m_movi_start + some_list.m_size;
                //if m_is_indx_present is set to true we should find index
                if(m_is_indx_present)
                {
                    //we are expecting to find index section after movi list
                    unsigned int indx_pos = (unsigned int)m_movi_start + 4;
                    indx_pos += (some_list.m_size - 4);
                    m_file_stream->seekg(indx_pos);

                    RiffChunk index_chunk;
                    *m_file_stream >> index_chunk;

                    if(m_file_stream && index_chunk.m_four_cc == IDX1_CC)
                    {
                        is_index_found = parseIndex(index_chunk.m_size, in_frame_list);
                        //we are not going anywhere else
                    }
                    else
                    {
                        printError(index_chunk, IDX1_CC);
                    }
                }
                //index not present or we were not able to find it
                //parsing movi list
                if(!is_index_found)
                {
                    //not implemented
                    parseMovi(in_frame_list);

                    fprintf(stderr, "Failed to parse avi: index was not found\n");
                    //we are not going anywhere else
                }
            }
            else
            {
                printError(some_list, MOVI_CC);
            }
        }
    }
    else
    {
        printError(hdrl_list, HDRL_CC);
    }

    return in_frame_list.size() > 0;
}

std::vector<char> AVIReadContainer::readFrame(frame_iterator it)
{
    m_file_stream->seekg(it->first);

    RiffChunk chunk;
    *(m_file_stream) >> chunk;

    std::vector<char> result;

    result.reserve(chunk.m_size);
    result.resize(chunk.m_size);

    m_file_stream->read(&(result[0]), chunk.m_size); // result.data() failed with MSVS2008

    return result;
}

bool AVIReadContainer::parseRiff(frame_list &m_mjpeg_frames_)
{
    bool result = false;
    while(*m_file_stream)
    {
        RiffList riff_list;

        *m_file_stream >> riff_list;

        if( *m_file_stream && riff_list.m_riff_or_list_cc == RIFF_CC &&
            ((riff_list.m_list_type_cc == AVI_CC) | (riff_list.m_list_type_cc == AVIX_CC)) )
        {
            unsigned long long int next_riff = m_file_stream->tellg();
            //RiffList::m_size includes fourCC field which we have already read
            next_riff += (riff_list.m_size - 4);

            bool is_parsed = parseAvi(m_mjpeg_frames_, MJPEG);
            result = result || is_parsed;
            m_file_stream->seekg(next_riff);
        }
        else
        {
            break;
        }
    }
    return result;
}

void AVIReadContainer::printError(RiffList &list, unsigned int expected_fourcc)
{
    if(!m_file_stream)
    {
        fprintf(stderr, "Unexpected end of file while searching for %s list\n", fourccToString(expected_fourcc).c_str());
    }
    else if(list.m_riff_or_list_cc != LIST_CC)
    {
        fprintf(stderr, "Unexpected element. Expected: %s. Got: %s.\n", fourccToString(LIST_CC).c_str(), fourccToString(list.m_riff_or_list_cc).c_str());
    }
    else
    {
        fprintf(stderr, "Unexpected list type. Expected: %s. Got: %s.\n", fourccToString(expected_fourcc).c_str(), fourccToString(list.m_list_type_cc).c_str());
    }
}

void AVIReadContainer::printError(RiffChunk &chunk, unsigned int expected_fourcc)
{
    if(!m_file_stream)
    {
        fprintf(stderr, "Unexpected end of file while searching for %s chunk\n", fourccToString(expected_fourcc).c_str());
    }
    else
    {
        fprintf(stderr, "Unexpected element. Expected: %s. Got: %s.\n", fourccToString(expected_fourcc).c_str(), fourccToString(chunk.m_four_cc).c_str());
    }
}

class BitStream
{
public:
    BitStream();
    ~BitStream() { close(); }

    bool open(const String& filename);
    bool isOpened() const { return m_f != 0; }
    void close();

    void writeBlock();
    size_t getPos() const;
    void putByte(int val);
    void putBytes(const uchar* buf, int count);

    void putShort(int val);
    void putInt(int val);
    void jputShort(int val);
    void patchInt(int val, size_t pos);
    void jput(unsigned currval);
    void jflush(unsigned currval, int bitIdx);

protected:
    std::vector<uchar> m_buf;
    uchar*  m_start;
    uchar*  m_end;
    uchar*  m_current;
    size_t  m_pos;
    bool    m_is_opened;
    FILE*   m_f;
};

static const size_t DEFAULT_BLOCK_SIZE = (1 << 15);

BitStream::BitStream()
{
    m_buf.resize(DEFAULT_BLOCK_SIZE + 1024);
    m_start = &m_buf[0];
    m_end = m_start + DEFAULT_BLOCK_SIZE;
    m_is_opened = false;
    m_f = 0;
    m_current = 0;
    m_pos = 0;
}

bool BitStream::open(const String& filename)
{
    close();
    m_f = fopen(filename.c_str(), "wb");
    if( !m_f )
        return false;
    m_current = m_start;
    m_pos = 0;
    return true;
}

void BitStream::close()
{
    writeBlock();
    if( m_f )
        fclose(m_f);
    m_f = 0;
}

void BitStream::writeBlock()
{
    size_t wsz0 = m_current - m_start;
    if( wsz0 > 0 && m_f )
    {
        size_t wsz = fwrite(m_start, 1, wsz0, m_f);
        CV_Assert( wsz == wsz0 );
    }
    m_pos += wsz0;
    m_current = m_start;
}

size_t BitStream::getPos() const {
    return (size_t)(m_current - m_start) + m_pos;
}

void BitStream::putByte(int val)
{
    *m_current++ = (uchar)val;
    if( m_current >= m_end )
        writeBlock();
}

void BitStream::putBytes(const uchar* buf, int count)
{
    uchar* data = (uchar*)buf;
    CV_Assert(m_f && data && m_current && count >= 0);
    if( m_current >= m_end )
        writeBlock();

    while( count )
    {
        int l = (int)(m_end - m_current);

        if (l > count)
            l = count;

        if( l > 0 )
        {
            memcpy(m_current, data, l);
            m_current += l;
            data += l;
            count -= l;
        }
        if( m_current >= m_end )
            writeBlock();
    }
}

void BitStream::putShort(int val)
{
    m_current[0] = (uchar)val;
    m_current[1] = (uchar)(val >> 8);
    m_current += 2;
    if( m_current >= m_end )
        writeBlock();
}

void BitStream::putInt(int val)
{
    m_current[0] = (uchar)val;
    m_current[1] = (uchar)(val >> 8);
    m_current[2] = (uchar)(val >> 16);
    m_current[3] = (uchar)(val >> 24);
    m_current += 4;
    if( m_current >= m_end )
        writeBlock();
}

void BitStream::jputShort(int val)
{
    m_current[0] = (uchar)(val >> 8);
    m_current[1] = (uchar)val;
    m_current += 2;
    if( m_current >= m_end )
        writeBlock();
}

void BitStream::patchInt(int val, size_t pos)
{
    if( pos >= m_pos )
    {
        ptrdiff_t delta = pos - m_pos;
        CV_Assert( delta < m_current - m_start );
        m_start[delta] = (uchar)val;
        m_start[delta+1] = (uchar)(val >> 8);
        m_start[delta+2] = (uchar)(val >> 16);
        m_start[delta+3] = (uchar)(val >> 24);
    }
    else
    {
        long fpos = ftell(m_f);
        fseek(m_f, (long)pos, SEEK_SET);
        uchar buf[] = { (uchar)val, (uchar)(val >> 8), (uchar)(val >> 16), (uchar)(val >> 24) };
        fwrite(buf, 1, 4, m_f);
        fseek(m_f, fpos, SEEK_SET);
    }
}

void BitStream::jput(unsigned currval)
{
    uchar v;
    uchar* ptr = m_current;
    v = (uchar)(currval >> 24);
    *ptr++ = v;
    if( v == 255 )
        *ptr++ = 0;
    v = (uchar)(currval >> 16);
    *ptr++ = v;
    if( v == 255 )
        *ptr++ = 0;
    v = (uchar)(currval >> 8);
    *ptr++ = v;
    if( v == 255 )
        *ptr++ = 0;
    v = (uchar)currval;
    *ptr++ = v;
    if( v == 255 )
        *ptr++ = 0;
    m_current = ptr;
    if( m_current >= m_end )
        writeBlock();
}

void BitStream::jflush(unsigned currval, int bitIdx)
{
    uchar v;
    uchar* ptr = m_current;
    currval |= (1 << bitIdx)-1;
    while( bitIdx < 32 )
    {
        v = (uchar)(currval >> 24);
        *ptr++ = v;
        if( v == 255 )
            *ptr++ = 0;
        currval <<= 8;
        bitIdx += 8;
    }
    m_current = ptr;
    if( m_current >= m_end )
        writeBlock();
}

AVIWriteContainer::AVIWriteContainer() : strm(makePtr<BitStream>())
{
    outfps = 0;
    height = 0;
    width = 0;
    channels = 0;
    moviPointer = 0;
    strm->close();
}

AVIWriteContainer::~AVIWriteContainer() {
    strm->close();
    frameOffset.clear();
    frameSize.clear();
    AVIChunkSizeIndex.clear();
    frameNumIndexes.clear();
}

bool AVIWriteContainer::initContainer(const String& filename, double fps, Size size, bool iscolor)
{
    outfps = cvRound(fps);
    width = size.width;
    height = size.height;
    channels = iscolor ? 3 : 1;
    moviPointer = 0;
    bool result = strm->open(filename);
    return result;
}

void AVIWriteContainer::startWriteAVI(int stream_count)
{
    startWriteChunk(RIFF_CC);

    strm->putInt(AVI_CC);

    startWriteChunk(LIST_CC);

    strm->putInt(HDRL_CC);
    strm->putInt(AVIH_CC);
    strm->putInt(AVIH_STRH_SIZE);
    strm->putInt(cvRound(1e6 / outfps));
    strm->putInt(MAX_BYTES_PER_SEC);
    strm->putInt(0);
    strm->putInt(AVI_DWFLAG);

    frameNumIndexes.push_back(strm->getPos());

    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(stream_count); // number of streams
    strm->putInt(SUG_BUFFER_SIZE);
    strm->putInt(width);
    strm->putInt(height);
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(0);
}

void AVIWriteContainer::writeStreamHeader(Codecs codec_)
{
    // strh
    startWriteChunk(LIST_CC);

    strm->putInt(STRL_CC);
    strm->putInt(STRH_CC);
    strm->putInt(AVIH_STRH_SIZE);
    strm->putInt(VIDS_CC);
    switch (codec_) {
      case MJPEG:
        strm->putInt(MJPG_CC);
      break;
    }
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(AVI_DWSCALE);
    strm->putInt(outfps);
    strm->putInt(0);

    frameNumIndexes.push_back(strm->getPos());

    strm->putInt(0);
    strm->putInt(SUG_BUFFER_SIZE);
    strm->putInt(AVI_DWQUALITY);
    strm->putInt(0);
    strm->putShort(0);
    strm->putShort(0);
    strm->putShort(width);
    strm->putShort(height);

    // strf (use the BITMAPINFOHEADER for video)
    startWriteChunk(STRF_CC);

    strm->putInt(STRF_SIZE);
    strm->putInt(width);
    strm->putInt(height);
    strm->putShort(1); // planes (1 means interleaved data (after decompression))

    strm->putShort(8 * channels); // bits per pixel
    switch (codec_) {
      case MJPEG:
        strm->putInt(MJPG_CC);
      break;
    }
    strm->putInt(width * height * channels);
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(0);
    strm->putInt(0);
    // Must be indx chunk
    endWriteChunk(); // end strf

    endWriteChunk(); // end strl

    // odml
    startWriteChunk(LIST_CC);
    strm->putInt(ODML_CC);
    startWriteChunk(DMLH_CC);

    frameNumIndexes.push_back(strm->getPos());

    strm->putInt(0);
    strm->putInt(0);

    endWriteChunk(); // end dmlh
    endWriteChunk(); // end odml

    endWriteChunk(); // end hdrl

    // JUNK
    startWriteChunk(JUNK_CC);
    size_t pos = strm->getPos();
    for( ; pos < (size_t)JUNK_SEEK; pos += 4 )
        strm->putInt(0);
    endWriteChunk(); // end JUNK

    // movi
    startWriteChunk(LIST_CC);
    moviPointer = strm->getPos();
    strm->putInt(MOVI_CC);
}

void AVIWriteContainer::startWriteChunk(int fourcc)
{
    CV_Assert(fourcc != 0);
    strm->putInt(fourcc);

    AVIChunkSizeIndex.push_back(strm->getPos());
    strm->putInt(0);
}

void AVIWriteContainer::endWriteChunk()
{
    if( !AVIChunkSizeIndex.empty() )
    {
        size_t currpos = strm->getPos();
        size_t pospos = AVIChunkSizeIndex.back();
        AVIChunkSizeIndex.pop_back();
        int chunksz = (int)(currpos - (pospos + 4));
        strm->patchInt(chunksz, pospos);
    }
}

int AVIWriteContainer::getAVIIndex(int stream_number, StreamType strm_type) {
    char strm_indx[2];
    strm_indx[0] = '0' + static_cast<char>(stream_number / 10);
    strm_indx[1] = '0' + static_cast<char>(stream_number % 10);

    switch (strm_type) {
      case db: return CV_FOURCC(strm_indx[0], strm_indx[1], 'd', 'b');
      case dc: return CV_FOURCC(strm_indx[0], strm_indx[1], 'd', 'c');
      case pc: return CV_FOURCC(strm_indx[0], strm_indx[1], 'p', 'c');
      case wb: return CV_FOURCC(strm_indx[0], strm_indx[1], 'w', 'b');
      default: return CV_FOURCC(strm_indx[0], strm_indx[1], 'd', 'b');
    }
}

void AVIWriteContainer::writeIndex(int stream_number, StreamType strm_type)
{
    // old style AVI index. Must be Open-DML index
    startWriteChunk(IDX1_CC);
    int nframes = (int)frameOffset.size();
    for( int i = 0; i < nframes; i++ )
    {
        strm->putInt(getAVIIndex(stream_number, strm_type));
        strm->putInt(AVIIF_KEYFRAME);
        strm->putInt((int)frameOffset[i]);
        strm->putInt((int)frameSize[i]);
    }
    endWriteChunk(); // End idx1
}

void AVIWriteContainer::finishWriteAVI()
{
    int nframes = (int)frameOffset.size();
    // Record frames numbers to AVI Header
    while (!frameNumIndexes.empty())
    {
        size_t ppos = frameNumIndexes.back();
        frameNumIndexes.pop_back();
        strm->patchInt(nframes, ppos);
    }
    endWriteChunk(); // end RIFF
}

bool AVIWriteContainer::isOpenedStream() const { return strm->isOpened(); }

size_t AVIWriteContainer::getStreamPos() const { return strm->getPos(); }

void AVIWriteContainer::jputStreamShort(int val) { strm->jputShort(val); }

void AVIWriteContainer::putStreamBytes(const uchar *buf, int count) { strm->putBytes( buf, count ); }

void AVIWriteContainer::putStreamByte(int val) { strm->putByte(val); }

void AVIWriteContainer::jputStream(unsigned currval) { strm->jput(currval); }

void AVIWriteContainer::jflushStream(unsigned currval, int bitIdx) {  strm->jflush(currval, bitIdx); }

}
