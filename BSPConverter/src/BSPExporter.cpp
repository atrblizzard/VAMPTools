#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif
#include <fbxsdk.h>

#include <vector>

#include "Logging.h"
#include "Utility.h"

#include "BSPExporter.h"
#include "BSPFile.h"

extern FbxManager *g_pFbxManager;
extern FbxScene *g_pFbxScene;
extern bool g_bAsciiMode;

using namespace std;

bool BSPExporter::ExportData( BSPFile *BSP )
{
	if( BSP )
	{
		// Exporting mesh data
		VLOG( " Creating mesh data...\n" );
		ExportMeshes( BSP );

		// Verbose
		VLOG( " Exporting FBX scene...\n" );
		ExportScene( BSP->Filename.c_str() );

		// Verbose
		VLOG( "  Exporting texture list...\n" );
		ExportTextureList( BSP, BSP->Filename );

		// Verbose
		VLOG( "  Exporting pakfile...\n" );
		ExportPakFile( BSP );

		// Verbose
		VLOG( "  Exporting entity list...\n" );
		ExportEntityList( BSP );

		// Verbose
		VLOG( "  Exporting static prop list...\n" );
		ExportStaticPropList( BSP );
	}

	return true;
}

bool BSPExporter::ExportMeshes( BSPFile *BSP )
{
	// Create the export mesh
	FbxMesh *ExportMesh = CreateFbxMesh( BSP, "mesh" );
	if( ExportMesh )
	{
		// Add the geometry node
		FbxNode *GeometryNode = FbxNode::Create( g_pFbxScene, "geometry" );
		g_pFbxScene->GetRootNode()->AddChild( GeometryNode );

		GeometryNode->SetNodeAttribute( ExportMesh );
	}

	return true;
}

