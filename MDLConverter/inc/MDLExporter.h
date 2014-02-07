#ifndef MDLEXPORTER_H
#define MDLEXPORTER_H

#include <vector>

class MDLExporter
{
public:

	// Export all the datas!
	static bool ExportData( std::vector<class MDLFile *> MDLs );

	// Creates the FBX scene
	static bool CreateFbxScene();
	// Destroys the FBX scene
	static bool DestroyFbxScene();
	// Resets the FBX scene
	static bool ResetFbxScene();
	// Converts the scene to use the right units and on the right axes for Maya
	static bool ConvertFbxScene();

private:

	// Exports the entire FBX scene to the given filepath
	static bool ExportScene( const char *FilePath );

	// Exports all the skeleton data to the FBX scene
	static bool ExportSkeleton( class MDLFile* MDL );
	// Exports all mesh data to the FBX scene
	static bool ExportMeshes( class MDLFile *MDL, bool bExportSkin );
	// Creates a bind pose for the mesh in the FBX scene, binding the mesh to the skeleton. Can only be called after skeleton and mesh have been created
	static void ComputeBindPose( class MDLFile *MDL );

	// Exports one scene per animation
	static bool ExportAnimations( class MDLFile *MDL );

	// Creates a FbxMesh for export
	static class FbxMesh *CreateFbxMesh( class MDLFile *MDL, const std::string &MeshName, struct StudioMesh *MeshData );
	// Creates a FbxMaterial for binding to the export mesh
	static class FbxSurfacePhong *CreateFbxMaterial( const std::string &MatName, struct StudioMesh *MeshData );
	// Creates a FbxFileTexture for binding to the export mesh
	static class FbxFileTexture *CreateFbxTexture( class MDLFile *MDL, class FbxMesh *ExportMesh, struct StudioMesh *MeshData );
	// Creates a skin cluster for binding to the export mesh
	static class FbxSkin *CreateFbxSkin( class MDLFile *MDL, struct StudioMesh *MeshData );
	// Creates an animation for export
	static class FbxAnimStack *CreateFbxAnimation( class MDLFile *MDL, class Animation *AnimData );

	// list of exported animations
	static std::vector<std::string> ExportedAnimations;
};

#endif // MDLEXPORTER_H