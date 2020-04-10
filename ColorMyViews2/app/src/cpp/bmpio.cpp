//
// Created by Stanislav Jordanov on 9.04.20.
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
    };

    struct InfoHeader {
      DWORD  biSize;
      DWORD  biWidth;
      DWORD  biHeight;
      WORD   biPlanes;
      WORD   biBitCount;
      DWORD  biCompression;
      DWORD  biImgSize;
      DWORD  biXPelsPerMeter;
      DWORD  biYPelsPerMeter;
      DWORD  biClrUsed;
      DWORD  biClrImportant;
    };

private:
    FileHeader fileHdr;
    InfoHeader infoHdr;

    static inline uint16_t  toWORD(const WORD bytes) {
#ifdef __LITTLE_ENDIAN__
        return *(uint16_t*)bytes;
#elif  __BIG_ENDIAN__
        return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
#else
#   error Unexpected endianness!
#endif
    }

    static inline uint32_t  toDWORD(const DWORD bytes) {
#ifdef __LITTLE_ENDIAN__
        return *(uint32_t*)bytes;
#elif  __BIG_ENDIAN__
        return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16
             | (uint32_t)bytes[2] <<  8 | (uint32_t)bytes[3];
#else
#   error Unexpected endianness!
#endif
    }

    uint32_t totalMemSize() const {
        return toDWORD(fileHdr.imgOffset) + toDWORD(infoHdr.biImgSize);
    }

    uint32_t imgOffset() const {
        assert(toDWORD(fileHdr.imgOffset) >= sizeof(Bitmap));
        return toDWORD(fileHdr.imgOffset);
    }

public:
    void* operator new(size_t objSize, const Bitmap& bmp) {
        assert(bmp.totalMemSize() >= objSize);
        return new char[bmp.totalMemSize()];
    }
    void  operator delete(void* mem) { delete[] (char*)mem; }

    Bitmap() {
        memset(this, 0, sizeof(Bitmap));
        DLOG("Hollow Bitmap created\n");
    }
    Bitmap(const Bitmap& bmp) {
        *this = bmp;
        DLOG("Bitmap copy created (totalMemSize=" << totalMemSize() << ")\n");
    }

   ~Bitmap() { DLOG("Bitmap destroyed\n"); }

    int imgHeight() const { return toDWORD(infoHdr.biHeight); }
    int imgWidth()  const { return toDWORD(infoHdr.biWidth); }

    int bitsPerPixel()  const { return toDWORD(infoHdr.biBitCount); }
    int bytesPerPixel() const { return bitsPerPixel() >> 3; }
    int bytesPerLine()  const { return (bitsPerPixel() * imgWidth() + 31) / 32 * 4; }

    const uint8_t* imgData() const { return (const uint8_t*)this + imgOffset(); }
          uint8_t* imgData()       { return       (uint8_t*)this + imgOffset(); }
          size_t   imgSize() const { return toDWORD(infoHdr.biImgSize); }

    uint32_t  pixel(int x, int y) const {
        //TODO
        return 0;
    }

    void  setPixel(int x, int y, uint32_t val) {
        //TODO
    }

    void  darken() {
        for (size_t i=0; i < imgSize(); ++i) {
            //imgData()[i] = (imgData()[i] > 50 ? imgData()[i] - 50 : 0);
            imgData()[i] = imgData()[i] * 4 / 5;
        }
    }

    Bitmap* shrinkTwice() {
        Bitmap* res; //TODO initialize properly to (w/2, h/2)
        for (int h=0; h < imgHeight(); h += 2) {
            for (int w=0; w < imgWidth(); w += 2) {
                uint32_t p[] = { pixel(w, h),     pixel(w + 1, h),
                                 pixel(w, h + 1), pixel(w + 1, h + 1) };
                res->setPixel(w/2, h/2, avgPixels(p, sizeof(p)/sizeof(p[0])));
            }
        }
        return res;
    }

    uint32_t  avgPixels(uint32_t* pixels, size_t nPixels) {
        double l[4] = { .0, .0, .0, .0 };
        for (size_t i = 0; i < nPixels; ++i) {
            uint32_t val = pixels[i];
            l[0] += RGB_to_linear(val & 0xff);
            l[1] += RGB_to_linear(val >>  8 & 0xff);
            l[2] += RGB_to_linear(val >> 16 & 0xff);
            l[3] += RGB_to_linear(val >> 24 & 0xff);
        }
        uint32_t avg =
            linear_to_RGB(l[0] / nPixels)
          | linear_to_RGB(l[1] / nPixels) <<  8
          | linear_to_RGB(l[2] / nPixels) << 16
          | linear_to_RGB(l[3] / nPixels) << 24;
        return avg;
    }

    double RGB_to_linear(uint8_t val) {
        double s = val / 255.;
        if (s <= 0.04045)
            return s / 12.92;
        else
            return pow((s + 0.055) / (1 + 0.055), 2.4);
    }

    uint8_t  linear_to_RGB(double s) {
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

    static Bitmap*  Load(const char* filepath);
    static bool     Store(const Bitmap* bmp, const char* filepath);

    void  printHeaders(ostream& os) const;
};


