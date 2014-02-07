#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif
#include <fbxsdk.h>

#include <assert.h>

#include "external\studio.h"
#include "external\optimize.h"

#include "MDLParser.h"
#include "MDLFile.h"
#include "MDLExporter.h"
#include "VTFFile.h"
#include "Utility.h"
#include "Logging.h"

extern FbxScene *g_pFbxScene;

using namespace std;
using namespace OptimizedModel;

/** Exports all of the mesh data to the FBX scene */
bool MDLExporter::ExportMeshes( MDLFile *MDL, bool bExportSkin )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	FileHeader_t *VTXHdr = MDL ? MDL->GetVTXHeader() : 0;
	if( HDR && VTXHdr )
	{
		// STOLEN COMMENT
			// meshes are deeply hierarchial, divided between three stores, follow the white rabbit
			// body parts -> models -> lod meshes -> strip groups -> strips
			// the vertices and indices are pooled, the trick is knowing the offset to determine your indexed base 

		// Verbose
		VLOG( "  Found %d part(s)\n", HDR->NumBodyParts );

		// Iterate over the body parts and look up the models
		for( int Idx = 0; Idx < HDR->NumBodyParts; Idx++ )
		{
			// Walk through both buffers...
			BodyPartHeader_t *VtxBodyPart = VTXHdr->pBodyPart( Idx );
			StudioBodyPart *Part = HDR->GetBodyPart( Idx );

			// Verbose
			VLOG( "   Found %d model(s)\n", Part->NumModels );

			// Look up the vertex data for each model
			for( int ModelIdx = 0; ModelIdx < Part->NumModels; ModelIdx++ )
			{
				ModelHeader_t *VtxModel = VtxBodyPart->pModel( ModelIdx );
				StudioModel *Model = Part->GetModel( ModelIdx );

				FbxNode *GeometryNode = FbxNode::Create( g_pFbxScene, "geometry" );
				g_pFbxScene->GetRootNode()->AddChild( GeometryNode );

				// Assume LOD0 (don't really care about other LODs)
				int nLod = 0;
				ModelLODHeader_t *VtxLOD = VtxModel->pLOD( nLod );

				// Verbose
				VLOG( "   Found %d mesh(es)\n", Model->NumMeshes );

				for( int MeshIdx = 0; MeshIdx < Model->NumMeshes; MeshIdx++ )
				{
					StudioMesh *Mesh = Model->GetMesh( MeshIdx );
					MeshHeader_t *VtxMesh = VtxLOD->pMesh( MeshIdx );

					// Create the export mesh
					FbxMesh *ExportMesh = CreateFbxMesh( MDL, "mesh", Mesh );

					// Unique name for the node
					char PartName[128];
					sprintf_s( PartName, 128, "mesh%02d", MeshIdx );
					
					// Create the node and set the mesh
					FbxNode *Node = FbxNode::Create( g_pFbxScene, PartName );
					Node->SetNodeAttribute( ExportMesh );

					// Add the node to the geometry node
					GeometryNode->AddChild( Node );

					// Now that we have the node set up, create a material for it
					char MatName[64];
					sprintf_s( MatName, 64, "material%02d", MeshIdx );

					// Create an export material for this mesh
					FbxSurfacePhong *Material = CreateFbxMaterial( MatName, Mesh );
					Node->AddMaterial( Material );

					// Create the material
					FbxFileTexture *Texture = CreateFbxTexture( MDL, ExportMesh, Mesh );

					// Connect the texture to the material
					Material->Diffuse.ConnectSrcObject( Texture );
					Node->SetShadingMode( FbxNode::eTextureShading );

					// Only create the skin for this model if we've specified the flag
					if( bExportSkin )
					{
						// Create a skin for this mesh
						FbxSkin *Skin = CreateFbxSkin( MDL, Mesh );
						if( Skin )
						{
							// Get the global transform for the root this mesh piece
							FbxAMatrix Mat = Node->EvaluateGlobalTransform();

							// Set up the global matrices for each of the clusters on the skin
							for( int ClusterIdx = 0; ClusterIdx < Skin->GetClusterCount(); ClusterIdx++ )
							{
								FbxCluster *Cluster = Skin->GetCluster( ClusterIdx );
								if( Cluster )
								{
									Cluster->SetTransformMatrix( Mat );
								}
							}

							// Add the skin as a deformer for this mesh
							ExportMesh->AddDeformer( Skin );
						}
					}

					// Verbose
					VLOG( "     Found %d triangle strip group(s) in Mesh(%d)\n", VtxMesh->numStripGroups, MeshIdx );

					// Find the strip groups (they have all the data about the ordering of vertices for the polys)
					for( int SGIdx = 0; SGIdx < VtxMesh->numStripGroups; SGIdx++ )
					{
						StripGroupHeader_t *StripGroup = VtxMesh->pStripGroup( SGIdx );

						// Pull out the strip information
						for( int StripIdx = 0; StripIdx < StripGroup->numStrips; StripIdx++ )
						{
							StripHeader_t *Strip = StripGroup->pStrip( StripIdx );
							if( Strip )
							{
								int NumIndices = Strip->numIndices;

								// cDataOffset == pStripGroup->indexOffset (offset from the strip group header to the start of the indice table)
								// cHeaderOffset == pStripGroup->vertTableOffset (offset from the strip group header to the start of the vertex table)
								// cDataSubSetStart == pStrip->indexOffset (offset into the indice table)

								for( int IIndex = 0; IIndex < NumIndices; IIndex += 3 )
								{
									// Look up the index
									unsigned short *pIndice1 = StripGroup->pIndex( IIndex + 0 + Strip->indexOffset );
									unsigned short *pIndice2 = StripGroup->pIndex( IIndex + 1 + Strip->indexOffset );
									unsigned short *pIndice3 = StripGroup->pIndex( IIndex + 2 + Strip->indexOffset );

									// look up the vertices
									if( (HDR->Flags & STUDIOHDR_FLAGS_STATIC_PROP) != 0 )
									{
										unsigned short *pVertex1, *pVertex2, *pVertex3;
										
										pVertex1 = StripGroup->pVertex( (*pIndice1) );
										pVertex2 = StripGroup->pVertex( (*pIndice2) );
										pVertex3 = StripGroup->pVertex( (*pIndice3) );

										// Make me a triangle!
										ExportMesh->BeginPolygon();

										// - BKH - Sep 10, 2012
										// Changed polygon draw order for Maya (3, 2, 1) instead of (1, 2, 3), fixes inverted surface issues
										ExportMesh->AddPolygon( (*pVertex1) );
										ExportMesh->AddPolygon( (*pVertex3) );
										ExportMesh->AddPolygon( (*pVertex2) );

										ExportMesh->EndPolygon();
									}
									else
									{
										Vertex_t *pVertex1, *pVertex2, *pVertex3;

										pVertex1 = StripGroup->pVertex2( (*pIndice1) );
										pVertex2 = StripGroup->pVertex2( (*pIndice2) );
										pVertex3 = StripGroup->pVertex2( (*pIndice3) );

										// Make me a triangle!
										ExportMesh->BeginPolygon();

										// - BKH - Sep 10, 2012
										// Changed polygon draw order for Maya (3, 2, 1) instead of (1, 2, 3), fixes inverted surface issues
										ExportMesh->AddPolygon( pVertex3->origMeshVertID );
										ExportMesh->AddPolygon( pVertex2->origMeshVertID );
										ExportMesh->AddPolygon( pVertex1->origMeshVertID );

										ExportMesh->EndPolygon();
									}
								}
							}
						}
					}

					// Done processing all the strips in the mesh
				}
			}
		}
	}

	return true;
}

