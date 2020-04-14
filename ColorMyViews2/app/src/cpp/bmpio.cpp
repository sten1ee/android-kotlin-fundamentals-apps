//
// Created by Stanislav Jordanov on 10.04.20.
//
#include <cstdint>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace std;

#ifdef NDEBUG
# define DLOG(E) ((void)0)
#else
# define DLOG(E) std::cout << E
#endif

#ifndef __LITTLE_ENDIAN__
# warning: Although this was written with both Big and Little endian CPUs in mind, it was only tested on Little endian CPU!
#endif

class Bitmap {
    typedef uint8_t  WORD[2];
    typedef uint8_t  DWORD[4];

public:
    struct FileHeader {
      WORD   signature;
      DWORD  fileSize;
      DWORD  reserved;
      DWORD  imgOffset;
      DWORD  hdrSize; // byte size of the following header (including this DWORD)
    };

    struct V2xInfoHeader { // 40 bytes in total
    //DWORD  hdrSize;          // Size of this header in bytes
      DWORD  width;
      DWORD  height;
      WORD   numPlanes;
      WORD   bitsPerPixel;
      DWORD  compression;
      DWORD  imgSize;
      DWORD  xResolution;
      DWORD  yResolution;
      DWORD  colorsUsed;
      DWORD  colorsImportant;
    };

    struct V1xInfoHeader { // 12 bytes in total;
      //DWORD hdrSize;         // Size of this header in bytes
        WORD  width;           // Image width in pixels
        WORD  height;          // Image height in pixels
        WORD  numPlanes;       // Number of color planes
        WORD  bitsPerPixel;    // Number of bits per pixel
    };

private:
    size_t  _imgOffset;
    size_t  _hdrSize; // byte size of the following header (including this DWORD)
    size_t  _width;
    size_t  _height;
    size_t  _bitsPerPixel;
    bool    _isV1Hdr;

    mutable FileHeader fileHdr;
    union {
        mutable V1xInfoHeader v1InfoHdr;
        mutable V2xInfoHeader infoHdr;
    };

#ifdef __LITTLE_ENDIAN__
    static inline uint16_t  fromWORD(const WORD bytes) {
        return *(uint16_t*)bytes;
    }

    static inline void  intoWORD(const WORD bytes, uint16_t val) {
        *(uint16_t*)bytes = val;
    }

    static inline uint32_t  fromDWORD(const DWORD bytes) {
        return *(uint32_t*)bytes;
    }

    static inline void  intoDWORD(const DWORD bytes, uint32_t val) {
        *(uint32_t*)bytes = val;
    }

#elif  __BIG_ENDIAN__
    static inline uint16_t  fromWORD(const WORD bytes) {
        return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
    }

    static inline void  intoWORD(const WORD bytes, uint16_t val) {
        *(uint16_t*)bytes = (val & 0xff) << 8 | (val >> 8);
    }

    static inline void  fromDWORD(const DWORD bytes) {
        return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16
             | (uint32_t)bytes[2] <<  8 | (uint32_t)bytes[3];
    }

    static inline void  intoDWORD(const DWORD bytes, uint32_t val) {
        *(uint32_t*)bytes = (val << 24) | (val & 0xff00) << 8
                          | (val & 0xff0000) >>  8 | (val >> 24);
    }

#else
#   error Unexpected endianness!
#endif

    // Total mem size allocated for this Bitmap object on the heap:
    uint32_t totalMemSize() const {
        return sizeof(Bitmap) + imgSize();
    }

    // Calculate the mem size required by the 'img' portion of a Bitmap having
    // all properties same as this's but different image dimensions:
    uint32_t  imgSizeFor(size_t imgWidth, size_t imgHeight) const {
        return imgHeight * ((bitsPerPixel() * imgWidth + 31) / 32 * 4);
    }

    // Calculate the total mem size required for a Bitmap object having
    // all properties same as this's but different image dimensions:
    uint32_t totalMemSizeFor(size_t imgWidth, size_t imgHeight) const {
        return sizeof(Bitmap) + imgSizeFor(imgWidth, imgHeight);
    }

