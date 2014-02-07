#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif
#include <fbxsdk.h>

#include <vector>

#include "MDLExporter.h"
#include "MDLFile.h"
#include "Utility.h"
#include "Logging.h"

extern FbxManager *g_pFbxManager;
extern FbxScene *g_pFbxScene;
extern bool g_bAsciiMode;
extern bool g_bVerboseMode;
extern bool g_bApplyScale;

using namespace std;

vector<string> MDLExporter::ExportedAnimations;

/** Exports specific bits of data */
bool MDLExporter::ExportData( vector<MDLFile *> MDLs )
{
	MDLFile *RootMDL = (*MDLs.begin());

	// Clear the list of animations we've exported
	ExportedAnimations.clear();

	// After we export the main mesh, we don't to export the root model's mesh for all the included animations
	bool bRemovedMesh = false;
	for( vector<MDLFile *>::iterator It = MDLs.begin(); It != MDLs.end(); ++It )
	{
		MDLFile *MDLToExport = (*It);

		bool bRootModel = MDLToExport == RootMDL;

		// Verbose
		VLOG( "Exporting file: %s\n", MDLToExport->Filename.c_str() );

		// We only want to export one skeleton to the FBX file
		if( bRootModel )
		{
			// Verbose
			VLOG( " Exporting skeleton information...\n" );
			bool bExportedSkeleton = ExportSkeleton( MDLToExport );

			// Verbose
			VLOG( " Exporting mesh information...\n" );
			ExportMeshes( MDLToExport, bExportedSkeleton );

			// Only compute a bind pose if there was a skeleton
			if( bExportedSkeleton )
			{
				// Verbose
				VLOG( " Computing bind pose...\n" );
				ComputeBindPose( MDLToExport );
			}

			// Verbose
			VLOG( " Exporting reference mesh...\n" );
			ExportScene( MDLToExport->Filename.c_str() );

			// Only export animations if we have a skeleton
			if( bExportedSkeleton )
			{
				// Verbose
				VLOG( " Extracting animation information...\n" );
				ExportAnimations( MDLToExport );
			}
		}
		else
		{
			// For each new model we're exporting... we should probably rebuild the skeleton, to do so we'll need to junk
			// the current scene
			ResetFbxScene();

			// Verbose
			VLOG( " Exporting skeleton information...\n" );
			bool bExportedSkeleton = ExportSkeleton( MDLToExport );

			// This shouldn't really happen... but if it does - we should probably be checking if we actually exported
			// a skeleton before trying to export the anim data
			if( bExportedSkeleton )
			{
				// Verbose
				VLOG( " Extracting animation information...\n" );
				ExportAnimations( MDLToExport );
			}
		}
	}

	return true;
}

/** Exports the data to FBX! */
bool MDLExporter::ExportScene( const char *FilePath )
{
	string OutFilename = FilePath;
	Utility::FindAndReplace( OutFilename, Utility::MDLExt, Utility::FBXExt );

	// Verbose
	VLOG( "Starting FBX export...\n" );

	// By default, write in binary
	FbxIOPluginRegistry *IORegistry = g_pFbxManager->GetIOPluginRegistry();
		
	int FileFormat = IORegistry->GetNativeWriterFormat();

	// If we've specified an ascii format, try to look up the output format
	if( g_bAsciiMode )
	{
		// Verbose
		VLOG( " ASCII output format specified\n" );

		// Try to find the fbx ascii exporter
		for( int Idx = 0; Idx < IORegistry->GetWriterFormatCount(); Idx++ )
		{
			if( IORegistry->WriterIsFBX(Idx) )
			{
				FbxString WriterDesc = IORegistry->GetWriterFormatDescription(Idx);
				if( WriterDesc.Find("ascii") >= 0 )
				{
					FileFormat = Idx;
					break;
				}
			}
		}
	}

	// Verbose
	VLOG( " Initializing exporter with output filename: %s\n", OutFilename.c_str() );

	FbxExporter *Exporter = FbxExporter::Create( g_pFbxManager, "Exporter" );
	if( Exporter->Initialize(OutFilename.c_str(),FileFormat,g_pFbxManager->GetIOSettings()) == false )
	{
		VLOGWARN( "Warning! Could not create FBX exporter for filename: %s\n", OutFilename.c_str() );
		return false;
	}

	// Export!
	bool bSuccess = Exporter->Export( g_pFbxScene );

	// Destroy the exporter now that we're done
	Exporter->Destroy();

	if( bSuccess )
	{
		VLOG( "File was exported successfully!\n" );
		return true;
	}

	VLOGWARN( "Warning! File was not exported successfully...\n" );
	return false;
}

/** Creates a new FbxScene */
bool MDLExporter::CreateFbxScene()
{
	g_pFbxScene = FbxScene::Create( g_pFbxManager, "ExportScene" );
	if( !g_pFbxScene )
	{
		FBXSDK_printf( "Error! Unable to create FBX scene\n" );
		exit( 1 );
	}
	return true;
}

/** Destroys the FbxScene */
bool MDLExporter::DestroyFbxScene()
{
	if( g_pFbxScene )
	{
		// Kill the scene so that we don't have any leakage between models (assuming there's more than one to export)
		g_pFbxScene->Destroy( true );
		g_pFbxScene = 0;
	}
	return true;
}

/** Resets the FBX scene */
bool MDLExporter::ResetFbxScene()
{
	// Nuke the scene!
	DestroyFbxScene();

	// Create a new scene
	CreateFbxScene();

	// Convert the scene
	ConvertFbxScene();

	return true;
}

/** Converts the scene to use the right units and to be on the right axes */
bool MDLExporter::ConvertFbxScene()
{
	if( g_bApplyScale )
	{
		// Change the units
		FbxSystemUnit:: Inch.ConvertScene( g_pFbxScene );
	}

	// Change the coordinate system
	FbxAxisSystem::MayaYUp.ConvertScene( g_pFbxScene );

	return true;
}