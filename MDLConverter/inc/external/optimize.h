//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#ifdef _WIN32
#pragma once
#endif

#include "external\studio.h"

// NOTE: You can change this without affecting the vtx file format.
#define MAX_NUM_BONES_PER_TRI ( MAX_NUM_BONES_PER_VERT * 3 )
#define MAX_NUM_BONES_PER_STRIP 512

#define OPTIMIZED_MODEL_FILE_VERSION 7

extern bool g_bDumpGLViewFiles;

struct s_bodypart_t;

namespace OptimizedModel
{

#pragma pack(1)

struct BoneStateChangeHeader_t
{
	int hardwareID;
	int newBoneID;
};

// - BKH - Sep 9, 2012 - Adjusting properties in here...
struct Vertex_t
{
	unsigned short origMeshVertID;

	// these index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
	unsigned char numBones;
	
	// for sw skinned verts, these are indices into the global list of bones
	// for hw skinned verts, these are hardware bone indices
	char boneID[MAX_NUM_BONES_PER_VERT];
	char unknown[3];
};

enum StripHeaderFlags_t {
	STRIP_IS_TRILIST	= 0x01,
	STRIP_IS_TRISTRIP	= 0x02
};

// a strip is a piece of a stripgroup that is divided by bones 
// (and potentially tristrips if we remove some degenerates.)
struct StripHeader_t
{
	// indexOffset offsets into the mesh's index array.
	short numIndices;
	short indexOffset;

	// vertexOffset offsets into the mesh's vert array.
	short numVerts;
	short vertOffset;

	// use this to enable/disable skinning.  
	// May decide (in optimize.cpp) to put all with 1 bone in a different strip 
	// than those that need skinning.
	unsigned char flags; 

	char spacing[7];
	
	//unsigned char flags;
	//
	//int numBoneStateChanges;
	//int boneStateChangeOffset;
	//inline BoneStateChangeHeader_t *pBoneStateChange( int i ) const 
	//{ 
	//	return (BoneStateChangeHeader_t *)(((byte *)this) + boneStateChangeOffset) + i; 
	//};
};

enum StripGroupFlags_t 
{
	STRIPGROUP_IS_FLEXED		= 0x01,
	STRIPGROUP_IS_HWSKINNED		= 0x02,
	STRIPGROUP_IS_DELTA_FLEXED	= 0x04,
	STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
};

// a locking group
// a single vertex buffer
// a single index buffer
struct StripGroupHeader_t
{
	// - BKH - Sep 9, 2012 - Some additional significant changes to this structure to match the disk format
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	short numVerts;
	short vertOffset;
	inline unsigned short *pVertex( int i ) const
	{
		return (unsigned short *)(((byte *)this) + vertTableOffset) + i;
	};
	inline Vertex_t *pVertex2( int i ) const 
	{ 
		return (Vertex_t *)(((byte *)this) + vertTableOffset + 10) + i;
	};

	short numStrips;
	short padding;
	inline StripHeader_t *pStrip( int i ) const 
	{ 
		return (StripHeader_t *)(((byte *)this) + stripOffset) + i; 
	};

	int vertTableOffset;
	int indexOffset;
	//short numIndices;
	//short indexOffset;
	inline unsigned short *pIndex( int i ) const 
	{ 
		return (unsigned short *)(((byte *)this) + indexOffset) + i; 
	};

	int stripOffset;

	// - BKH - Sep 8, 2012 - I'm not sure what this is... but the structure needs to be 24 bytes (and the verts and strips seem to line up)
	//short Unknown[2];
	//int Q;
	//int UnknownB;

	// - BKH - Sep 8, 2012 - Adjusting alignment to fit disk structures
	//unsigned char flags;
};

enum MeshFlags_t { 
	// these are both material properties, and a mesh has a single material.
	MESH_IS_TEETH	= 0x01, 
	MESH_IS_EYES	= 0x02
};

// a collection of locking groups:
// up to 4:
// non-flexed, hardware skinned
// flexed, hardware skinned
// non-flexed, software skinned
// flexed, software skinned
//
// A mesh has a material associated with it.
struct MeshHeader_t
{
	// - BKH - Sep 9, 2012 - numStripGroups -> short, added padding value?
	short numStripGroups;
	short padding;
	int stripGroupHeaderOffset;
	inline StripGroupHeader_t *pStripGroup( int i ) const 
	{ 
		StripGroupHeader_t *pDebug = (StripGroupHeader_t *)(((byte *)this) + stripGroupHeaderOffset) + i; 
		return pDebug;
	};
	// - BKH - Sep 8, 2012 - Adjusting alignment to fit disk structures
	//unsigned char flags;
};

struct ModelLODHeader_t
{
	int numMeshes;
	int meshOffset;
	float switchPoint;
	inline MeshHeader_t *pMesh( int i ) const 
	{ 
		MeshHeader_t *pDebug = (MeshHeader_t *)(((byte *)this) + meshOffset) + i; 
		return pDebug;
	};
};

// This maps one to one with models in the mdl file.
// There are a bunch of model LODs stored inside potentially due to the qc $lod command
struct ModelHeader_t
{
	int numLODs; // garymcthack - this is also specified in FileHeader_t
	int lodOffset;
	inline ModelLODHeader_t *pLOD( int i ) const 
	{ 
		ModelLODHeader_t *pDebug = ( ModelLODHeader_t *)(((byte *)this) + lodOffset) + i; 
		return pDebug;
	};
};

struct BodyPartHeader_t
{
	int numModels;
	int modelOffset;
	inline ModelHeader_t *pModel( int i ) const 
	{ 
		ModelHeader_t *pDebug = (ModelHeader_t *)(((byte *)this) + modelOffset) + i;
		return pDebug;
	};
};

struct MaterialReplacementHeader_t
{
	short materialID;
	int replacementMaterialNameOffset;
	inline const char *pMaterialReplacementName( void )
	{
		const char *pDebug = (const char *)(((byte *)this) + replacementMaterialNameOffset); 
		return pDebug;
	}
};

struct MaterialReplacementListHeader_t
{
	int numReplacements;
	int replacementOffset;
	inline MaterialReplacementHeader_t *pMaterialReplacement( int i ) const
	{
		MaterialReplacementHeader_t *pDebug = ( MaterialReplacementHeader_t *)(((byte *)this) + replacementOffset) + i; 
		return pDebug;
	}
};

struct FileHeader_t
{
	// file version as defined by OPTIMIZED_MODEL_FILE_VERSION
	int version;

	// hardware params that affect how the model is to be optimized.
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerTri;
	int maxBonesPerVert;

	// must match checkSum in the .mdl
	long checkSum;
	
	int numLODs; // garymcthack - this is also specified in ModelHeader_t and should match

	// one of these for each LOD
	int materialReplacementListOffset;
	MaterialReplacementListHeader_t *pMaterialReplacementList( int lodID ) const
	{ 
		MaterialReplacementListHeader_t *pDebug = 
			(MaterialReplacementListHeader_t *)(((byte *)this) + materialReplacementListOffset) + lodID;
		return pDebug;
	}

	int numBodyParts;
	int bodyPartOffset;
	inline BodyPartHeader_t *pBodyPart( int i ) const 
	{
		BodyPartHeader_t *pDebug = (BodyPartHeader_t *)(((byte *)this) + bodyPartOffset) + i;
		return pDebug;
	};	
};

#pragma pack()

}; // namespace OptimizedModel

#endif // OPTIMIZE_H