    // *After* loading the bitmap headers structs from a file,
    // onLoad() will extract the important properties:
    void onLoad() {
        _imgOffset = fromDWORD(fileHdr.imgOffset);
        _hdrSize   = fromDWORD(fileHdr.hdrSize);

        if (_isV1Hdr) {
            _width  = fromWORD(v1InfoHdr.width);
            _height = fromWORD(v1InfoHdr.height);
            _bitsPerPixel = fromDWORD(v1InfoHdr.bitsPerPixel);
        }
        else {
            _width  = fromDWORD(infoHdr.width);
            _height = fromDWORD(infoHdr.height);
            _bitsPerPixel = fromDWORD(infoHdr.bitsPerPixel);
        }
    }

    // *Before* storing the bitmap into a file,
    // onStore() will transfer the (important) properties into the headers structs:
    void onStore() const {
        intoDWORD(fileHdr.imgOffset, 14U + _hdrSize);
        intoDWORD(fileHdr.hdrSize, _hdrSize);

        if (_isV1Hdr) {
            intoWORD(v1InfoHdr.width, _width);
            intoWORD(v1InfoHdr.height, _height);
            intoDWORD(v1InfoHdr.bitsPerPixel, _bitsPerPixel);
        }
        else {
            intoDWORD(infoHdr.width, _width);
            intoDWORD(infoHdr.height, _height);
            intoDWORD(infoHdr.bitsPerPixel, _bitsPerPixel);
            intoDWORD(infoHdr.imgSize, imgSize());
        }
    }

    void* operator new(size_t objSize, size_t totalMemSize) {
        assert(totalMemSize >= objSize);
        return new char[totalMemSize];
    }
    void  operator delete(void* mem) { delete[] (char*)mem; }

    Bitmap() {
        memset(this, 0, sizeof(Bitmap));
        DLOG("Hollow Bitmap created\n");
    }

    Bitmap(const Bitmap& bmp) {
        *this = bmp;
        DLOG("Bitmap copy created (W = " << imgWidth()
                             << ", H = " << imgHeight()
                             << ", totalMemSize=" << totalMemSize() << ")\n");
    }

    Bitmap(const Bitmap& bmp, size_t width, size_t height) {
        *this = bmp;
        _width = width;
        _height = height;
        DLOG("Bitmap resize created (W = " << imgWidth()
                               << ", H = " << imgHeight()
                               << ", totalMemSize=" << totalMemSize() << ")\n");
    }

   ~Bitmap() { DLOG("Bitmap destroyed\n"); }

    int bitsPerPixel()  const { return _bitsPerPixel; }
    int bytesPerPixel() const { return bitsPerPixel() >> 3; }
    int bytesPerLine()  const { return (bitsPerPixel() * imgWidth() + 31) / 32 * 4; }

    int imgOffset() const { return _imgOffset; }
    int imgSize()   const { return imgSizeFor(_width, _height); }
    int imgHeight() const { return _height; }
    int imgWidth()  const { return _width; }

    const uint8_t* imgData() const { return (const uint8_t*)this + sizeof(Bitmap); }
          uint8_t* imgData()       { return       (uint8_t*)this + sizeof(Bitmap); }

    uint32_t  pixel(int x, int y) const {
        assert(_bitsPerPixel == 24 || _bitsPerPixel == 32);
        const uint8_t* ppx = &imgData()[y * bytesPerLine() + x * bytesPerPixel()];
        DWORD px;
        px[3] = 0;
        memcpy(px, ppx, _bitsPerPixel / 8);
        return fromDWORD(px);
    }

    void  setPixel(int x, int y, uint32_t val) {
        assert(_bitsPerPixel == 24 || _bitsPerPixel == 32);
        uint8_t* ppx = &imgData()[y * bytesPerLine() + x * bytesPerPixel()];
        DWORD px;
        intoDWORD(px, val);
        memcpy(ppx, px, _bitsPerPixel / 8);
    }

    static uint32_t  avgPixels(uint32_t* pixels, size_t nPixels) {
        double l[4] = { .0, .0, .0, .0 };
        for (size_t i = 0; i < nPixels; ++i) {
            uint32_t val = pixels[i];
            l[0] += RGB_to_linear(val & 0xff);
            l[1] += RGB_to_linear(val >>  8 & 0xff);
            l[2] += RGB_to_linear(val >> 16 & 0xff);
            l[3] += RGB_to_linear(val >> 24 & 0xff);
            // TODO: in 'real world' the alpha channel should be 'averaged' in a diff way ...
        }
        uint32_t avg =
            linear_to_RGB(l[0] / nPixels)
          | linear_to_RGB(l[1] / nPixels) <<  8
          | linear_to_RGB(l[2] / nPixels) << 16
          | linear_to_RGB(l[3] / nPixels) << 24;
        return avg;
    }

