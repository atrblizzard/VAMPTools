#include <string>
#include <io.h>

#include "external\optimize.h"

#include "MDLFile.h"
#include "Logging.h"
#include "Utility.h"

using namespace std;
using namespace OptimizedModel;

MDLFile::MDLFile()
{
}

MDLFile::~MDLFile()
{
	if( Buffer )
	{
		delete [] Buffer;
	}

	if( VTXBuffer )
	{
		delete [] VTXBuffer;
	}
}

// Finds the specified model and creates a new MDLFile for it
MDLFile *MDLFile::FindModel( const char *InFilename )
{
	string NewFilename = InFilename;
	// Adjust any slashes in the filename
	Utility::FixupSlashes( NewFilename );

	// Check to see if the file exists
	if( _access(NewFilename.c_str(),0) == 0 )
	{
		FILE *File = 0;
		if( fopen_s(&File,NewFilename.c_str(),"rb") == 0 )
		{
			MDLFile *Model = new MDLFile( NewFilename );

			// Look up the size of the file, so we can allocate a buffer
			int Size = Utility::GetFileSize( File );
			if( Size > 0 )
			{
				Model->Buffer = new unsigned char[Size];

				// Read!
				_fread_nolock( Model->Buffer, Size, 1, File );
			}

			// Clean up file stuff
			fclose( File );
			File = 0;

			// Now look for the VTX file
			Utility::FindAndReplace( NewFilename, Utility::MDLExt, Utility::VTXExt );
			if( _access(NewFilename.c_str(),0) != 0 )
			{
				// Look for the backup VTX format file
				Utility::FindAndReplace( NewFilename, Utility::VTXExt, Utility::VTXExt2 );
				if( _access(NewFilename.c_str(),0) != 0 )
				{
					delete Model;
					return 0;
				}
			}

			// Open the VTX file
			if( fopen_s(&File,NewFilename.c_str(),"rb") == 0 )
			{
				Size = Utility::GetFileSize( File );
				if( Size > 0 )
				{
					Model->VTXBuffer = new unsigned char[Size];

					// Read!
					_fread_nolock( Model->VTXBuffer, Size, 1, File );
				}
			}

			// Clean up file stuff
			fclose( File );
			File = 0;

			FileHeader_t *VTXHeader = Model->GetVTXHeader();
			if( VTXHeader->version != MDLFile::VTMB_MODEL_FILE_VERSION )
			{
				// Verbose
				VLOGWARN( "Warning! Incorrect version (%d) expected: %d\n", VTXHeader->version, VTMB_MODEL_FILE_VERSION );
				delete Model;
				return 0;
			}

			if( VTXHeader->checkSum != Model->GetHeader()->Checksum )
			{
				// Verbose
				VLOGWARN( "Warning! Incorrect checksum (%x) expected: %x\n", VTXHeader->checkSum, Model->GetHeader()->Checksum );
				delete Model;
				return 0;
			}

			return Model;
		}
	}

	// Verbose
	VLOGWARN( "Warning! Cannot access specified file: %s\n", NewFilename.c_str() );

	return 0;
}