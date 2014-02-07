#include <string>
#include <io.h>

#include "zlib.h"
#include "il.h"

#include "MDLParser.h"
#include "MDLFile.h"
#include "VTFFile.h"
#include "Utility.h"
#include "Logging.h"

#define ZIPCHUNK_SIZE 32768
#define ZIPFILE "mdl.tmp"

extern bool g_bForceOverwrite;
extern void InitializeDefaultPaths();

using namespace std;

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

/** Merges new search paths into the list */
static void AddTempSearchPaths( vector<string>& NewSearchPaths, vector<string>& SearchPathList )
{
	for( vector<string>::iterator It = NewSearchPaths.begin(); It != NewSearchPaths.end(); ++It )
	{
		string NewPath = (*It);

		// Fixup any slashes
		Utility::FixupSlashes( NewPath );

		// Add the new path
		SearchPathList.push_back( NewPath );
	}
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

/** Extracts all the texture information from this model and stores the result on the model */
bool MDLParser::ExtractTextures( MDLFile *MDL )
{
	// Reset the default texture paths
	InitializeDefaultPaths();

	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		vector<string> TextureSearchPaths;

		// Verbose
		VLOG( "  Found %d texture search path(s)\n", HDR->NumTextureSearchPaths );

		for( int Idx = 0; Idx < HDR->NumTextureSearchPaths; Idx++ )
		{
			// Verbose
			VLOG( "   Texture Search Path[%d]: %s\n", Idx, HDR->GetTextureSearchPath(Idx) );

			// Add the search path
			string NewSearchPath = HDR->GetTextureSearchPath(Idx);

			// Adjust for slashes
			Utility::FixupSlashes( NewSearchPath );

			// Add the search path to the list
			TextureSearchPaths.push_back( NewSearchPath );
		}

		// Verbose
		VLOG( "  Found %d material(s)\n", HDR->NumTextures );

		// Get all the material data
		ExtractMaterials( MDL, TextureSearchPaths );

		// Iterate over the materials and load each texturre outt of the VMT file
		for( vector<MDLMaterial *>::iterator It = MDL->Materials.begin(); It != MDL->Materials.end(); ++It )
		{
			MDLMaterial *Material = (*It);

			// Add the search paths to the temp list
			vector<string> TempTextureSearchPaths;
			AddTempSearchPaths( Material->m_lstTexturePaths, TempTextureSearchPaths );

			// Extract the texture data for each texture / skip any textures that we've already created this run
			for( vector<string>::iterator It = Material->m_lstTextureNames.begin(); It != Material->m_lstTextureNames.end(); ++It )
			{
				const string& TextureName = (*It);

				// Create the output filepath and check to see if the specified texture already exists...
				string OutFilename;
				Utility::MakeTextureOutputPath( MDL->Filename, TextureName, OutFilename );

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
						VLOG( "   Skipping %s, found existing file at: %s\n", TextureName.c_str(), OutFilename.c_str() );
						continue;
					}
				}

				// Verbose
				VLOG( "   Loading texture: %s\n", TextureName.c_str() );

				// Find the TTH file in any of the various directories
				string TTHFilename;
				if( Utility::FindTTHFile(TextureName.c_str(),MDL->Filename,TextureSearchPaths,TempTextureSearchPaths,TTHFilename) == false )
				{
					// Verbose
					VLOGWARN( "   Warning! Could not find file %s.tth in any of the search directories!\n", TextureName.c_str() );
					continue;
				}

				// Find the TTZ file in any of the various directories
				string TTZFilename;
				if( Utility::FindTTZFile(TextureName.c_str(),MDL->Filename,TextureSearchPaths,TempTextureSearchPaths,TTZFilename) == false )
				{
					// Verbose
					VLOGWARN( "   Warning! Could not find file %s.ttz in any of the search directories!\n", TextureName.c_str() );
					continue;
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
						continue;
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
		}
	}

	return true;
}

/** Extracts all of the material data */
bool MDLParser::ExtractMaterials( MDLFile *MDL, const vector<string>& SearchPaths )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		for( int MatIdx = 0; MatIdx < HDR->NumTextures; MatIdx++ )
		{
			StudioTexture *Texture = HDR->GetTexture( MatIdx );
			
			// Verbose
			VLOG( "   Material[%d]: %s\n", MatIdx, Texture->GetName() );

			// Find the VTM file
			string VMTFilename;
			if( Utility::FindVMTFile(Texture->GetName(),MDL->Filename,SearchPaths,VMTFilename) == false )
			{
				// Verbose
				VLOGWARN( "   Warning! Could not find file %s.vmt in any of the search directories!\n", Texture->GetName() );
				continue;
			}

			// Verbose
			VLOG( "   Attempting to load material file: %s\n", VMTFilename.c_str() );

			// Open the material file and look for any base textures
			FILE *VMTFile;
			if( fopen_s(&VMTFile,VMTFilename.c_str(),"r+") != 0 )
			{
				// Verbose
				VLOGWARN( "    Warning: Could not open file %s for read\n", VMTFilename.c_str() );
				continue;
			}

			MDLMaterial *Material = new MDLMaterial( MatIdx );

			// Start parsing the material for textures
			char MaterialType[128];
			fscanf( VMTFile, "%s", &MaterialType );

			// Verbose
			VLOG( "    Shader: %s\n", MaterialType );

			// Flag to make sure we were able to parse out the texture name
			bool bParsedTexture = false;

			// Read parameters until we hit the end of the file
			while( feof(VMTFile) == 0 )
			{
				char ParamBuffer[128];
				fscanf( VMTFile, "%s", &ParamBuffer );

				// Look for a string with a $<param>
				if( strstr(ParamBuffer,"$") != 0 )
				{
					// Fixup the shader parameter
					string Param = ParamBuffer;
					Utility::FixupShaderValue( Param );

					// Verbose
					VLOG( "    Shader parameter: %s\n", Param.c_str() );

					// Extract texture parameter data
					if( Param == "$basetexture" || Param == "$dudvmap" || Param == "$iris" )
					{
						char ValueBuffer[128];
						fscanf( VMTFile, "%s", &ValueBuffer );

						// Store the file path
						string TexturePath = ValueBuffer;
						Utility::FixupShaderValue( TexturePath );

						// Add the search path
						string Path = TexturePath;
						int nPos = Path.find_last_of( "\\" );
						Path.erase( nPos + 1 );

						// Add the path
						Material->m_lstTexturePaths.push_back( Path );

						string TexName = TexturePath;
						TexName = TexturePath.substr( nPos + 1 );

						// Add the texture name
						Material->m_lstTextureNames.push_back( TexName );

						// We found a texture
						bParsedTexture = true;
					}
				}
			}

			// If we weren't able to find a texture, we're probably looking at a new type of material definition
			// dump some information about the material to identify the problem
			if( bParsedTexture == false )
			{
				// Verbose
				VLOGWARN( "Warning! Could not identify a texture parameter in material %s\n", MaterialType );
			}

			// Close the file
			fclose( VMTFile );
			VMTFile = 0;

			// Add the material to the list
			MDL->Materials.push_back( Material );
		}
	}

	return true;
}