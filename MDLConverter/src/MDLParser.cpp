#include <string>
#include <vector>

#include "MDLParser.h"
#include "MDLFile.h"
#include "MDLExporter.h"
#include "Utility.h"
#include "Logging.h"

extern bool g_bVerboseMode;

using namespace std;

/** Extracts all of the data from the model file and stores the resulting translated data back into specialized structures on that class */
bool MDLParser::ExtractData( MDLFile *MDL )
{
	vector<MDLFile *> ModelsToParse;

	// We're going to iterate over all included models... so treat the primary model just like any other
	ModelsToParse.push_back( MDL );

	// Recursively look up the included models and add them to the list
	GetIncludedModels( MDL, ModelsToParse );

	for( vector<MDLFile *>::iterator It = ModelsToParse.begin(); It != ModelsToParse.end(); ++It )
	{
		MDLFile *MDLToParse = (*It);

		bool bRootModel = MDLToParse == MDL;

		// Verbose
		VLOG( "Extracting data from file: %s\n", MDLToParse->Filename.c_str() );

		// Only extract and export textures for the root model
		if( bRootModel )
		{
			// Verbose
			VLOG( " Extracting texture information...\n" );
			ExtractTextures( MDLToParse );
		}

		// Verbose
		VLOG( " Extracting skeleton information...\n" );
		ExtractSkeleton( MDLToParse );

		// Verbose
		VLOG( " Extracting animation information...\n" );
		ExtractAnimations( MDLToParse );
	}

	// Export all the data!
	MDLExporter::ExportData( ModelsToParse );

	return true;
}

/** Recursively add all included models to the list */
void MDLParser::GetIncludedModels( MDLFile *MDL, vector<MDLFile *>& IncludedModels )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Loop over all the included models
		for( int Idx = 0; Idx < HDR->NumIncludeModels; Idx++ )
		{
			StudioModelGroup *ModelGroup = HDR->GetModelGroup( Idx );
			if( ModelGroup )
			{
				// Check to see if we can find the file
				string Filepath;
				if( Utility::FindModelFile(MDL->Filename,ModelGroup->GetFilename(),Filepath) )
				{
					MDLFile *NewFile = MDLFile::FindModel( Filepath.c_str() );
					if( NewFile )
					{
						// Add the new model to the included files list
						IncludedModels.push_back( NewFile );

						// Recurse into the new file
						GetIncludedModels( NewFile, IncludedModels );
					}
				}
			}
		}
	}
}