/** Creates a FbxMesh element for export to the scene */
FbxMesh *MDLExporter::CreateFbxMesh( MDLFile* MDL, const string& MeshName, StudioMesh* MeshData )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Create the fbxmesh
		FbxMesh *ExportMesh = FbxMesh::Create( g_pFbxScene, MeshName.c_str() );

		// Set up the # of vertices on the export mesh
		ExportMesh->InitControlPoints( MeshData->NumVertices );

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

		// Verbose
		VLOG( "    Caching %d total vertices\n", MeshData->NumVertices );

		StudioModel *Model = MeshData->GetModel();
		if( Model )
		{
			StudioVertex *pVert1 = 0;
			StudioVertex2 *pVert2 = 0;
			StudioVertex3 *pVert3 = 0;

			// Add a warning for now
			if( Model->VertexListType == VLIST_TYPE_UNSKINNED )
			{
				VLOGWARN( "Warning! Currently vertex type 1 is not fully supported - normal information cannot be retrieved (cleanup may be required in your DCC package)\n" );
			}

			// Loop over all the vertices and add them to the export mesh
			for( int VertIdx = 0; VertIdx < MeshData->NumVertices; VertIdx++ )
			{
				// Look up the skinned vertex and set the control point on the export mesh
				if( Model->VertexListType == VLIST_TYPE_SKINNED )
				{
					// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
					// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
					pVert1 = Model->GetSkinnedVertex( VertIdx + MeshData->VertexOffset );

					// The indexes on the mesh for vertices will always start at 0 though!
					FbxVector4 Vert( pVert1->VecPosition.x, pVert1->VecPosition.y, pVert1->VecPosition.z );
					ExportMesh->SetControlPointAt( Vert, VertIdx );

					FbxVector4 Norm( pVert1->VecNormal.x, pVert1->VecNormal.y, pVert1->VecNormal.z );
					NormalLayer->GetDirectArray().Add( Norm );
				}
				// Look up the unskinned vertex and set the control point on the export mesh
				else if( Model->VertexListType == VLIST_TYPE_UNSKINNED )
				{
					// For unskinned data sets, the stored positions are all scalar values that are applied to the bounding hull
					// we then compute the actual values from the HullMin (verts) and HullMax (normals)

					// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
					// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
					pVert2 = Model->GetUnSkinnedVertex( VertIdx + MeshData->VertexOffset );

					// The indexes on the mesh for vertices will always start at 0 though!
					FbxVector4 Vert( (pVert2->PosX / 65535.f) * HDR->HullMin.x, (pVert2->PosY / 65535.f) * HDR->HullMin.y, (pVert2->PosZ / 65535.f) * HDR->HullMin.z );
					ExportMesh->SetControlPointAt( Vert, VertIdx );

					//FbxVector4 Norm( (pVert2->NormX / 65535.f) * HDR->HullMax.x, (pVert2->NormY / 65535.f) * HDR->HullMax.y, (pVert2->NormZ / 65535.f) * HDR->HullMax.z );
					//NormalLayer->GetDirectArray().Add( Norm );
				}
				// Look up the compressed vertex and set the control point on the export mesh
				else if( Model->VertexListType == VLIST_TYPE_COMPRESSED )
				{
					// For compressed data sets, the stored positions are all 0-255 (scalar) values that are applied to the bounding hull
					// we then compute the actual values from the HullMin (verts) and HullMax (normals)

					// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
					// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
					pVert3 = Model->GetCompressedVertex( VertIdx + MeshData->VertexOffset );

					FbxVector4 Vert( (pVert3->PosX / 255.f) * HDR->HullMin.x, (pVert3->PosY / 255.f) * HDR->HullMin.y, (pVert3->PosZ / 255.f) * HDR->HullMin.z );
					ExportMesh->SetControlPointAt( Vert, VertIdx );

					// The normal data seems to be all messed up - not sure why, but we don't really need vertex normals as it will default to a reasonable
					// value in Maya or most likely any other DCC
					// - BKH - Feb 1, 2013
					//FbxVector4 Norm( (pVert3->NormX / 255.f) * HDR->HullMax.x, (pVert3->NormY / 255.f) * HDR->HullMax.y, (pVert3->NormZ / 255.f) * HDR->HullMax.z );
					//NormalLayer->GetDirectArray().Add( Norm );
				}
				else
				{
					assert( 0 && "Error! Cannot parse vertex list, unsupported format?" );
					exit( -1 );
				}
			}

			Layer->SetNormals( NormalLayer );
		}

		return ExportMesh;
	}

	return 0;
}