static_assert(14 == sizeof(Bitmap::FileHeader), "Bitmap file header should be exactly 14 bytes!");

static_assert(sizeof(Bitmap) == sizeof(Bitmap::FileHeader) + sizeof(Bitmap::InfoHeader),
    "Either someone has added a member to the Bitmap class"
    " or the compiler aligns struct members on > 2bytes boundary");


ostream& operator << (ostream& os, const Bitmap& bmp)
{
    bmp.printHeaders(os);
    return os;
}


void  Bitmap::printHeaders(ostream& os) const
{
    assert((char*)&fileHdr.fileSize - (char*)&fileHdr.signature == 2);
    os << "BMP {"
    << "\n   FileHeader:"
       << "\n\tWORD   signature: " << hex << toWORD(fileHdr.signature) <<dec
       << "\n\tDWORD  fileSize: " << toDWORD(fileHdr.fileSize)
       << "\n\tDWORD  reserved: " << toDWORD(fileHdr.reserved)
       << "\n\tDWORD  imgOffset: " << toDWORD(fileHdr.imgOffset)
    << "\n   InfoHeader:"
       << "\n\tDWORD  biSize: " << toDWORD(infoHdr.biSize)
       << "\n\tDWORD  biWidth: " << toDWORD(infoHdr.biWidth)
       << "\n\tDWORD  biHeight: " << toDWORD(infoHdr.biHeight)
       << "\n\tWORD   biPlanes: " << toWORD(infoHdr.biPlanes)
       << "\n\tWORD   biBitCount: " << toWORD(infoHdr.biBitCount)
       << "\n\tDWORD  biCompression: " << toDWORD(infoHdr.biCompression)
       << "\n\tDWORD  biImgSize: " << toDWORD(infoHdr.biImgSize)
       << "\n\tDWORD  biXPelsPerMeter: " << toDWORD(infoHdr.biXPelsPerMeter)
       << "\n\tDWORD  biYPelsPerMeter: " << toDWORD(infoHdr.biYPelsPerMeter)
       << "\n\tDWORD  biClrUsed: " << toDWORD(infoHdr.biClrUsed)
       << "\n\tDWORD  biClrImportant: " << toDWORD(infoHdr.biClrImportant)
       << "\n}\n";
}


Bitmap* Bitmap::Load(const char* filepath)
{
	ifstream file(filepath, ios::binary);
	if(!file) {
		cerr << "Failed to open file '" << filepath << "'\n";
		return nullptr;
	}

	Bitmap bmp;

	file.read((char*)&bmp, sizeof(Bitmap));
	DLOG(bmp);

	if(toWORD(bmp.fileHdr.signature) != 0x4D42) {
		cerr << "File '" << filepath << "' isn't a bitmap file\n";
		return nullptr;
	}

	// Allocate the resultant Bitmap object on the heap:
	Bitmap* res = new (bmp) Bitmap(bmp);

	// Go to where image data starts and read it:
	DLOG("reading image data (offset=" << res->imgOffset()
	                      << ", size=" << res->imgSize() << ") ... ");
	file.seekg(res->imgOffset());
	file.read((char*)res->imgData(), res->imgSize());

    // Check if there's some unused data left in the file after image data:
	streampos readEnd = file.tellg();
	file.seekg(0, ios_base::end);
	streampos fileEnd = file.tellg();
	DLOG((fileEnd - readEnd) << " bytes of data unused at end of file\n");

    return res;
}


bool Bitmap::Store(const Bitmap* bmp, const char* filepath)
{
	ofstream file(filepath, ios::binary);
	if(!file) {
		cerr << "Failed to write to file '" << filepath << "'\n";
		return false;
	}

    DLOG("Storing bmp '" << filepath << "' of size " << bmp->totalMemSize() << "\n");
	file.write((char*)bmp, bmp->totalMemSize());
    file.close();
    return !file.fail();
}


int  main(int argc, char* argv[])
{
    assert(argc == 2);
    Bitmap* bmp = Bitmap::Load(argv[1]);
    //bmp->darken();
    bool ok = Bitmap::Store(bmp, (string(argv[1]) + ".copy.bmp").c_str());
    assert(ok);
    delete bmp;
}