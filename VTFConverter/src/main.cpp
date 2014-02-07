//#include <stdlib.h>
//#include <stdio.h>
//#include <errno.h>
#include <io.h>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "zlib.h"
#include "il.h"
#include "ArgHandler.h"
#include "Logging.h"
#include "Utility.h"

#include "VTFFile.h"

#define ZIPCHUNK_SIZE 32768
#define ZIPFILE "vtf.tmp"

bool g_bVerboseMode = false;
bool g_bAsciiMode = false;
bool g_bForceOverwrite = false;

typedef unsigned char byte;

using namespace std;

void DisplayHelp()
{
	printf("usage: VTFConverter [opts] [file(s)]\n");
	printf("opts:\n");
	printf("    -?		help (this screen)\n");
	printf("    -v		verbose mode (spammier version of the parser)\n");
	printf("    -o		will force overwrite any existing output files\n");
	printf("Created By: Kiyoshi555\n");
	printf("Libraries: \n");
	printf(" Source SDK (modified) - http://developer.valvesoftware.com/ \n");
	printf(" VTFLib - http://nemesis.thewavelength.net/index.php?p=40 \n");
	printf(" DevIL - http://openil.sourceforge.net/ \n" );
	printf(" zlib - http://zlib.net/ \n");
	printf(" Changes to libraries available upon request as per GPL/LPGL licenses (where applicable)\n" );
}

void InitializeSDK()
{
	// Initialize DevIL
	ilInit();
}

/** Helper function to unzip a buffer of compressed data */
static bool Decompress( unsigned char *InBuffer, unsigned long BufferSize, unsigned char **OutBuffer, unsigned long *OutBufferSize )
{
	int nRet;
	unsigned nHave;
	z_stream pStream;
	unsigned char pOut[ZIPCHUNK_SIZE];

	/* allocate inflate state */
	pStream.zalloc = Z_NULL;
	pStream.zfree = Z_NULL;
	pStream.opaque = Z_NULL;
	pStream.avail_in = 0;
	pStream.next_in = Z_NULL;

	// Initialize
	nRet = inflateInit(&pStream);
	if( nRet != Z_OK )
	{
		// Verbose
		VLOGWARN( "Warning! Could not initalize z_stream!\n" );
		return false;
	}

	// How much data is coming in?
	pStream.avail_in = BufferSize;
	if( pStream.avail_in == 0 )
	{
		VLOGWARN( "Warning! Decompress buffersize is invalid!\n" );
		return false;
	}

	FILE *pTempFile = 0;
	if( fopen_s(&pTempFile,ZIPFILE,"w+b") != 0 )
	{
		VLOGWARN( "Warning! Couldn't open temp output file!\n" );
		return false;
	}

	// Pointer to the input buffer
	pStream.next_in = InBuffer;

	/* run inflate() on input until output buffer not full, then dump it and keep going */
	do 
	{
		// Configure the next output chunk
		pStream.avail_out = ZIPCHUNK_SIZE;
		pStream.next_out = pOut;

		// inflate some data
		nRet = inflate( &pStream, Z_NO_FLUSH );
		assert( nRet != Z_STREAM_ERROR );  /* state not clobbered */

		// What happened?
		switch( nRet )
		{
		case Z_NEED_DICT:
			nRet = Z_DATA_ERROR; /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			// Verbose
			VLOGWARN( "Warning! Could not decompress data!\n" );
			(void)inflateEnd( &pStream );
			fclose( pTempFile );
			return false;
		}

		// How much data was decompressed?
		nHave = ZIPCHUNK_SIZE - pStream.avail_out;

		if( fwrite(pOut,1,nHave,pTempFile) != nHave || ferror(pTempFile) ) 
		{
			// Verbose
			VLOGWARN( "Warning! Error writing data to temp file!\n" );
			(void)inflateEnd(&pStream);
			fclose( pTempFile );
			return false;
		}
	} 
	while( nRet != Z_STREAM_END );

	/* clean up zip stuff! */
	(void)inflateEnd( &pStream );

	// Now that we've cheated to figure out how big the uncompressed data is (by streaming it to disk)
	fseek( pTempFile, 0, SEEK_END );
	int nSize = ftell( pTempFile );
	fseek( pTempFile, 0, SEEK_SET );

	// Read all the data back into a new buffer - which we'll return
	(*OutBuffer) = new unsigned char[nSize];
	_fread_nolock( (*OutBuffer), nSize, 1, pTempFile );
	(*OutBufferSize) = nSize;

	fclose( pTempFile );
	remove( ZIPFILE );
	return true;
}