/** Creates a FbxMaterial for export to the FBX scene */
FbxSurfacePhong *MDLExporter::CreateFbxMaterial( const string &MatName, StudioMesh *MeshData )
{
	// Create the new material
	FbxSurfacePhong *NewMaterial = FbxSurfacePhong::Create( g_pFbxScene, MatName.c_str() );
	if( NewMaterial )
	{
		// Default emissive...
		NewMaterial->Emissive.Set( FbxDouble3(0.f) );
		// Ambient channel...
		NewMaterial->Ambient.Set( FbxDouble3(0.f) );
		// Diffuse channel...
		NewMaterial->Diffuse.Set( FbxDouble3(0.5f) );
		// Transparency
		NewMaterial->TransparencyFactor.Set( 0.f );

		// Shader
		NewMaterial->ShadingModel.Set( "Phong" );

		// Shininess
		NewMaterial->Shininess.Set( 0.5f );

		return NewMaterial;
	}

	return 0;
}

/** Creates a FbxFileTexture for export to the FBX scene */
FbxFileTexture *MDLExporter::CreateFbxTexture( MDLFile *MDL, FbxMesh *ExportMesh, StudioMesh *MeshData )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Look up the specific model material
		MDLMaterial *Mat = MDL->Materials[MeshData->Material];
		if( Mat && Mat->m_lstTextureNames.size() > 0 )
		{
			FbxGeometryElementUV *UVLayer = ExportMesh->CreateElementUV( "DiffuseUV" );
			UVLayer->SetMappingMode( FbxGeometryElement::eByControlPoint );
			UVLayer->SetReferenceMode( FbxGeometryElement::eDirect );

			FbxFileTexture *NewTexture = FbxFileTexture::Create( g_pFbxScene, "Diffuse_Texture" );

			// Make a texture name
			string TextureName;
			Utility::MakeTextureOutputPath( MDL->Filename, Mat->m_lstTextureNames[0], TextureName );

			// Set the absolute path name
			NewTexture->SetFileName( TextureName.c_str() );
			// Standard texture (not a lightmap, spherical map, bump map... etc)
			NewTexture->SetTextureUse( FbxTexture::eStandard );
			// UV-Mapped texture
			NewTexture->SetMappingType( FbxTexture::eUV );
			// Default material or material applied to this mesh
			NewTexture->SetMaterialUse( FbxFileTexture::eModelMaterial );

			// Verbose
			VLOG( "    Caching %d total uv coords\n", MeshData->NumVertices );

			StudioModel *Model = MeshData->GetModel();
			if( Model )
			{
				// Loop over all the vertices and add them to the export mesh
				for( int VertIdx = 0; VertIdx < MeshData->NumVertices; VertIdx++ )
				{
					// Look up the skinned vertex and set the control point on the export mesh
					if( Model->VertexListType == VLIST_TYPE_SKINNED )
					{
						// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
						// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
						StudioVertex const *StudioVert = Model->GetSkinnedVertex( VertIdx + MeshData->VertexOffset );

						// The indexes on the mesh for texcoords will always start at 0 though!
						// - BKH - Sep 15, 2012 - Note, since we flip the textures on the output we need to invert the y value
						FbxVector2 const UV( StudioVert->VecTexCoord.x, 1.f - StudioVert->VecTexCoord.y );
						UVLayer->GetDirectArray().Add( UV );
					}
					// Look up the unskinned vertex and set the control point on the export mesh
					else if( Model->VertexListType == VLIST_TYPE_UNSKINNED )
					{
						// For unskinned data sets, the stored positions are all scalar values that are applied to the bounding hull
						// we then compute the actual values from the HullMin (verts) and HullMax (normals)

						// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
						// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
						StudioVertex2 const *StudioVert = Model->GetUnSkinnedVertex( VertIdx + MeshData->VertexOffset );

						// The indexes on the mesh for texcoords will always start at 0 though!
						// - BKH - Sep 15, 2012 - AS A NOTE - This seems wrong... tex coords are the same as the normal values?
						// - BKH - Sep 15, 2012 - Note, since we flip the textures on the output we need to invert the y value
						FbxVector2 const UV( (StudioVert->NormY / 65535.f), 1.f - (StudioVert->NormZ / 65535.f) );
						UVLayer->GetDirectArray().Add( UV );
					}
					// Look up the compressed vertex and set the control point on the export mesh
					else if( Model->VertexListType == VLIST_TYPE_COMPRESSED )
					{
						// For compressed data sets, the stored positions are all 0-255 (scalar) values that are applied to the bounding hull
						// we then compute the actual values from the HullMin (verts) and HullMax (normals)

						// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
						// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
						StudioVertex3 const *StudioVert = Model->GetCompressedVertex( VertIdx + MeshData->VertexOffset );

						// - BKH - Sep 15, 2012 - Note, since we flip the textures on the output we need to invert the y value
						FbxVector2 const UV( (StudioVert->TexX / 255.f), 1.f - (StudioVert->TexY / 255.f) );
						UVLayer->GetDirectArray().Add( UV );
					}
					else
					{
						assert( 0 && "Error! Cannot parse vertex list, unsupported format?" );
						exit( -1 );
					}
				}
			}

			return NewTexture;
		}
	}

	return 0;
}

