#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif

#include <fbxsdk.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "il.h"
#include "MDLFile.h"
#include "MDLParser.h"
#include "MDLExporter.h"
#include "ArgHandler.h"

bool g_bVerboseMode = false;
bool g_bAsciiMode = false;
bool g_bForceOverwrite = false;
bool g_bApplyScale = false;
FbxManager *g_pFbxManager;
FbxScene *g_pFbxScene;

void DisplayHelp()
{
	printf("usage: MDLConverter [opts] [file(s)]\n");
	printf("opts:\n");
	printf("    -?		help (this screen)\n");
	printf("    -v		verbose mode (spammier version of the parser)\n");
	printf("    -o		will force overwrite any existing output files\n");
	printf("    -ascii	ascii fbx mode (will output fbx files as ascii)\n");
	printf("    -s		will convert scale from inch to cm for export\n\n");
	printf("Created By: Kiyoshi555\n");
	printf("Libraries: \n");
	printf(" Source SDK (modified) - http://developer.valvesoftware.com/ \n");
	printf(" VTFLib - http://nemesis.thewavelength.net/index.php?p=40 \n");
	printf(" DevIL - http://openil.sourceforge.net/ \n" );
	printf(" zlib - http://zlib.net/ \n");
	printf(" Autodesk FBX SDK(ver): %s - http://usa.autodesk.com/ \n\n", g_pFbxManager->GetVersion());
	printf(" Changes to libraries available upon request as per GPL/LPGL licenses (where applicable)\n" );
}

void InitializeSDK()
{
	// Create the global manager
	g_pFbxManager = FbxManager::Create();

	// Create an IO settings object for the FBX layer
	FbxIOSettings *IOSettings = FbxIOSettings::Create( g_pFbxManager, IOSROOT );
	g_pFbxManager->SetIOSettings( IOSettings );

	// Initialize DevIL
	ilInit();
}

void ShutdownSDK()
{
	// Cleanup the manager
	if( g_pFbxManager )
	{
		g_pFbxManager->Destroy();
	}
}

// Main function
int main( int argc, char *argv[] )
{
	// Set up the global fbx data
	InitializeSDK();

	// Evaluate any arguments passed in
	ArgHandler AH;
	AH.SetOpts( "-?", "help" );
	AH.SetOpts( "-v", "verbose" );
	AH.SetOpts( "-o", "overwrite" );
	AH.SetOpts( "-ascii", "ascii" );
	AH.SetOpts( "-s", "scale" );

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
	// Set the ascii mode
	g_bAsciiMode = AH.QueryKey( "ascii" );
	// Set the scale mode
	g_bApplyScale = AH.QueryKey( "scale" );

	// Loop over specified files
	for( int Idx = 0; Idx < AH.NumArgs(); Idx++ )
	{
		// Look up the filename
		char *Filename;
		if( AH.GetArg(Idx,&Filename) )
		{
			// Reset the FbxScene if its been created (will also apply proper units and axis conversion)
			MDLExporter::ResetFbxScene();

			// Load the model file and vtx data
			MDLFile *F = MDLFile::FindModel( Filename );
			if( F )
			{
				// Start extracting all the information
				MDLParser::ExtractData( F );
			}
		}
	}

	ShutdownSDK();

	printf( "\n" );
	system("PAUSE");
	return 0;
}