    /*  Based on the Python sources found in http://www.ericbrasseur.org/gamma.html
     *  Note: As the domain of that function is [0..255], for performance reasons
     *  its range of values should better be precomputed
     *  thus the function evaluation will be reduced to a single array indexing.
     */
    static double RGB_to_linear(uint8_t val) {
        double s = val / 255.;
        if (s <= 0.04045)
            return s / 12.92;
        else
            return pow((s + 0.055) / (1 + 0.055), 2.4);
    }

    /* The reverse of RGB_to_linear (see the comment to the above function)
     * As the range of this function is [0..255], for performance reasons
     * it is worth experimenting with replacing this function with a search in
     * the sorted array used in the optimized implementation of RGB_to_linear.
     */
    static uint8_t  linear_to_RGB(double s) {
        double q;
        if (s <= 0.0031308)
            q = 12.92 * s;
        else
            q = (1 + 0.055) * pow(s, 1/2.4) - 0.055;
        assert(0. <= q);
        assert(q <= 1.);

        if (q < 0) return 0;
        if (q > 1) return 255;
        return (uint8_t)round(q * 255);
   }

#ifndef NDEBUG
    void  printHeaders(ostream& os) const;
    friend ostream& operator << (ostream& os, const Bitmap& bmp) {
               bmp.printHeaders(os);
               return os;
           }
#endif

public:
    typedef Bitmap* Type;

    // Load a bitmap from the specified file location.
    // May fail if the file open or file read operation fails.
    // Only 24 and 32 bits per pixel bitmap files are supported
    // If successful, caller should take care to Bitmap::Delete() the returned bitmap
    // when it is no longer needed.
    static Bitmap::Type  Load(const char* filepath);

    // Store a bitmap into the specified file location.
    // May fail if the file open or file write operation fails.
    static bool          Store(const Bitmap::Type bmp, const char* filepath);

    // Create a 50% scaled down version of the source bitmap.
    // Only 24 and 32 bits per pixel bitmap files are supported.
    // Caller should take care to Bitmap::Delete() the returned bitmap
    // when it is no longer needed.
    static Bitmap::Type  halfShrunkCopyOf(const Bitmap::Type src);

    // Safely dispose of a bitmap that is no longer needed.
    // Failing to do so will result in resource leak(s).
    static void          Delete(Bitmap::Type bmp) { delete bmp; }
};


static_assert(14 + 4 == sizeof(Bitmap::FileHeader), "Bitmap file header should be exactly 14 bytes!");


void  Bitmap::printHeaders(ostream& os) const
{
    assert((char*)&fileHdr.fileSize - (char*)&fileHdr.signature == 2);
    os << "BMP {"
    << "\n   FileHeader:"
       << "\n\tWORD   signature: " << hex << fromWORD(fileHdr.signature) <<dec
       << "\n\tDWORD  fileSize: " << fromDWORD(fileHdr.fileSize)
       << "\n\tDWORD  reserved: " << fromDWORD(fileHdr.reserved)
       << "\n\tDWORD  imgOffset: " << fromDWORD(fileHdr.imgOffset)
    << "\n   InfoHeader:"
       << "\n\tDWORD  hdrSize: " << fromDWORD(fileHdr.hdrSize)
       << "\n\tDWORD  width: " << fromDWORD(infoHdr.width)
       << "\n\tDWORD  height: " << fromDWORD(infoHdr.height)
       << "\n\tWORD   numPlanes: " << fromWORD(infoHdr.numPlanes)
       << "\n\tWORD   bitsPerPixel: " << fromWORD(infoHdr.bitsPerPixel)
       << "\n\tDWORD  compression: " << fromDWORD(infoHdr.compression)
       << "\n\tDWORD  imgSize: " << fromDWORD(infoHdr.imgSize)
       << "\n\tDWORD  xResolution: " << fromDWORD(infoHdr.xResolution)
       << "\n\tDWORD  yResolution: " << fromDWORD(infoHdr.yResolution)
       << "\n\tDWORD  colorsUsed: " << fromDWORD(infoHdr.colorsUsed)
       << "\n\tDWORD  colorsImportant: " << fromDWORD(infoHdr.colorsImportant)
       << "\n}\n";
}