/** Creates a set of clusters for binding to the mesh and bones */
FbxSkin *MDLExporter::CreateFbxSkin( MDLFile *MDL, StudioMesh *MeshData )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		StudioModel *Model = MeshData->GetModel();
		if( Model )
		{
			// Create a new skin for this mesh
			FbxSkin *NewSkin = FbxSkin::Create( g_pFbxScene, "MeshSkin" );
			NewSkin->SetSkinningType( FbxSkin::eDualQuaternion );

			vector<FbxCluster *> Clusters;
			// Create a unique set of clusters for this mesh
			for( unsigned int BoneIdx = 0; BoneIdx < MDL->Bones.size(); BoneIdx++ )
			{
				FbxCluster *Cluster = FbxCluster::Create( g_pFbxScene, "Cluster" );
				Clusters.push_back( Cluster );
			}

			// Loop over all the vertices and add them to the export mesh
			for( int VertIdx = 0; VertIdx < MeshData->NumVertices; VertIdx++ )
			{
				// Look up the skinned vertex and set the control point on the export mesh
				if( Model->VertexListType == VLIST_TYPE_SKINNED )
				{
					// Since we're importing the subset of control points into the mesh... we need to take the vertexoffset into account
					// The actual vertex we get will include the vertex offset (since they're all stored in one big list)
					StudioVertex const *pVert = Model->GetSkinnedVertex( VertIdx + MeshData->VertexOffset );

					// Look up the bone weight information - NumBones doesn't have a value, so just read it all in and ignore any 0 weight values
					const StudioBoneWeight &WeightInfo = pVert->BoneWeights;
					for( int nWeightIdx = 0; nWeightIdx < MAX_NUM_BONES_PER_VERT; nWeightIdx++ )
					{
						float fBoneWeight = WeightInfo.Weight[nWeightIdx] / 255.f;
						if( fBoneWeight > 0.f )
						{
							// Look up the bone and verify that the IDs match
							MDLBone *Bone = MDL->Bones[WeightInfo.Bone[nWeightIdx]];
							assert( Bone && Bone->m_nID == WeightInfo.Bone[nWeightIdx] );

							// Add the control point and weight values to the cluster
							Clusters[Bone->m_nID]->AddControlPointIndex( VertIdx, fBoneWeight );
						}
					}
				}
			}

			// Loop over all the clusters and link them to the nodes and set up the weight mode
			for( int nClusterIdx = 0; nClusterIdx < Clusters.size(); nClusterIdx++ )
			{
				// Is this a valid cluster (did it get any data)
				FbxCluster *Cluster = Clusters[nClusterIdx];

				// We need to specify skinning data, even for clusters with no control points associated with them
				// like the root bone

				// Look up the bone (clusters are 1-1 per bone)
				MDLBone *Bone = MDL->Bones[nClusterIdx];
				if( Bone )
				{
					// Link the cluster to the bone
					Cluster->SetLink( Bone->m_pNode );
					Cluster->SetLinkMode( FbxCluster::eNormalize );

					// Get the link transform for the skeleton node
					FbxAMatrix Mat = Bone->m_pNode->EvaluateGlobalTransform();
					Cluster->SetTransformLinkMatrix( Mat );

					// Add the cluster to the skin
					NewSkin->AddCluster( Cluster );

					// Add the cluster to the skeleton bone list
					Bone->m_lstClusters.push_back( Cluster );
				}
			}

			if( NewSkin->GetClusterCount() > 0 )
			{
				return NewSkin;
			}

			NewSkin->Destroy();
		}
	}

	return 0;
}