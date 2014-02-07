#include <string>
#include <io.h>

#include "Logging.h"
#include "Utility.h"

#include "BSPFile.h"

using namespace std;

/** Setup list of cull entity types */
static void SetupCullEntities( vector<string>& CullEntities )
{
	CullEntities.push_back( "env_sprite" );
	CullEntities.push_back( "func_brush" );
	CullEntities.push_back( "func_rotating" );
	CullEntities.push_back( "func_lod" );
	CullEntities.push_back( "move_rope" );
}

/** ctor */
BSPFile::BSPFile()
{
	SetupCullEntities( m_lstCullEntities );
}

/** ctor */
BSPFile::BSPFile( string InFilename ) : Filename(InFilename)
{
	SetupCullEntities( m_lstCullEntities );
}

// Finds the specified bsp file and creates a new BSPFile for it
BSPFile *BSPFile::FindBSP( const char *InFilename )
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
			BSPFile *BSP = new BSPFile( NewFilename );

			// Look up the size of the file, so we can allocate a buffer
			int Size = Utility::GetFileSize( File );
			if( Size > 0 )
			{
				BSP->Buffer = new unsigned char[Size];

				// Read!
				_fread_nolock( BSP->Buffer, Size, 1, File );
			}

			// Clean up file stuff
			fclose( File );
			File = 0;

			return BSP;
		}
	}

	// Verbose
	VLOGWARN( "Warning! Cannot access specified file: %s\n", NewFilename.c_str() );

	return 0;
}