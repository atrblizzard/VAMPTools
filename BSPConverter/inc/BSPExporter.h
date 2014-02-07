#pragma once

#include <string>

class BSPExporter
{
public:

	// Exports the BSP data
	static bool ExportData( class BSPFile *BSP );

	// Creates the FBX scene
	static bool CreateFbxScene();
	// Destroys the FBX scene
	static bool DestroyFbxScene();
	// Resets the FBX scene
	static bool ResetFbxScene();
	// Converts the scene to use the right units and on the right axes for Maya
	static bool ConvertFbxScene();

private:

	// Exports mesh data
	static bool ExportMeshes( class BSPFile *BSP );
	// Exports the entire FBX scene to the given filepath
	static bool ExportScene( const char *FilePath );
	// Exports the list of textures used by this scene
	static bool ExportTextureList( class BSPFile *BSP, std::string& FilePath );
	// Exports the pak file
	static bool ExportPakFile( class BSPFile *BSP );
	// Exports the list of entities
	static bool ExportEntityList( class BSPFile *BSP );
	// Export the list of static props
	static bool ExportStaticPropList( class BSPFile *BSP );

	// Creates a FbxMesh for export
	static class FbxMesh *CreateFbxMesh( class BSPFile *MDL, const std::string &MeshName );
};