/** Creates a VTF file based on data read out of the two specified files */
static VTFFile *CreateVTFFile( const string& TTHFilename, const string& TTZFilename )
{
	// Read in the TTH file
	FILE *TTHFile;
	if( fopen_s(&TTHFile,TTHFilename.c_str(),"r+b") != 0 )
	{
		// Verbose
		VLOGWARN( "   Warning: Could not open file %s for read\n", TTHFilename.c_str() );
		return 0;
	}

	// Open the TTZ file
	FILE  *TTZFile;
	if( fopen_s(&TTZFile,TTZFilename.c_str(),"r+b") != 0 )
	{
		// Verbose
		VLOGWARN( "   Warning: Could not open file %s for read\n", TTZFilename.c_str() );
		fclose( TTHFile );
		return 0;
	}

	// Read in the TTH data
	int TTHSize = Utility::GetFileSize( TTHFile );
	byte *TTHBuffer(0);

	if( TTHSize > 0 )
	{
		// Verbose
		VLOG( "   Reading data out of %s\n", TTHFilename.c_str() );

		TTHBuffer = new byte[TTHSize];
		_fread_nolock( TTHBuffer, TTHSize, 1, TTHFile );
	}

	// Clean up the TTHFile
	fclose( TTHFile );
	TTHFile = 0;

	// Create a new VTFFile
	VTFFile *VTF = new VTFFile;
	TTHHeader *TTHHdr = (TTHHeader *)TTHBuffer;

	// Read in all of the compressed data in the TTZ
	int TTZSize = Utility::GetFileSize( TTZFile );
	byte *TTZBuffer(0);

	if( TTZSize > 0 )
	{
		// Verbose
		VLOG( "   Reading data out of %s\n", TTZFilename.c_str() );

		TTZBuffer = new byte[TTZSize];
		_fread_nolock( TTZBuffer, TTZSize, 1, TTZFile );

		// We've read in a bunch of compressed data, now we need to decompress it
		byte *RawTTZBuffer(0);
		unsigned long RawBufferSize;
		// Decompress will allocate RawTTZBuffer and fill it with the uncompressed data
		if( Decompress(TTZBuffer,TTZSize,&RawTTZBuffer,&RawBufferSize) )
		{
			// Verbose
			VLOG( "   Decompressing %s from %d to %d bytes\n", TTZFilename.c_str(), TTZSize, RawBufferSize );

			// Figure out where the VTF header starts in the TTH file - we don't really care about the TTH data anymore
			// since we really only use it to figure out TTZ data
			int VTFOffset = TTHSize - TTHHdr->VTFSize;

			// Create a new combined buffer to hold the final uncompressed TTZ file
			VTF->InitializeBuffers( TTHHdr->VTFSize + RawBufferSize );

			// Copy the uncompressed data out of the TTH file buffer into the start of the combined buffer
			memcpy_s( VTF->GetBuffer(), TTHHdr->VTFSize, TTHBuffer + VTFOffset, TTHHdr->VTFSize );
			// Copy all the data out of the uncompressed TTZ buffer
			memcpy_s( VTF->GetBuffer() + TTHHdr->VTFSize, RawBufferSize, RawTTZBuffer, RawBufferSize );

			// Don't need the raw buffer anymore (we've copied the data into the VTF
			if( RawTTZBuffer )
			{
				delete[] RawTTZBuffer;
			}
		}

		// Cleanup 
		delete[] TTZBuffer;
		TTZBuffer = 0;
	}

	fclose( TTZFile );
	TTZFile = 0;
	if( TTHBuffer )
	{
		delete[] TTHBuffer;
		TTHBuffer = 0;
	}

	return VTF;
}

