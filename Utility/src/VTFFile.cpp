#include "Logging.h"
#include "VTFDXTn.h"
#include "VTFFile.h"

// Array which holds information about our image format
// (taken from imageloader.cpp, Valve Source SDK)
//------------------------------------------------------
static VTFImageFormatInfo ImageFormatInfo[] =
{
	{ "RGBA8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_RGBA8888,
	{ "ABGR8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_ABGR8888, 
	{ "RGB888",				 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_RGB888,
	{ "BGR888",				 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGR888,
	{ "RGB565",				 16,  2,  5,  6,  5,  0, false,  true },		// IMAGE_FORMAT_RGB565, 
	{ "I8",					  8,  1,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_I8,
	{ "IA88",				 16,  2,  0,  0,  0,  8, false,  true },		// IMAGE_FORMAT_IA88
	{ "P8",					  8,  1,  0,  0,  0,  0, false, false },		// IMAGE_FORMAT_P8
	{ "A8",					  8,  1,  0,  0,  0,  8, false,  true },		// IMAGE_FORMAT_A8
	{ "RGB888 Bluescreen",	 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_RGB888_BLUESCREEN
	{ "BGR888 Bluescreen",	 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGR888_BLUESCREEN
	{ "ARGB8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_ARGB8888
	{ "BGRA8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_BGRA8888
	{ "DXT1",				  4,  0,  0,  0,  0,  0,  true,  true },		// IMAGE_FORMAT_DXT1
	{ "DXT3",				  8,  0,  0,  0,  0,  8,  true,  true },		// IMAGE_FORMAT_DXT3
	{ "DXT5",				  8,  0,  0,  0,  0,  8,  true,  true },		// IMAGE_FORMAT_DXT5
	{ "BGRX8888",			 32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGRX8888
	{ "BGR565",				 16,  2,  5,  6,  5,  0, false,  true },		// IMAGE_FORMAT_BGR565
	{ "BGRX5551",			 16,  2,  5,  5,  5,  0, false,  true },		// IMAGE_FORMAT_BGRX5551
	{ "BGRA4444",			 16,  2,  4,  4,  4,  4, false,  true },		// IMAGE_FORMAT_BGRA4444
	{ "DXT1 One Bit Alpha",	  4,  0,  0,  0,  0,  1,  true,  true },		// IMAGE_FORMAT_DXT1_ONEBITALPHA
	{ "BGRA5551",			 16,  2,  5,  5,  5,  1, false,  true },		// IMAGE_FORMAT_BGRA5551
	{ "UV88",				 16,  2,  8,  8,  0,  0, false,  true },		// IMAGE_FORMAT_UV88
	{ "UVWQ8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_UVWQ8899
	{ "RGBA16161616F",	     64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_RGBA16161616F
	{ "RGBA16161616",	     64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_RGBA16161616
	{ "UVLX8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_UVLX8888
	{ "R32F",				 32,  4, 32,  0,  0,  0, false,  true },		// IMAGE_FORMAT_R32F
	{ "RGB323232F",			 96, 12, 32, 32, 32,  0, false,  true },		// IMAGE_FORMAT_RGB323232F
	{ "RGBA32323232F",		128, 16, 32, 32, 32, 32, false,  true },		// IMAGE_FORMAT_RGBA32323232F
	{ "nVidia DST16",		 16,  2,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_DST16
	{ "nVidia DST24",		 24,  3,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_DST24
	{ "nVidia INTZ",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_INTZ
	{ "nVidia RAWZ",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_RAWZ
	{ "ATI DST16",			 16,  2,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_ATI_DST16
	{ "ATI DST24",			 24,  3,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_ATI_DST24
	{ "nVidia NULL",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_NULL
	{ "ATI1N",				  4,  0,  0,  0,  0,  0,  true,  true },		// IMAGE_FORMAT_ATI1N
	{ "ATI2N",				  8,  0,  0,  0,  0,  0,  true,  true }/*,		// IMAGE_FORMAT_ATI2N
	{ "Xbox360 DST16",		 16,  0,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_X360_DST16
	{ "Xbox360 DST24",		 24,  0,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_X360_DST24
	{ "Xbox360 DST24F",		 24,  0,  0,  0,  0,  0, false , true },		// IMAGE_FORMAT_X360_DST24F
	{ "Linear BGRX8888",	 32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGRX8888
	{ "Linear RGBA8888",     32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_RGBA8888
	{ "Linear ABGR8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_ABGR8888
	{ "Linear ARGB8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_ARGB8888
	{ "Linear BGRA8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_BGRA8888
	{ "Linear RGB888",		 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_RGB888
	{ "Linear BGR888",		 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGR888
	{ "Linear BGRX5551",	 16,  2,  5,  5,  5,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGRX5551
	{ "Linear I8",			  8,  1,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_LINEAR_I8
	{ "Linear RGBA16161616", 64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_LINEAR_RGBA16161616
	{ "LE BGRX8888",         32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LE_BGRX8888
	{ "LE BGRA8888",		 32,  4,  8,  8,  8,  8, false,  true }*/		// IMAGE_FORMAT_LE_BGRA8888
};

typedef void (*TransformProc)(unsigned __int16& R, unsigned __int16& G, unsigned __int16& B, unsigned __int16& A);

struct VTFImageConvertInfo
{
	int BitsPerPixel;			// Format bytes per pixel.
	int BytesPerPixel;		// Format bytes per pixel.
	int RBitsPerPixel;		// Format conversion red bits per pixel.  0 for N/A.
	int GBitsPerPixel;		// Format conversion green bits per pixel.  0 for N/A.
	int BBitsPerPixel;		// Format conversion blue bits per pixel.  0 for N/A.
	int ABitsPerPixel;		// Format conversion alpha bits per pixel.  0 for N/A.
	int R;						// "Red" index.
	int G;						// "Green" index.
	int B;						// "Blue" index.
	int A;						// "Alpha" index.
	bool bIsCompressed;			// Format is compressed (DXT).
	bool bIsSupported;			// Format is supported by VTFLib.
	TransformProc pToTransform;		// Custom transform to function.
	TransformProc pFromTransform;	// Custom transform from function.
	ImageFormat Format;
};

static VTFImageConvertInfo ImageConvertInfo[] =
{
	{	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3,	false,  true,	0,	0,		IMAGE_FORMAT_RGBA8888},
	{	 32,  4,  8,  8,  8,  8,	 3,	 2,	 1,	 0, false,  true,	0,	0,		IMAGE_FORMAT_ABGR8888},
	{	 24,  3,  8,  8,  8,  0,	 0,	 1,	 2,	-1, false,  true,	0,	0,		IMAGE_FORMAT_RGB888},
	{	 24,  3,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_BGR888},
	{	 16,  2,  5,  6,  5,  0,	 0,	 1,	 2,	-1, false,  true,	0,	0,		IMAGE_FORMAT_RGB565},
	//{	  8,  1,  8,  8,  8,  0,	 0,	-1,	-1,	-1, false,  true,	ToLuminance,	FromLuminance,	IMAGE_FORMAT_I8},
	{	  8,  1,  8,  8,  8,  0,	 0,	-1,	-1,	-1, false,  false,	0,	0,		IMAGE_FORMAT_I8},
	//{	 16,  2,  8,  8,  8,  8,	 0,	-1,	-1,	 1, false,  true,	ToLuminance,	FromLuminance,	IMAGE_FORMAT_IA88},
	{	 16,  2,  8,  8,  8,  8,	 0,	-1,	-1,	 1, false,  false,	0,	0,		IMAGE_FORMAT_IA88},
	{	  8,  1,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false,	0,	0,		IMAGE_FORMAT_P8},
	{ 	  8,  1,  0,  0,  0,  8,	-1,	-1,	-1,	 0, false,  true,	0,	0,		IMAGE_FORMAT_A8},
	//{ 	 24,  3,  8,  8,  8,  8,	 0,	 1,	 2,	-1, false,  true,	ToBlueScreen,	FromBlueScreen,	IMAGE_FORMAT_RGB888_BLUESCREEN},
	{ 	 24,  3,  8,  8,  8,  8,	 0,	 1,	 2,	-1, false,  false,	0,	0,		IMAGE_FORMAT_RGB888_BLUESCREEN},
	//{ 	 24,  3,  8,  8,  8,  8,	 2,	 1,	 0,	-1, false,  true,	ToBlueScreen,	FromBlueScreen,	IMAGE_FORMAT_BGR888_BLUESCREEN},
	{ 	 24,  3,  8,  8,  8,  8,	 2,	 1,	 0,	-1, false,  false,	0,	0,		IMAGE_FORMAT_BGR888_BLUESCREEN},
	{ 	 32,  4,  8,  8,  8,  8,	 3,	 0,	 1,	 2, false,  true,	0,	0,		IMAGE_FORMAT_ARGB8888},
	{ 	 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	 3, false,  true,	0,	0,		IMAGE_FORMAT_BGRA8888},
	{ 	  4,  0,  0,  0,  0,  0,	-1,	-1,	-1,	-1,  true,  true,	0,	0,		IMAGE_FORMAT_DXT1},
	{ 	  8,  0,  0,  0,  0,  8,	-1,	-1,	-1,	-1,  true,  true,	0,	0,		IMAGE_FORMAT_DXT3},
	{ 	  8,  0,  0,  0,  0,  8,	-1,	-1,	-1,	-1,  true,  true,	0,	0,		IMAGE_FORMAT_DXT5},
	{ 	 32,  4,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_BGRX8888},
	{ 	 16,  2,  5,  6,  5,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_BGR565},
	{ 	 16,  2,  5,  5,  5,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_BGRX5551},
	{ 	 16,  2,  4,  4,  4,  4,	 2,	 1,	 0,	 3, false,  true,	0,	0,		IMAGE_FORMAT_BGRA4444},
	{ 	  4,  0,  0,  0,  0,  1,	-1,	-1,	-1,	-1,  true,  true,	0,	0,		IMAGE_FORMAT_DXT1_ONEBITALPHA},
	{ 	 16,  2,  5,  5,  5,  1,	 2,	 1,	 0,	 3, false,  true,	0,	0,		IMAGE_FORMAT_BGRA5551},
	{ 	 16,  2,  8,  8,  0,  0,	 0,	 1,	-1,	-1, false,  true,	0,	0,		IMAGE_FORMAT_UV88},
	{ 	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3, false,  true,	0,	0,		IMAGE_FORMAT_UVWQ8888},
	//{    64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true,	ToFP16,	FromFP16,	IMAGE_FORMAT_RGBA16161616F},
	{    64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  false,	0,	0,		IMAGE_FORMAT_RGBA16161616F},
	{	 64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true,	0,	0,		IMAGE_FORMAT_RGBA16161616},
	{ 	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3, false,  true,	0,	0,		IMAGE_FORMAT_UVLX8888},
	{ 	 32,  4, 32,  0,  0,  0,	 0,	-1,	-1,	-1, false, false,	0,	0,		IMAGE_FORMAT_R32F},
	{ 	 96, 12, 32, 32, 32,  0,	 0,	 1,	 2,	-1, false, false,	0,	0,		IMAGE_FORMAT_RGB323232F},
	{	128, 16, 32, 32, 32, 32,	 0,	 1,	 2,	 3, false, false,	0,	0,		IMAGE_FORMAT_RGBA32323232F},
	{    16,  2, 16,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true,	0,	0,		IMAGE_FORMAT_NV_DST16},
	{	 24,  3, 24,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true,	0,	0,		IMAGE_FORMAT_NV_DST24},
	{	 32,  4,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false,	0,	0,		IMAGE_FORMAT_NV_INTZ},
	{	 24,  3,  0,  0,  0,  0,    -1,	-1,	-1,	-1, false, false,	0,	0,		IMAGE_FORMAT_NV_RAWZ},
	{	 16,  2, 16,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true,	0,	0,		IMAGE_FORMAT_ATI_DST16},
	{	 24,  3, 24,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true,	0,	0,		IMAGE_FORMAT_ATI_DST24},
	{	 32,  4,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false,	0,	0,		IMAGE_FORMAT_NV_NULL},
	{	  4,  0,  0,  0,  0,  0,	-1, -1, -1, -1,  true, false,	0,	0,		IMAGE_FORMAT_ATI1N},
	{     8,  0,  0,  0,  0,  0,	-1, -1, -1, -1,  true, false,	0,	0,		IMAGE_FORMAT_ATI2N}/*,
	{	 16,  2, 16,  0,  0,  0,	 0, -1, -1, -1, false,  true,	0,	0,		IMAGE_FORMAT_X360_DST16},
	{	 24,  3, 24,  0,  0,  0,	 0, -1, -1, -1, false,  true,	0,	0,		IMAGE_FORMAT_X360_DST24},
	{	 24,  3,  0,  0,  0,  0,	-1, -1, -1, -1, false, false,	0,	0,		IMAGE_FORMAT_X360_DST24F},
	{ 	 32,  4,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_BGRX8888},
	{	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3,	false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_RGBA8888},
	{	 32,  4,  8,  8,  8,  8,	 3,	 2,	 1,	 0, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_ABGR8888},
	{ 	 32,  4,  8,  8,  8,  8,	 3,	 0,	 1,	 2, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_ARGB8888},
	{ 	 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	 3, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_BGRA8888},
	{	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	-1,	false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_RGB888},
	{	 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	-1,	false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_BGR888},
	{ 	 16,  2,  5,  5,  5,  0,	 2,	 1,	 0,	-1, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_BGRX5551},
	{	  8,  1,  8,  8,  8,  0,	 0,	-1,	-1,	-1, false,  true,	ToLuminance,	FromLuminance,	IMAGE_FORMAT_LINEAR_I8},	
	{	 64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true,	0,	0,		IMAGE_FORMAT_LINEAR_RGBA16161616}*/
};


VTFFile::VTFFile() : 
	m_pBuffer( 0 ),
	m_nBufferSize( 0 ),
	m_pHeader( 0 ),
	m_pLowResData( 0 ),
	m_pHighResData( 0 )
{
}

VTFFile::~VTFFile()
{
	if( m_pHeader )
	{
		delete m_pHeader;
	}
}

void VTFFile::InitializeBuffers( int Size )
{
	m_pBuffer = new unsigned char[Size];
	m_nBufferSize = Size;

	// Header data points to the start of the buffer
	m_pHeader = (VTFHeader *)m_pBuffer;
}

//
// GetDepth()
// Gets the depth of the largest level mipmap.
//
int VTFFile::GetDepth() const
{
	return 1;
}

//
// GetFrameCount()
// Gets the number of frames the image has.  All images have at least 1 frame.
//
int VTFFile::GetFrameCount() const
{
	return GetVTFHeader()->frames;
}

//---------------------------------------------------------------------------------
// GetFaceCount()
//
// Returns the number of faces in the texture based on the status of the header
// flags. Cubemaps have 6 or 7 faces, others just 1.
//---------------------------------------------------------------------------------
int VTFFile::GetFaceCount() const
{
	if( GetVTFHeader()->flags & TEXTUREFLAGS_ENVMAP )
	{
		if( GetVTFHeader()->firstFrame != 0xffff )
		{
			return CUBEMAP_FACE_COUNT;
		}

		return CUBEMAP_FACE_COUNT - 1;
	}

	return 1;
}

// GetMipmapCount()
// Gets the number of mipmaps the image has.
//
int VTFFile::GetMipmapCount() const
{
	return GetVTFHeader()->numMipLevels;
}

// Get each channels shift and mask (for encoding and decoding).
template<typename T>
void GetShiftAndMask(const VTFImageConvertInfo& Info, T &RShift, T &GShift, T &BShift, T &AShift, T &RMask, T &GMask, T &BMask, T &AMask)
{
	if(Info.R >= 0)
	{
		if(Info.G >= 0 && Info.G < Info.R)
			RShift += (T)Info.GBitsPerPixel;

		if(Info.B >= 0 && Info.B < Info.R)
			RShift += (T)Info.BBitsPerPixel;

		if(Info.A >= 0 && Info.A < Info.R)
			RShift += (T)Info.ABitsPerPixel;

		RMask = (T)(~0) >> (T)((sizeof(T) * 8) - Info.RBitsPerPixel); // Mask is for down shifted values.
	}

	if(Info.G >= 0)
	{
		if(Info.R >= 0 && Info.R < Info.G)
			GShift += (T)Info.RBitsPerPixel;

		if(Info.B >= 0 && Info.B < Info.G)
			GShift += (T)Info.BBitsPerPixel;

		if(Info.A >= 0 && Info.A < Info.G)
			GShift += (T)Info.ABitsPerPixel;

		GMask = (T)(~0) >> (T)((sizeof(T) * 8) - Info.GBitsPerPixel);
	}

	if(Info.B >= 0)
	{
		if(Info.R >= 0 && Info.R < Info.B)
			BShift += (T)Info.RBitsPerPixel;

		if(Info.G >= 0 && Info.G < Info.B)
			BShift += (T)Info.GBitsPerPixel;

		if(Info.A >= 0 && Info.A < Info.B)
			BShift += (T)Info.ABitsPerPixel;

		BMask = (T)(~0) >> (T)((sizeof(T) * 8) - Info.BBitsPerPixel);
	}

	if(Info.A >= 0)
	{
		if(Info.R >= 0 && Info.R < Info.A)
			AShift += (T)Info.RBitsPerPixel;

		if(Info.G >= 0 && Info.G < Info.A)
			AShift += (T)Info.GBitsPerPixel;

		if(Info.B >= 0 && Info.B < Info.A)
			AShift += (T)Info.BBitsPerPixel;

		AMask = (T)(~0) >> (T)((sizeof(T) * 8) - Info.ABitsPerPixel);
	}
}

// Downsample a channel.
template<typename T>
T Shrink(T S, T SourceBits, T DestBits)
{
	if(SourceBits == 0 || DestBits == 0)
		return 0;

	return S >> (SourceBits - DestBits);
}

// Upsample a channel.
template<typename T>
T Expand(T S, T SourceBits, T DestBits)
{
	if(SourceBits == 0 || DestBits == 0)
		return 0;

	T D = 0;

	// Repeat source bit pattern as much as possible.
	while(DestBits >= SourceBits)
	{
		D <<= SourceBits;
		D |= S;
		DestBits -= SourceBits;
	}

	// Add most significant part of source bit pattern to least significant part of dest bit pattern.
	if(DestBits)
	{
		S >>= SourceBits - DestBits;
		D <<= DestBits;
		D |= S;
	}

	return D;
}

// Run custom transformation functions.
template<typename T, typename U> void Transform( TransformProc pTransform1, TransformProc pTransform2, T SR, T SG, T SB, T SA, T SRBits, T SGBits, T SBBits, T SABits, U& DR, U& DG, U& DB, U& DA, U DRBits, U DGBits, U DBBits, U DABits )
{
	unsigned __int16 TR, TG, TB, TA;

	// Expand from source to 16 bits for transform functions.
	SRBits && SRBits < 16 ? TR = (unsigned __int16)Expand<T>(SR, SRBits, 16) : TR = (unsigned __int16)SR;
	SGBits && SGBits < 16 ? TG = (unsigned __int16)Expand<T>(SG, SGBits, 16) : TG = (unsigned __int16)SG;
	SBBits && SBBits < 16 ? TB = (unsigned __int16)Expand<T>(SB, SBBits, 16) : TB = (unsigned __int16)SB;
	SABits && SABits < 16 ? TA = (unsigned __int16)Expand<T>(SA, SABits, 16) : TA = (unsigned __int16)SA;

	// Source transform then dest transform.
	if(pTransform1)
		pTransform1(TR, TG, TB, TA);
	if(pTransform2)
		pTransform2(TR, TG, TB, TA);

	// Shrink to dest from 16 bits.
	DRBits && DRBits < 16 ? DR = (U)Shrink<unsigned __int16>(TR, 16, (unsigned __int16)DRBits) : DR = (U)TR;
	DGBits && DGBits < 16 ? DG = (U)Shrink<unsigned __int16>(TG, 16, (unsigned __int16)DGBits) : DG = (U)TG;
	DBBits && DBBits < 16 ? DB = (U)Shrink<unsigned __int16>(TB, 16, (unsigned __int16)DBBits) : DB = (U)TB;
	DABits && DABits < 16 ? DA = (U)Shrink<unsigned __int16>(TA, 16, (unsigned __int16)DABits) : DA = (U)TA;
}

float sHDRLogAverageLuminance;

// Convert source to dest using required storage requirments (hence the template).
template<typename T, typename U>
bool ConvertTemplated( unsigned char* Source, unsigned char* Dest, int Width, int Height, const VTFImageConvertInfo& SourceInfo, const VTFImageConvertInfo& DestInfo )
{
	unsigned __int16 SourceRShift = 0, SourceGShift = 0, SourceBShift = 0, SourceAShift = 0;
	unsigned __int16 SourceRMask = 0, SourceGMask = 0, SourceBMask = 0, SourceAMask = 0;

	unsigned __int16 DestRShift = 0, DestGShift = 0, DestBShift = 0, DestAShift = 0;
	unsigned __int16 DestRMask = 0, DestGMask = 0, DestBMask = 0, DestAMask = 0;

	GetShiftAndMask<unsigned __int16>( SourceInfo, SourceRShift, SourceGShift, SourceBShift, SourceAShift, SourceRMask, SourceGMask, SourceBMask, SourceAMask );
	GetShiftAndMask<unsigned __int16>( DestInfo, DestRShift, DestGShift, DestBShift, DestAShift, DestRMask, DestGMask, DestBMask, DestAMask );

	// If we are in the FP16 HDR format we will need a log average.
	if( SourceInfo.Format == IMAGE_FORMAT_RGBA16161616F )
	{
		unsigned char* pFPSource = Source;

		sHDRLogAverageLuminance = 0.0f;

		unsigned char *pFPSourceEnd = pFPSource + (Width * Height * SourceInfo.BytesPerPixel );
		for(; pFPSource < pFPSourceEnd; pFPSource += SourceInfo.BytesPerPixel )
		{
			unsigned __int16* p = (unsigned __int16*)pFPSource;

			float sLuminance = (float)p[0] * 0.299f + (float)p[1] * 0.587f + (float)p[2] * 0.114f;

			sHDRLogAverageLuminance += log(0.0000000001f + sLuminance);
		}

		sHDRLogAverageLuminance = exp(sHDRLogAverageLuminance / (float)(Width * Height));
	}

	unsigned char* pSourceEnd = Source + (Width * Height * SourceInfo.BytesPerPixel);
	for(; Source < pSourceEnd; Source += SourceInfo.BytesPerPixel, Dest += DestInfo.BytesPerPixel)
	{
		// read source into single variable
		int i;
		T TempSource = 0;
		for(i = 0; i < SourceInfo.BytesPerPixel; i++)
		{
			TempSource |= (T)Source[i] << ((T)i * 8);
		}

		unsigned __int16 SR = 0, SG = 0, SB = 0, SA = ~0;
		unsigned __int16 DR = 0, DG = 0, DB = 0, DA = ~0;	// default values

		// read source values
		if(SourceRMask)
			SR = (unsigned __int16)(TempSource >> (T)SourceRShift) & SourceRMask;	// isolate R channel

		if(SourceGMask)
			SG = (unsigned __int16)(TempSource >> (T)SourceGShift) & SourceGMask;	// isolate G channel

		if(SourceBMask)
			SB = (unsigned __int16)(TempSource >> (T)SourceBShift) & SourceBMask;	// isolate B channel

		if(SourceAMask)
			SA = (unsigned __int16)(TempSource >> (T)SourceAShift) & SourceAMask;	// isolate A channel

		if(SourceInfo.pFromTransform || DestInfo.pToTransform)
		{
			// transform values
			Transform<unsigned __int16, unsigned __int16>(SourceInfo.pFromTransform, DestInfo.pToTransform, SR, SG, SB, SA, SourceInfo.RBitsPerPixel, SourceInfo.GBitsPerPixel, SourceInfo.BBitsPerPixel, SourceInfo.ABitsPerPixel, DR, DG, DB, DA, DestInfo.RBitsPerPixel, DestInfo.GBitsPerPixel, DestInfo.BBitsPerPixel, DestInfo.ABitsPerPixel);
		}
		else
		{
			// default value transform
			if(SourceRMask && DestRMask)
			{
				if(DestInfo.RBitsPerPixel < SourceInfo.RBitsPerPixel)	// downsample
					DR = Shrink<unsigned __int16>(SR, SourceInfo.RBitsPerPixel, DestInfo.RBitsPerPixel);
				else if(DestInfo.RBitsPerPixel > SourceInfo.RBitsPerPixel)	// upsample
					DR = Expand<unsigned __int16>(SR, SourceInfo.RBitsPerPixel, DestInfo.RBitsPerPixel);
				else
					DR = SR;
			}

			if(SourceGMask && DestGMask)
			{
				if(DestInfo.GBitsPerPixel < SourceInfo.GBitsPerPixel)	// downsample
					DG = Shrink<unsigned __int16>(SG, SourceInfo.GBitsPerPixel, DestInfo.GBitsPerPixel);
				else if(DestInfo.GBitsPerPixel > SourceInfo.GBitsPerPixel)	// upsample
					DG = Expand<unsigned __int16>(SG, SourceInfo.GBitsPerPixel, DestInfo.GBitsPerPixel);
				else
					DG = SG;
			}

			if(SourceBMask && DestBMask)
			{
				if(DestInfo.BBitsPerPixel < SourceInfo.BBitsPerPixel)	// downsample
					DB = Shrink<unsigned __int16>(SB, SourceInfo.BBitsPerPixel, DestInfo.BBitsPerPixel);
				else if(DestInfo.BBitsPerPixel > SourceInfo.BBitsPerPixel)	// upsample
					DB = Expand<unsigned __int16>(SB, SourceInfo.BBitsPerPixel, DestInfo.BBitsPerPixel);
				else
					DB = SB;
			}

			if(SourceAMask && DestAMask)
			{
				if(DestInfo.ABitsPerPixel < SourceInfo.ABitsPerPixel)	// downsample
					DA = Shrink<unsigned __int16>(SA, SourceInfo.ABitsPerPixel, DestInfo.ABitsPerPixel);
				else if(DestInfo.ABitsPerPixel > SourceInfo.ABitsPerPixel)	// upsample
					DA = Expand<unsigned __int16>(SA, SourceInfo.ABitsPerPixel, DestInfo.ABitsPerPixel);
				else
					DA = SA;
			}
		}

		// write source to single variable
		U TempDest = ((U)(DR & DestRMask) << (U)DestRShift) | ((U)(DG & DestGMask) << (U)DestGShift) | ((U)(DB & DestBMask) << (U)DestBShift) | ((U)(DA & DestAMask) << (U)DestAShift);
		for(i = 0; i < DestInfo.BytesPerPixel; i++)
		{
			Dest[i] = (unsigned char)((TempDest >> ((T)i * 8)) & 0xff);
		}
	}

	return true;
}

bool VTFFile::Convert( unsigned char *Source, unsigned char *Dest, int Width, int Height, ImageFormat SourceFmt, ImageFormat DestFmt )
{
	assert( Source != 0 && Dest != 0 );
	assert( SourceFmt >= 0 && SourceFmt < NUM_IMAGE_FORMATS );
	assert( DestFmt >= 0 && DestFmt < NUM_IMAGE_FORMATS );

	const VTFImageConvertInfo& SourceInfo = ImageConvertInfo[SourceFmt];
	const VTFImageConvertInfo& DestInfo = ImageConvertInfo[DestFmt];

	if( SourceInfo.bIsSupported == false || DestInfo.bIsSupported == false )
	{
		// Verbose
		VLOGWARN( "Warning! Image format not supported! Source(%s) -> Dest(%s)\n", GetFormatName(SourceInfo.Format), GetFormatName(DestInfo.Format) );
		return false;
	}

	// Optimize common convertions.
	if( SourceFmt == DestFmt )
	{
		memcpy( Dest, Source, ComputeImageSize(Width,Height,1,DestFmt) );
		return true;
	}

	if( SourceFmt == IMAGE_FORMAT_RGB888 && DestFmt == IMAGE_FORMAT_RGBA8888 )
	{
		// Find the end of the buffer...
		unsigned char *pEnd = Source + ComputeImageSize( Width, Height, 1, SourceFmt );

		for(; Source < pEnd; Source += 3, Dest += 4)
		{
			Dest[0] = Source[0];
			Dest[1] = Source[1];
			Dest[2] = Source[2];
			Dest[3] = 255;
		}
		return true;
	}

	if( SourceFmt == IMAGE_FORMAT_RGBA8888 && DestFmt == IMAGE_FORMAT_RGB888 )
	{
		unsigned char *pEnd = Source + ComputeImageSize( Width, Height, 1, SourceFmt );
		for(; Source < pEnd; Source += 4, Dest += 3)
		{
			Dest[0] = Source[0];
			Dest[1] = Source[1];
			Dest[2] = Source[2];
		}
		return true;
	}

	// Do general conversions
	if( SourceInfo.bIsCompressed || DestInfo.bIsCompressed )
	{
		unsigned char *pSourceRGBA = Source;
		bool bResult = true;

		// allocate temp data for intermittent conversions
		if( SourceFmt != IMAGE_FORMAT_RGBA8888 )
		{
			pSourceRGBA = new unsigned char[ComputeImageSize(Width,Height,1,IMAGE_FORMAT_RGBA8888)];
		}

		// decompress the source or convert it to RGBA for compressing
		switch( SourceFmt )
		{
		case IMAGE_FORMAT_RGBA8888:
			break;
		case IMAGE_FORMAT_DXT1:
		case IMAGE_FORMAT_DXT1_ONEBITALPHA:
			bResult = DecompressDXT1( Source, pSourceRGBA, Width, Height );
			break;
		case IMAGE_FORMAT_DXT3:
			bResult = DecompressDXT3( Source, pSourceRGBA, Width, Height );
			break;
		case IMAGE_FORMAT_DXT5:
			bResult = DecompressDXT5( Source, pSourceRGBA, Width, Height );
			break;
		default:
			bResult = Convert( Source, pSourceRGBA, Width, Height, SourceFmt, IMAGE_FORMAT_RGBA8888 );
			break;
		}

		if( bResult )
		{
			// compress the source or convert it to the dest format if it is not compressed
			switch( DestFmt )
			{
			case IMAGE_FORMAT_DXT1:
			case IMAGE_FORMAT_DXT1_ONEBITALPHA:
			case IMAGE_FORMAT_DXT3:
			case IMAGE_FORMAT_DXT5:
				// Verbose
				VLOGWARN( "Warning! DXTN formats not supported as destination formats!\n" );
				return false;
				break;
			default:
				bResult = Convert( pSourceRGBA, Dest, Width, Height, IMAGE_FORMAT_RGBA8888, DestFmt );
				break;
			}
		}

		// free temp data
		if( pSourceRGBA != Source )
		{
			delete []pSourceRGBA;
		}

		return bResult;
	}
	else
	{
		// convert from one variable order and bit format to another
		if( SourceInfo.BytesPerPixel <= 1 )
		{
			if(DestInfo.BytesPerPixel <= 1)
			{
				return ConvertTemplated<unsigned __int8, unsigned __int8>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 2)
			{
				return ConvertTemplated<unsigned __int8, unsigned __int16>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 4)
			{
				return ConvertTemplated<unsigned __int8, unsigned __int32>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 8)
			{
				return ConvertTemplated<unsigned __int8, unsigned __int64>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
		}
		else if(SourceInfo.BytesPerPixel <= 2)
		{
			if(DestInfo.BytesPerPixel <= 1)
			{
				return ConvertTemplated<unsigned __int16, unsigned __int8>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 2)
			{
				return ConvertTemplated<unsigned __int16, unsigned __int16>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 4)
			{
				return ConvertTemplated<unsigned __int16, unsigned __int32>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 8)
			{
				return ConvertTemplated<unsigned __int16, unsigned __int64>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
		}
		else if(SourceInfo.BytesPerPixel <= 4)
		{
			if(DestInfo.BytesPerPixel <= 1)
			{
				return ConvertTemplated<unsigned int, unsigned __int8>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 2)
			{
				return ConvertTemplated<unsigned int, unsigned __int16>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 4)
			{
				return ConvertTemplated<unsigned int, unsigned __int32>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 8)
			{
				return ConvertTemplated<unsigned int, unsigned __int64>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
		}
		else if(SourceInfo.BytesPerPixel <= 8)
		{
			if(DestInfo.BytesPerPixel <= 1)
			{
				return ConvertTemplated<unsigned __int64, unsigned __int8>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 2)
			{
				return ConvertTemplated<unsigned __int64, unsigned __int16>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 4)
			{
				return ConvertTemplated<unsigned __int64, unsigned int>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
			else if(DestInfo.BytesPerPixel <= 8)
			{
				return ConvertTemplated<unsigned __int64, unsigned __int64>(Source, Dest, Width, Height, SourceInfo, DestInfo);
			}
		}
	}
	
	return false;
}

// Code taken from VTFLib
//-----------------------------------------------------------------------------------------------------
// DXTn decompression code is based on examples on Microsofts website and from the
// Developers Image Library (http://www.imagelib.org) (c) Denton Woods.
//
//-----------------------------------------------------------------------------------------------------
// DecompressDXT1(vlByte *src, vlByte *dst, vlUInt uiWidth, vlUInt uiHeight)
//
// Converts data from the DXT1 to RGBA8888 format. Data is read from *src
// and written to *dst. Width and height are needed to it knows how much data to process
//-----------------------------------------------------------------------------------------------------
bool VTFFile::DecompressDXT1( unsigned char* src, unsigned char* dst, unsigned int Width, unsigned int Height )
{
	unsigned int x, y, i, j, k, Select;
	unsigned char* Temp;
	Colour565 *color_0, *color_1;
	Colour8888 colours[4], *col;
	unsigned int bitmask, Offset;

	unsigned char nBpp = 4;				// bytes per pixel (4 channels (RGBA))
	unsigned char nBpc = 1;				// bytes per channel (1 byte per channel)
	unsigned int iBps = nBpp * nBpc * Width;		// bytes per scanline

	Temp = src;

	for (y = 0; y < Height; y += 4)
	{
		for (x = 0; x < Width; x += 4)
		{
			color_0 = ((Colour565*)Temp);
			color_1 = ((Colour565*)(Temp+2));
			bitmask = ((unsigned int*)Temp)[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			if (*((short*)color_0) > *((short*)color_1))
			{
				// Four-color block: derive the other two colors.    
				// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
				// These 2-bit codes correspond to the 2-bit fields 
				// stored in the 64-bit block.
				colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
				colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
				colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
				colours[2].a = 0xFF;

				colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
				colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
				colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
				colours[3].a = 0xFF;
			}
			else
			{
				// Three-color block: derive the other color.
				// 00 = color_0,  01 = color_1,  10 = color_2,
				// 11 = transparent.
				// These 2-bit codes correspond to the 2-bit fields 
				// stored in the 64-bit block. 
				colours[2].b = (colours[0].b + colours[1].b) / 2;
				colours[2].g = (colours[0].g + colours[1].g) / 2;
				colours[2].r = (colours[0].r + colours[1].r) / 2;
				colours[2].a = 0xFF;

				colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
				colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
				colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
				colours[3].a = 0x00;
			}

			for (j = 0, k = 0; j < 4; j++)
			{
				for (i = 0; i < 4; i++, k++)
				{
					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];

					if (((x + i) < Width) && ((y + j) < Height))
					{
						Offset = (y + j) * iBps + (x + i) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
						dst[Offset + 3] = col->a;
					}
				}
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------------------------------
// DecompressDXT3(vlByte *src, vlByte *dst, vlUInt uiWidth, vlUInt uiHeight)
//
// Converts data from the DXT3 to RGBA8888 format. Data is read from *src
// and written to *dst. Width and height are needed to it knows how much data to process
//-----------------------------------------------------------------------------------------------------
bool VTFFile::DecompressDXT3(unsigned char* src, unsigned char* dst, unsigned int Width, unsigned int Height )
{
	unsigned int x, y, i, j, k, Select;
	unsigned char* Temp;
	Colour565 *color_0, *color_1;
	Colour8888 colours[4], *col;
	unsigned int bitmask, Offset;
	short word;
	DXTAlphaBlockExplicit *alpha;

	unsigned char nBpp = 4;						// bytes per pixel (4 channels (RGBA))
	unsigned char nBpc = 1;						// bytes per channel (1 byte per channel)
	unsigned int iBps = nBpp * nBpc * Width;	// bytes per scanline

	Temp = src;

	for (y = 0; y < Height; y += 4)
	{
		for (x = 0; x < Width; x += 4)
		{
			alpha = (DXTAlphaBlockExplicit*)Temp;
			Temp += 8;
			color_0 = ((Colour565*)Temp);
			color_1 = ((Colour565*)(Temp+2));
			bitmask = ((unsigned int*)Temp)[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			// Four-color block: derive the other two colors.    
			// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
			// These 2-bit codes correspond to the 2-bit fields 
			// stored in the 64-bit block.
			colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
			colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
			colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
			colours[2].a = 0xFF;

			colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
			colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
			colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
			colours[3].a = 0xFF;

			k = 0;
			for (j = 0; j < 4; j++)
			{
				for (i = 0; i < 4; i++, k++)
				{
					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];

					if (((x + i) < Width) && ((y + j) < Height))
					{
						Offset = (y + j) * iBps + (x + i) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
					}
				}
			}

			for (j = 0; j < 4; j++)
			{
				word = alpha->row[j];
				for (i = 0; i < 4; i++)
				{
					if (((x + i) < Width) && ((y + j) < Height))
					{
						Offset = (y + j) * iBps + (x + i) * nBpp + 3;
						dst[Offset] = word & 0x0F;
						dst[Offset] = dst[Offset] | (dst[Offset] << 4);
					}
					
					word >>= 4;
				}
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------------------------------
// DecompressDXT5(vlByte *src, vlByte *dst, vlUInt uiWidth, vlUInt uiHeight)
//
// Converts data from the DXT5 to RGBA8888 format. Data is read from *src
// and written to *dst. Width and height are needed to it knows how much data to process
//-----------------------------------------------------------------------------------------------------
bool VTFFile::DecompressDXT5( unsigned char* src, unsigned char* dst, unsigned int Width, unsigned int Height)
{
	unsigned int x, y, i, j, k, Select;
	unsigned char* Temp;
	Colour565	*color_0, *color_1;
	Colour8888	colours[4], *col;
	unsigned int bitmask, Offset;
	unsigned char alphas[8], *alphamask;
	unsigned int bits;

	unsigned char nBpp = 4;						// bytes per pixel (4 channels (RGBA))
	unsigned char nBpc = 1;						// bytes per channel (1 byte per channel)
	unsigned int iBps = nBpp * nBpc * Width;	// bytes per scanline

	Temp = src;

	for (y = 0; y < Height; y += 4)
	{
		for (x = 0; x < Width; x += 4)
		{
			//if (y >= uiHeight || x >= uiWidth)
			//		break;

			alphas[0] = Temp[0];
			alphas[1] = Temp[1];
			alphamask = Temp + 2;
			Temp += 8;
			color_0 = ((Colour565*)Temp);
			color_1 = ((Colour565*)(Temp+2));
			bitmask = ((unsigned int*)Temp)[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			// Four-color block: derive the other two colors.    
			// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
			// These 2-bit codes correspond to the 2-bit fields 
			// stored in the 64-bit block.
			colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
			colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
			colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
			colours[2].a = 0xFF;

			colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
			colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
			colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
			colours[3].a = 0xFF;

			k = 0;
			for (j = 0; j < 4; j++)
			{
				for (i = 0; i < 4; i++, k++)
				{
					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];

					// only put pixels out < width or height
					if (((x + i) < Width) && ((y + j) < Height)) 
					{
						Offset = (y + j) * iBps + (x + i) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
					}
				}
			}

			// 8-alpha or 6-alpha block?    
			if (alphas[0] > alphas[1])
			{ 
				// 8-alpha block:  derive the other six alphas.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7;	// bit code 010
				alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7;	// bit code 011
				alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7;	// bit code 100
				alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7;	// bit code 101
				alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7;	// bit code 110
				alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7;	// bit code 111  
			}    
			else
			{  
				// 6-alpha block.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5;	// Bit code 010
				alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5;	// Bit code 011
				alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5;	// Bit code 100
				alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5;	// Bit code 101
				alphas[6] = 0x00;										// Bit code 110
				alphas[7] = 0xFF;										// Bit code 111
			}

			// Note: Have to separate the next two loops,
			//	it operates on a 6-byte system.

			// First three bytes
			bits = *((int*)alphamask);
			for (j = 0; j < 2; j++)
			{
				for (i = 0; i < 4; i++)
				{
					// only put pixels out < width or height
					if (((x + i) < Width) && ((y + j) < Height)) 
					{
						Offset = (y + j) * iBps + (x + i) * nBpp + 3;
							dst[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}

			// Last three bytes
			bits = *((int*)&alphamask[3]);
			for (j = 2; j < 4; j++)
			{
				for (i = 0; i < 4; i++)
				{
					// only put pixels out < width or height
					if (((x + i) < Width) && ((y + j) < Height)) 
					{
						Offset = (y + j) * iBps + (x + i) * nBpp + 3;
							dst[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}
		}
	}

	return true;
}

// Flips image data over the X axis.
void VTFFile::FlipImage( unsigned char* pImageDataRGBA8888, int Width, int Height )
{
	unsigned int *pImageData = (unsigned int *)pImageDataRGBA8888;

	for( int i = 0; i < Width; i++ )
	{
		for( int j = 0; j < Height / 2; j++ )
		{
			unsigned int *pOne = pImageData + (i + j * Width);
			unsigned int *pTwo = pImageData + (i + (Height - j - 1) * Width);

			unsigned int Temp = *pOne;
			*pOne = *pTwo;
			*pTwo = Temp;
		}
	}
}

const VTFImageFormatInfo& VTFFile::GetImageFormatInfo( ImageFormat Fmt )
{
	assert(Fmt >= 0 && Fmt < NUM_IMAGE_FORMATS);
	return ImageFormatInfo[Fmt];
}

int VTFFile::ComputeImageSize( int Width, int Height, int Depth, ImageFormat Fmt  )
{
	switch( Fmt )
	{
	case IMAGE_FORMAT_DXT1:
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		{
			if( Width < 4 && Width > 0 )
			{
				Width = 4;
			}

			if( Height < 4 && Height > 0 )
			{
				Height = 4;
			}
			return ((Width + 3) / 4) * ((Height + 3) / 4) * 8 * Depth;
		}
	case IMAGE_FORMAT_DXT3:
	case IMAGE_FORMAT_DXT5:
		{
			if( Width < 4 && Width > 0 )
			{
				Width = 4;
			}

			if( Height < 4 && Height > 0 )
			{
				Height = 4;
			}

			return ((Width + 3) / 4) * ((Height + 3) / 4) * 16 * Depth;
		}
	default:
		return Width * Height * Depth * VTFFile::GetImageFormatInfo(Fmt).BytesPerPixel;
	}
}

int VTFFile::ComputeImageSize( int Width, int Height, int Depth, int NumMips, ImageFormat Fmt )
{
	int nImageSize = 0;
	assert( Width != 0 && Height != 0 && Depth != 0 && "Invalid Width, Depth or Height value specified in image!" );

	for( int nIdx = 0; nIdx < NumMips; nIdx++ )
	{
		nImageSize += ComputeImageSize( Width, Height, Depth, Fmt );
		
		Width >>= 1;
		Height >>= 1;
		Depth >>= 1;

		if( Width < 1 )
		{
			Width = 1;
		}

		if( Height < 1 )
		{
			Height = 1;
		}

		if( Depth < 1 )
		{
			Depth = 1;
		}
	}

	return nImageSize;
}

//
// GetData()
// Gets the image data of the specified frame, face and mipmap in the format
// of the image.
//
unsigned char* VTFFile::GetData( int Frame, int Face, int Slice, int MipmapLevel ) const
{
	return m_pHighResData + ComputeDataOffset( Frame, Face, Slice, MipmapLevel, GetVTFHeader()->imageFormat );
}

// Returns the offset in our HiResDataBuffer of the data for an image at the 
// chose frame, face, and mip level. Frame number starts at 0, Face starts at 0
// MIP level 0 is the largest moving up to MIP count-1 for the smallest
// To get the first, and largest image, you would use 0, 0, 0
unsigned int VTFFile::ComputeDataOffset( int Frame, int Face, int Slice, int MipLevel, ImageFormat ImageFormat ) const
{
	unsigned int nOffset = 0;

	int nFrameCount = this->GetFrameCount();
	int nFaceCount = this->GetFaceCount();
	int nSliceCount = this->GetDepth();
	int nMipCount = this->GetMipmapCount();

	if(Frame >= nFrameCount)
	{
		Frame = nFrameCount - 1;
	}
	
	if(Face >= nFaceCount)
	{
		Face = nFaceCount - 1;
	}

	if(Slice >= nSliceCount)
	{
		Slice = nSliceCount - 1;
	}

	if(MipLevel >= nMipCount)
	{
		MipLevel = nMipCount - 1;
	}

	// Transverse past all frames and faces of each mipmap (up to the requested one)
	for( int i = (nMipCount - 1); i > MipLevel; i-- )
	{
		nOffset += ComputeMipmapSize( GetVTFHeader()->width, GetVTFHeader()->height, 1, i, ImageFormat) * nFrameCount * nFaceCount;
	}

	unsigned int Temp1 = this->ComputeMipmapSize( GetVTFHeader()->width, GetVTFHeader()->height, 1, MipLevel, ImageFormat );
	unsigned int Temp2 = this->ComputeMipmapSize( GetVTFHeader()->width, GetVTFHeader()->height, 1, MipLevel, ImageFormat );

	// Transverse past requested frames and faces of requested mipmap.
	nOffset += Temp1 * Frame * nFaceCount * nSliceCount;
	nOffset += Temp1 * Face * nSliceCount;
	nOffset += Temp2 * Slice;

	//assert( nOffset < m_nHighResDataSize );
	return nOffset;
}

//-----------------------------------------------------------------------------
// ComputeMIPSize( vlInt iMipLevel, VTFImageFormat fmt )
//
// Computes the size (in bytes) of a single mipmap of a single face of a single frame 
//-----------------------------------------------------------------------------
unsigned int VTFFile::ComputeMipmapSize( int Width, int Height, int Depth, int MipmapLevel, ImageFormat ImageFmt)
{
	// figure out the width/height of this MIP level
	int nMipmapWidth, nMipmapHeight, nMipmapDepth;
	VTFFile::ComputeMipmapDimensions( Width, Height, Depth, MipmapLevel, nMipmapWidth, nMipmapHeight, nMipmapDepth );
	
	// return the memory requirements
	return VTFFile::ComputeImageSize( nMipmapWidth, nMipmapHeight, nMipmapDepth, ImageFmt );
}

//-----------------------------------------------------------------------------
// ComputeMIPMapDimensions( vlInt iMipLevel, vlInt *pMipWidth, vlInt *pMipHeight )
//
// Computes the dimensions of a particular mip level
//-----------------------------------------------------------------------------
void VTFFile::ComputeMipmapDimensions( int Width, int Height, int Depth, int MipmapLevel, int &MipmapWidth, int &MipmapHeight, int &MipmapDepth )
{
	// work out the width/height by taking the orignal dimension
	// and bit shifting them down uiMipmapLevel times
	MipmapWidth = Width >> MipmapLevel;
	MipmapHeight = Height >> MipmapLevel;
	MipmapDepth = Depth >> MipmapLevel;
	
	// stop the dimension being less than 1 x 1
	if( MipmapWidth < 1 )
	{
		MipmapWidth = 1;
	}

	if( MipmapHeight < 1 )
	{
		MipmapHeight = 1;
	}

	if( MipmapDepth < 1 )
	{
		MipmapDepth = 1;
	}
}