Bitmap::Type  Bitmap::Load(const char* filepath)
{
	ifstream file(filepath, ios::binary);
	if(!file) {
		cerr << "Failed to open file '" << filepath << "'\n";
		return nullptr;
	}

	Bitmap bmp;

	file.read((char*)&bmp.fileHdr, sizeof(bmp.fileHdr));

	if(fromWORD(bmp.fileHdr.signature) != 0x4D42) {
		cerr << "File '" << filepath << "' isn't a bitmap file\n";
		return nullptr;
	}

	switch (bmp._hdrSize = fromDWORD(bmp.fileHdr.hdrSize)) {
        case 40:
        case 64:
            bmp._isV1Hdr = false;
            file.read((char*)&bmp.infoHdr, sizeof(bmp.infoHdr));
            break;
        default:
            cerr << "Unknown BMP format variant (hdrSize=" << bmp._hdrSize << ")\n";
            return nullptr;
	}
	bmp.onLoad();
	DLOG("Loaded " << bmp << " from file path=" << filepath << "\n");

	// Allocate the resultant Bitmap object on the heap:
	Bitmap* res = new (bmp.totalMemSize()) Bitmap(bmp);

	// Go to where image data starts and read it:
	DLOG("reading image data (offset=" << res->imgOffset()
	                      << ", size=" << res->imgSize() << ") ... ");
	file.seekg(res->imgOffset());
	file.read((char*)res->imgData(), res->imgSize());

#ifndef NDEBUG
    // Check if there's some unused data left in the file after image data:
	streampos readEnd = file.tellg();
	file.seekg(0, ios_base::end);
	streampos fileEnd = file.tellg();
	DLOG((fileEnd - readEnd) << " bytes of data unused at end of file\n");
#endif

    return res;
}


bool Bitmap::Store(const Bitmap::Type bmp, const char* filepath)
{
	ofstream file(filepath, ios::binary);
	if(!file) {
		cerr << "Failed to write to file '" << filepath << "'\n";
		return false;
	}

	bmp->onStore();
    DLOG("Storing " << *bmp << " into file path=" << filepath << "\n");
    assert(!bmp->_isV1Hdr);
	file.write((char*)&bmp->fileHdr, sizeof(bmp->fileHdr));
	file.write((char*)&bmp->infoHdr, sizeof(bmp->infoHdr));
	file.write((char*)bmp->imgData(), bmp->imgSize());
    file.close();
    return !file.fail();
}


Bitmap::Type  Bitmap::halfShrunkCopyOf(const Bitmap::Type src)
{
    const size_t W = src->imgWidth();
    const size_t H = src->imgHeight();
    Bitmap* res = new (src->totalMemSizeFor(W/2, H/2)) Bitmap(*src, W/2, H/2);
    for (size_t h=0; h < H; h += 2) {
        for (size_t w=0; w < W; w += 2) {
            uint32_t p[] = { src->pixel(w, h),     src->pixel(w + 1, h),
                             src->pixel(w, h + 1), src->pixel(w + 1, h + 1) };
            res->setPixel(w/2, h/2, avgPixels(p, sizeof(p)/sizeof(p[0])));
        }
    }
    return res;
}


int  main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "Program should be invoked like this:\n\t" << argv[0] << " <path-to-src-bitmap>\n";
        return -1;
    }

    Bitmap::Type srcBmp = Bitmap::Load(argv[1]);
    if (!srcBmp) // hehe - illegal ! here, as Bitmap::Type is supposedly opaque type ...
        return -2;

    Bitmap::Type halfBmp = Bitmap::halfShrunkCopyOf(srcBmp);

    bool ok = Bitmap::Store(halfBmp, (string(argv[1]) + ".half.bmp").c_str());

    Bitmap::Delete(halfBmp);
    Bitmap::Delete(srcBmp);

    return ok ? 0 : -3;
}