/** Parses the file */
void ParseFile( const char* Filename )
{
	string BaseFilename;
	Utility::GetFilename( Filename, BaseFilename );
	Utility::StripExtension( BaseFilename );

	// Create the output filepath and check to see if the specified texture already exists...
	string OutFilename;
	Utility::MakeTextureOutputPath( Filename, BaseFilename, OutFilename );

	// If the file exists
	if( _access(OutFilename.c_str(),0) == 0 )
	{
		// If we're in overwrite mode...
		if( g_bForceOverwrite )
		{
			// Nuke the file if it exists
			remove( OutFilename.c_str() );
		}
		// Otherwise, skip processing this file
		else
		{
			// Verbose
			VLOG( "Skipping %s, found existing file at: %s\n", BaseFilename.c_str(), OutFilename.c_str() );
			return;
		}
	}

	// Verbose
	VLOG( " Parsing texture: %s\n", BaseFilename.c_str() );

	// Find the TTH file in any of the various directories
	string TTHFilename;
	vector<string> Empty;
	if( Utility::FindTTHFile(BaseFilename.c_str(),Filename,Empty,Empty,TTHFilename) == false )
	{
		// Verbose
		VLOGWARN( "  Warning! Could not find file %s.tth in any of the search directories!\n", BaseFilename.c_str() );
		return;
	}

	// Find the TTZ file in any of the various directories
	string TTZFilename;
	if( Utility::FindTTZFile(BaseFilename.c_str(),Filename,Empty,Empty,TTZFilename) == false )
	{
		// Verbose
		VLOGWARN( "  Warning! Could not find file %s.ttz in any of the search directories!\n", BaseFilename.c_str() );
		return;
	}

	// Create the VTF file from the two files (TTH and TTZ)
	VTFFile *VTF = CreateVTFFile( TTHFilename, TTZFilename );
	if( VTF )
	{
		const VTFHeader *VTFHdr = VTF->GetVTFHeader();
		// Now that we have a full VTF in memory...
		// Verbose
		VLOG( "    Version: %d.%d\n", VTFHdr->version[0], VTFHdr->version[1] );
		VLOG( "    Dimensions (WxH): %dx%d\n", VTFHdr->width, VTFHdr->height );
		VLOG( "    Format: %s\n", VTF->GetFormatName( VTFHdr->imageFormat) );

		// Compute the size of the low-res data
		int LowResBufferSize(0);
		if( VTFHdr->lowResImageFormat != IMAGE_FORMAT_UNKNOWN )
		{
			LowResBufferSize = VTF->ComputeImageSize( VTFHdr->lowResImageWidth, VTFHdr->lowResImageHeight, 1, VTFHdr->lowResImageFormat );
		}

		// Initialize the low res data
		VTF->SetLowResData( VTFHdr->headerSize, LowResBufferSize );

		// Compute the high res data size
		int HighResBufferSize = VTFFile::ComputeImageSize( VTFHdr->width, VTFHdr->height, 1, VTFHdr->numMipLevels, VTFHdr->imageFormat );

		// Initialize the high res data
		VTF->SetHighResData( VTFHdr->headerSize + LowResBufferSize, HighResBufferSize );

		// Sanity check (actual size should be == to all image data)
		if( VTFHdr->headerSize > VTF->GetBufferSize() || VTFHdr->headerSize + LowResBufferSize > VTF->GetBufferSize() || VTFHdr->headerSize + LowResBufferSize + HighResBufferSize > VTF->GetBufferSize() )
		{
			// Verbose
			VLOGWARN( "   Warning! File is too small for specified image data!\n" );
			return;
		}

		// Allocate a buffer for the output texture
		byte *ImageData(0);
		ImageData = new byte[ VTF->ComputeImageSize(VTFHdr->width,VTFHdr->height,1,1,IMAGE_FORMAT_RGBA8888) ];

		// Try to convert the highest level mipmap
		if( VTF->Convert(VTF->GetData(0,0,0,0),ImageData,VTFHdr->width,VTFHdr->height,VTFHdr->imageFormat,IMAGE_FORMAT_RGBA8888) )
		{
			// DevIL likes image data upside down...
			VTF->FlipImage( ImageData, VTFHdr->width, VTFHdr->height );

			// Write out a png file
			if( ilTexImage(VTFHdr->width, VTFHdr->height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, ImageData) )
			{
				ilSaveImage( OutFilename.c_str() );
			}
		}

		delete[] ImageData;
	}
}

// Main function
int main( int argc, char *argv[] )
{
	// Set up the global data
	InitializeSDK();

	// Evaluate any arguments passed in
	ArgHandler AH;
	AH.SetOpts( "-?", "help" );
	AH.SetOpts( "-v", "verbose" );
	AH.SetOpts( "-o", "overwrite" );

	// Evaluate the arguments
	AH.EvalArgs( argc, argv );

	// Force display the help text if there's only one 
	if( argc <= 1 || AH.NumArgs() == 0 )
	{
		AH.ForceOpt( "help", true );
	}

	// If help was specified on the command line, show it and bail
	if( AH.QueryKey("help") )
	{
		DisplayHelp();
		system("PAUSE");
		return 0;
	}

	// Set the verbose mode
	g_bVerboseMode = AH.QueryKey( "verbose" );
	// Set the overwrite mode
	g_bForceOverwrite = AH.QueryKey( "overwrite" );

	// Loop over specified files
	for( int Idx = 0; Idx < AH.NumArgs(); Idx++ )
	{
		// Look up the filename
		char *Filename;
		if( AH.GetArg(Idx,&Filename) )
		{
			// Parse and export the file
			ParseFile( Filename );
		}
	}

	printf( "\n" );
	system("PAUSE");
	return 0;
}