/** Exports the data to FBX! */
bool BSPExporter::ExportScene( const char *FilePath )
{
	string OutFilename = FilePath;
	Utility::FindAndReplace( OutFilename, Utility::BSPExt, Utility::FBXExt );

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

/** Dumps the list of textures used in this level */
bool BSPExporter::ExportTextureList( BSPFile *BSP, string& FilePath )
{
	FILE *F;
	string Filename;

	Utility::MakeTextureListPath( FilePath, Filename );
	if( Filename.size() > 0 )
	{
		fopen_s( &F, Filename.c_str(), "w+t" );
		if( F )
		{
			for( int Idx = 0; Idx < BSP->m_lstTextureData.size(); Idx++ )
			{
				dtexdata_t *TexData = BSP->m_lstTextureData[Idx];
				if( TexData )
				{
					string &TexString = BSP->m_lstStringTable[TexData->nameStringTableID];
					Utility::FindAndReplace( TexString, "\\", " " );
					Utility::FindAndReplace( TexString, "/", " " );
					fprintf( F, "%s\n", TexString.c_str() );
				}
			}

			// Verbose
			VLOG( "   Dumped %d texture name(s)\n", BSP->m_lstTextureData.size() );
		}
		fclose(F);
	}

	return true;
}

/** Exports the pak file */
bool BSPExporter::ExportPakFile( BSPFile *BSP )
{
	FILE *F;
	string Filename;

	Utility::MakePakfilePath( BSP->Filename, Filename );
	if( Filename.size() > 0 )
	{
		fopen_s( &F, Filename.c_str(), "w+b" );
		if( F )
		{
			// Write!
			fwrite( BSP->m_pPackFileLump, BSP->m_nPackFileSize, 1, F );

			// Verbose
			VLOG( "   Wrote %d bytes to pakfile\n", BSP->m_nPackFileSize );
		}
		fclose(F);
	}

	return true;
}

/** Exports the list of entities in the level */
bool BSPExporter::ExportEntityList( class BSPFile *BSP )
{
	FILE *F;
	string Filename;

	Utility::MakeUtilityPath( BSP->Filename, "_entity.txt", Filename );
	if( Filename.size() > 0 )
	{
		fopen_s( &F, Filename.c_str(), "w+b" );
		if( F )
		{
			int WriteCount = 0;

			// Dump the list of entities
			for( int Idx = 0; Idx < BSP->m_lstEntities.size(); Idx++ )
			{
				EntityData* Entity = BSP->m_lstEntities[Idx];
				if( Entity )
				{
					bool bCullEntity = false;

					// Iterate over all the different keys in the entity (we want to find the classname to check to see if the type is one we care about)
					for( int PropIdx = 0; PropIdx < Entity->Values.size(); PropIdx++ )
					{
						const string& Key = Entity->Values[PropIdx].Key;
						const string& Value = Entity->Values[PropIdx].Value;

						// Find the classname property
						if( Key == "classname" )
						{
							// Is it a type that we don't care about?
							for( int CullIdx = 0; CullIdx < BSP->m_lstCullEntities.size(); CullIdx++ )
							{
								// Cull light entities by default (they'll go into a different file)
								const string& CullType = BSP->m_lstCullEntities[CullIdx];
								if( Value == CullType || Value == "light" || Value == "light_spot" )
								{
									bCullEntity = true;
									break;
								}
							}

							// There's only one classname per entity, so we can skip the rest of the properties
							break;
						}
					}

					if( bCullEntity == false )
					{
						// Keep track of the number of entities written
						++WriteCount;

						for( int PropIdx = 0; PropIdx < Entity->Values.size(); PropIdx++ )
						{
							fprintf( F, "%s - %s\n", Entity->Values[PropIdx].Key.c_str(), Entity->Values[PropIdx].Value.c_str() );
						}
						fprintf( F, "\n" );
					}
				}
			}

			// Verbose
			VLOG( "   Wrote %d entities to file\n", WriteCount );
		}
		fclose(F);
	}

	// Dump light data
	Utility::MakeUtilityPath( BSP->Filename, "_lights.txt", Filename );
	if( Filename.size() > 0 )
	{
		fopen_s( &F, Filename.c_str(), "w+b" );
		if( F )
		{
			int WriteCount = 0;

			// Dump the list of entities
			for( int Idx = 0; Idx < BSP->m_lstEntities.size(); Idx++ )
			{
				EntityData* Entity = BSP->m_lstEntities[Idx];
				if( Entity )
				{
					bool bCullEntity = true;

					// Iterate over all the different keys in the entity (we want to find the classname to check to see if the type is one we care about)
					for( int PropIdx = 0; PropIdx < Entity->Values.size(); PropIdx++ )
					{
						const string& Key = Entity->Values[PropIdx].Key;
						const string& Value = Entity->Values[PropIdx].Value;

						// Find the classname property
						if( Key == "classname" && (Value == "light" || Value == "light_spot") )
						{
							bCullEntity = false;
							break;
						}
					}

					if( bCullEntity == false )
					{
						// Keep track of the number of entities written
						++WriteCount;

						for( int PropIdx = 0; PropIdx < Entity->Values.size(); PropIdx++ )
						{
							fprintf( F, "%s - %s\n", Entity->Values[PropIdx].Key.c_str(), Entity->Values[PropIdx].Value.c_str() );
						}
						fprintf( F, "\n" );
					}
				}
			}

			// Verbose
			VLOG( "   Wrote %d light entities to file\n", WriteCount );
		}
		fclose(F);
	}

	return true;
}

/** Exports the list of static props */
bool BSPExporter::ExportStaticPropList( BSPFile *BSP )
{
	FILE *F;
	string Filename;

	Utility::MakeStaticPropListPath( BSP->Filename, Filename );
	if( Filename.size() > 0 )
	{
		fopen_s( &F, Filename.c_str(), "w+b" );
		if( F )
		{
			// Dump the static prop name table
			for( int Idx = 0; Idx < BSP->m_lstStaticPropNameTable.size(); Idx++ )
			{
				const string &StaticPropName = BSP->m_lstStaticPropNameTable[Idx];
				fprintf( F, "%s\n", StaticPropName.c_str() );
			}

			// Verbose
			VLOG( "   Wrote %d static props to file\n", BSP->m_lstStaticPropNameTable.size() );
		}
		fclose(F);
	}

	return true;
}

/** Creates a FbxMesh element for export to the scene */
FbxMesh *BSPExporter::CreateFbxMesh( BSPFile* BSP, const string& MeshName )
{
	if( BSP )
	{
		// Create the fbxmesh
		FbxMesh *ExportMesh = FbxMesh::Create( g_pFbxScene, MeshName.c_str() );

		// If layer 0 doesn't exist yet... create it
		FbxLayer *Layer = ExportMesh->GetLayer(0);
		if( Layer == 0 )
		{
			ExportMesh->CreateLayer();
			Layer = ExportMesh->GetLayer(0);
		}

		// Create a normal layer
		FbxLayerElementNormal *NormalLayer = FbxLayerElementNormal::Create( ExportMesh, "NormalLayer" );
		// Define normals per control point (vertex)
		NormalLayer->SetMappingMode( FbxLayerElement::eByControlPoint );
		// Set the reference mode of the layer (so the nth layer element maps to the nth control point element)
		NormalLayer->SetReferenceMode( FbxLayerElement::eDirect );

		// Add the locators node
		FbxNode *Markers = FbxNode::Create( g_pFbxScene, "markers" );
		g_pFbxScene->GetRootNode()->AddChild( Markers );

		for( int FaceIdx = 0; FaceIdx < BSP->m_lstFaces.size(); FaceIdx++ )
		{
			dface_bsp17_t *FaceInfo = BSP->m_lstFaces[FaceIdx];
			if( FaceInfo )
			{
				// Initialize the face normal
				FbxVector4 FaceNormal(0,0,1);

				// Look up the plane information for this face
				dplane_t *PlaneInfo = BSP->m_lstPlanes[FaceInfo->planenum];
				if( PlaneInfo )
				{
					FaceNormal.Set( PlaneInfo->normal.x, PlaneInfo->normal.y, PlaneInfo->normal.z );
				}

				vector<int> Indices;

				// Get the vertices from the edges
				for( int SurfEdgeIdx = 0; SurfEdgeIdx < FaceInfo->numedges; SurfEdgeIdx++ )
				{
					int EdgeIdx = *BSP->m_lstSurfEdges[FaceInfo->firstedge + SurfEdgeIdx];
					bool bBackward = EdgeIdx < 0;

					dedge_t *Edge = BSP->m_lstEdges[abs(EdgeIdx)];
					if( Edge )
					{
						if( bBackward )
						{
							// Check for duplication of points
							if( Indices.size() == 0 || Indices[Indices.size()-1] != Edge->v[1] )
							{
								Indices.push_back( Edge->v[1] );
							}

							// Check for wrapping around the end, also check to make sure that we don't have duplicate points
							// for the same segment
							if( Indices[0] != Edge->v[0] && Indices[Indices.size()-1] != Edge->v[0] )
							{
								Indices.push_back( Edge->v[0] );
							}
						}
						else
						{
							// Check for duplication of points
							if( Indices.size() == 0 || Indices[Indices.size()-1] != Edge->v[0] )
							{
								Indices.push_back( Edge->v[0] );
							}

							// Check for wrapping around the end, also check to make sure that we don't have duplicate points
							// for the same segment
							if( Indices[0] != Edge->v[1] && Indices[Indices.size()-1] != Edge->v[1] )
							{
								Indices.push_back( Edge->v[1] );
							}
						}
					}
				}

				// Start a new polygon
				ExportMesh->BeginPolygon();

				// Retrieve the current number of control points and increment it by the polygon size
				int NumVerts = ExportMesh->GetControlPointsCount();
				ExportMesh->SetControlPointCount( NumVerts + Indices.size() );

				dface_bsp17_t *OrigFaceInfo = BSP->m_lstOrigFaces[FaceInfo->origFace];
				texinfo_t *TexInfo = BSP->m_lstTexInfo[OrigFaceInfo->texinfo];
				dtexdata_t *TexData = BSP->m_lstTextureData[TexInfo->texdata];
				string TexName = BSP->m_lstStringTable[TexData->nameStringTableID];
				Utility::FindAndReplace( TexName, "\\", "_" );
				Utility::FindAndReplace( TexName, "/", "_" );

				// Set the control point and normals
				//for( int InIdx = 0; InIdx < Indices.size(); InIdx++ )
				for( int InIdx = Indices.size()-1; InIdx >= 0; InIdx-- )
				{
					dvertex_t *Vert = BSP->m_lstVertices[Indices[InIdx]];
					if( Vert )
					{
						// Add the vertex and normal
						const FbxVector4 v( Vert->point.x, Vert->point.y, Vert->point.z );
						ExportMesh->SetControlPointAt( v, NumVerts + InIdx );
						ExportMesh->SetControlPointNormalAt( FaceNormal, NumVerts + InIdx );

						// Add the polygon in order
						ExportMesh->AddPolygon( NumVerts + InIdx );

						if( InIdx == 0 )
						{
							// Create a marker for this face
							FbxMarker *NewMarker = FbxMarker::Create( g_pFbxScene, (TexName + "_marker").c_str() );
							NewMarker->Look.Set( FbxMarker::eHardCross );
							FbxNode *NewNode = FbxNode::Create( g_pFbxScene, TexName.c_str() );
							NewNode->LclTranslation.Set( v );
							NewNode->SetNodeAttribute( NewMarker );
							Markers->AddChild( NewNode );
						}
					}
				}

				// Done the polygon
				ExportMesh->EndPolygon();
			}
		}

		Layer->SetNormals( NormalLayer );
		return ExportMesh;
	}

	return 0;
}

/** Creates a new FbxScene */
bool BSPExporter::CreateFbxScene()
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
bool BSPExporter::DestroyFbxScene()
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
bool BSPExporter::ResetFbxScene()
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
bool BSPExporter::ConvertFbxScene()
{
	// Change the units
	FbxSystemUnit:: Inch.ConvertScene( g_pFbxScene );

	// Change the coordinate system
	FbxAxisSystem::MayaYUp.ConvertScene( g_pFbxScene );

	return true;
}