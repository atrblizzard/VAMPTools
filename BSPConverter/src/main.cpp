#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif

#include <fbxsdk.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "ArgHandler.h"

#include "BSPFile.h"
#include "BSPParser.h"
#include "BSPExporter.h"

bool g_bVerboseMode = false;
bool g_bAsciiMode = false;
bool g_bForceOverwrite = false;
FbxManager *g_pFbxManager;
FbxScene *g_pFbxScene;

void DisplayHelp()
{
	printf("usage: BSPConverter [opts] [file(s)]\n");
	printf("opts:\n");
	printf("    -?		help (this screen)\n");
	printf("    -v		verbose mode (spammier version of the parser)\n");
	printf("    -o		will force overwrite any existing output files\n");
	printf("    -ascii	ascii fbx mode (will output fbx files as ascii)\n\n");
	printf("Created By: Kiyoshi555\n");
	printf("Libraries: \n");
	printf(" Source SDK (modified) - http://developer.valvesoftware.com/ \n");
	printf(" Autodesk FBX SDK(ver): %s - http://usa.autodesk.com/ \n\n", g_pFbxManager->GetVersion());
}

void InitializeSDK()
{
	// Create the global manager
	g_pFbxManager = FbxManager::Create();

	// Create an IO settings object for the FBX layer
	FbxIOSettings *IOSettings = FbxIOSettings::Create( g_pFbxManager, IOSROOT );
	g_pFbxManager->SetIOSettings( IOSettings );
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

	// Loop over specified files
	for( int Idx = 0; Idx < AH.NumArgs(); Idx++ )
	{
		// Look up the filename
		char *Filename;
		if( AH.GetArg(Idx,&Filename) )
		{
			// Reset the FbxScene if its been created (will also apply proper units and axis conversion)
			BSPExporter::ResetFbxScene();

			// Load the bsp file
			BSPFile *F = BSPFile::FindBSP( Filename );
			if( F )
			{
				BSPParser::ExtractData( F );
			}
		}
	}

	ShutdownSDK();

	printf( "\n" );
	system("PAUSE");
	return 0;
}