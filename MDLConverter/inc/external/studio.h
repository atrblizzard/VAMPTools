#ifndef STUDIO_H
#define STUDIO_H

#ifdef _WIN32
#pragma once
#endif

#include <assert.h>
#include <cmath>
#include <stddef.h>
#include "basicmath.h"

typedef unsigned char byte;

// little-endian "IDSV"
#define MODEL_VERTEX_FILE_ID		(('V'<<24)+('S'<<16)+('D'<<8)+'I')
#define MODEL_VERTEX_FILE_VERSION	4
// this id (IDCV) is used once the vertex data has been compressed (see CMDLCache::CreateThinVertexes)
#define MODEL_VERTEX_FILE_THIN_ID	(('V'<<24)+('C'<<16)+('D'<<8)+'I')

// NOTE!!! : Changing this number also changes the vtx file format!!!!!
#define MAX_NUM_BONES_PER_VERT 3

#define MAX_NUM_LODS 8

// Also defined in MDLParser.h
#define NUMANIMCHANNELS 7

#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
#define STUDIO_ANIM_DELTA	0x10
#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64

// This is set any time the .qc files has $staticprop in it
// Means there's no bones and no transforms
#define STUDIOHDR_FLAGS_STATIC_PROP				( 1 << 4 )

#define	ATTACHMENT_FLAG_WORLD_ALIGN 0x10000

enum StudioVertAnimType_t
{
	STUDIO_VERT_ANIM_NORMAL = 0,
	STUDIO_VERT_ANIM_WRINKLE,
};

struct MDLHeader
{
	char ID[4];
	int Version;

	// this has to be the same in the phy and vtx files to load!
	unsigned int Checksum;

	char Name[128];
	int Length;

	// Don't know if this is a real thing or not, but it's always 0.5, 0.5, 0.5
	Vector GlobalScale;

	// Ideal eye position
	Vector EyePosition;

	// illumination center
	Vector IllumPosition;

	// ideal movement hull size
	Vector HullMin, HullMax; 
	// clipping bounding box
	Vector ViewBBMin, ViewBBMax;

	int Flags;

	// Two Unknown ints
	int Unknown[2];

	// Bones
	int NumBones;
	int BoneIndex;
	struct StudioBone *GetBone( int Index ) const;
	//int					RemapSeqBone( int iSequence, int iLocalBone ) const;	// maps local sequence bone to global bone
	//int					RemapAnimBone( int iAnim, int iLocalBone ) const;		// maps local animations bone to global bone

	// Bone Controllers
	int	NumBoneControllers;
	int	BoneControllerIndex;
	//inline mstudiobonecontroller_t *pBonecontroller( int i ) const { Assert( i >= 0 && i < numbonecontrollers); return (mstudiobonecontroller_t *)(((byte *)this) + bonecontrollerindex) + i; };

	// Hitboxes
	int NumHitBoxSets;
	int HitBoxSetIndex;
	struct StudioHitBoxSet *GetHitBoxSet( int Index ) const;
	//// Calls through to hitbox to determine size of specified set
	//inline mstudiobbox_t *pHitbox( int i, int set ) const 
	//{ 
	//	mstudiohitboxset_t const *s = pHitboxSet( set );
	//	if ( !s )
	//		return NULL;

	//	return s->pHitbox( i );
	//};
	//// Calls through to set to get hitbox count for set
	//inline int			iHitboxCount( int set ) const
	//{
	//	mstudiohitboxset_t const *s = pHitboxSet( set );
	//	if ( !s )
	//		return 0;

	//	return s->numhitboxes;
	//};

	// file local animations? and sequences
		// animations/poses
	int NumLocalAnims;
		// animation descriptions
	int LocalAnimIndex;
	struct StudioAnimDesc* GetLocalAnim( int Index ) const;
	
	// Sequences
	int NumLocalSeq;
	int LocalSeqIndex;
	//inline mstudioseqdesc_t *pLocalSeqdesc( int i ) const { if (i < 0 || i >= numlocalseq) i = 0; return (mstudioseqdesc_t *)(((byte *)this) + localseqindex) + i; };

	// animation node to animation node transition graph
	//private:
	int NumLocalNodes;
	int LocalNodeIndex;
	int LocalNodeNameIndex;
	//inline char			*pszLocalNodeName( int iNode ) const { Assert( iNode >= 0 && iNode < numlocalnodes); return (((char *)this) + *((int *)(((byte *)this) + localnodenameindex) + iNode)); }
	//inline byte			*pLocalTransition( int i ) const { Assert( i >= 0 && i < (numlocalnodes * numlocalnodes)); return (byte *)(((byte *)this) + localnodeindex) + i; };

	////public:
	//int					EntryNode( int iSequence ) const;
	//int					ExitNode( int iSequence ) const;
	//char				*pszNodeName( int iNode ) const;
	//int					GetTransition( int iFrom, int iTo ) const;

	////public:
	//bool				SequencesAvailable() const;
	//int					GetNumSeq() const;
	//mstudioanimdesc_t	&pAnimdesc( int i ) const;
	//mstudioseqdesc_t	&pSeqdesc( int i ) const;
	//int					iRelativeAnim( int baseseq, int relanim ) const;	// maps seq local anim reference to global anim index
	//int					iRelativeSeq( int baseseq, int relseq ) const;		// maps seq local seq reference to global seq index

	////private:
	//mutable int			activitylistversion;	// initialization flag - have the sequences been indexed?
	//mutable int			eventsindexed;
	////public:
	//int					GetSequenceActivity( int iSequence );
	//void				SetSequenceActivity( int iSequence, int iActivity );
	//int					GetActivityListVersion( void ) const;
	//void				SetActivityListVersion( int version ) const;
	//int					GetEventListVersion( void ) const;
	//void				SetEventListVersion( int version ) const;

	// raw textures
	int NumTextures;
	int TextureIndex;
	struct StudioTexture *GetTexture( int Index ) const;
	//inline mstudiotexture_t *pTexture( int i ) const { Assert( i >= 0 && i < numtextures ); return (mstudiotexture_t *)(((byte *)this) + textureindex) + i; }; 

	// raw textures search paths
	int NumTextureSearchPaths;
	int	TextureSearchPathIndex;
	inline char *GetTextureSearchPath( int Index ) const
	{
		return (((char *)this) + *((int *)(((byte *)this) + TextureSearchPathIndex) + Index));
	}

	// replaceable textures tables
	int NumSkinRefs;
	int NumSkinFamilies;
	int	SkinIndex;
	inline short *GetSkinRef( int Index ) const { return (short *)(((byte *)this) + SkinIndex) + Index; }

	int NumBodyParts;
	int	BodyPartIndex;
	struct StudioBodyPart *GetBodyPart( int Index ) const;

	// queryable attachable points
	//private:
	int NumLocalAttachments;
	int	LocalAttachmentIndex;
	struct StudioAttachment *GetLocalAttachment( int Index ) const;
	int GetNumAttachments() const;
	//// used on my tools in hlmv, not persistant
	//void				SetAttachmentBone( int iAttachment, int iBone );

	int NumFlexDescs;
	int	FlexDescIndex;
	//inline mstudioflexdesc_t *pFlexdesc( int i ) const { Assert( i >= 0 && i < numflexdesc); return (mstudioflexdesc_t *)(((byte *)this) + flexdescindex) + i; };

	int NumFlexControllers;
	int	FlexControllerIndex;
	//inline mstudioflexcontroller_t *pFlexcontroller( LocalFlexController_t i ) const { Assert( i >= 0 && i < numflexcontrollers); return (mstudioflexcontroller_t *)(((byte *)this) + flexcontrollerindex) + i; };

	int NumFlexRules;
	int FlexRuleIndex;
	//inline mstudioflexrule_t *pFlexRule( int i ) const { Assert( i >= 0 && i < numflexrules); return (mstudioflexrule_t *)(((byte *)this) + flexruleindex) + i; };

	int NumIKChains;
	int IKChainIndex;
	//inline mstudioikchain_t *pIKChain( int i ) const { Assert( i >= 0 && i < numikchains); return (mstudioikchain_t *)(((byte *)this) + ikchainindex) + i; };

	int NumMouths;
	int MouthIndex;
	//inline mstudiomouth_t *pMouth( int i ) const { Assert( i >= 0 && i < nummouths); return (mstudiomouth_t *)(((byte *)this) + mouthindex) + i; };

	//private:
	int NumLocalPoseParameters;
	int	LocalPoseParamIndex;
	//inline mstudioposeparamdesc_t *pLocalPoseParameter( int i ) const { Assert( i >= 0 && i < numlocalposeparameters); return (mstudioposeparamdesc_t *)(((byte *)this) + localposeparamindex) + i; };
	////public:
	//int					GetNumPoseParameters( void ) const;
	//const mstudioposeparamdesc_t &pPoseParameter( int i ) const;
	//int					GetSharedPoseParameter( int iSequence, int iLocalPose ) const;

	// Key values
	//int	KeyValueIndex;
	//int	KeyValueSize;
	//inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int NumLocalIKAutoplayLocks;
	int	LocalIKAutoplayLockIndex;
	//inline mstudioiklock_t *pLocalIKAutoplayLock( int i ) const { Assert( i >= 0 && i < numlocalikautoplaylocks); return (mstudioiklock_t *)(((byte *)this) + localikautoplaylockindex) + i; };
	//int					GetNumIKAutoplayLocks( void ) const;
	//const mstudioiklock_t &pIKAutoplayLock( int i ) const;
	//int					CountAutoplaySequences() const;
	//int					CopyAutoplaySequences( unsigned short *pOut, int outCount ) const;
	//int					GetAutoplayList( unsigned short **pOut ) const;

	// The collision model mass that jay wanted
	//float Mass;
	//int Contents;

	// implementation specific back pointer to virtual data
	//mutable void		*virtualModel;
	//virtualmodel_t		*GetVirtualModel( void ) const;

	// for demand loaded animation blocks
	int AnimblockNameIndex;
	//inline char * const pszAnimBlockName( void ) const { return ((char *)this) + szanimblocknameindex; }

	int NumFlexControllerUI;
	int FlexControllerUIIndex;
	//mstudioflexcontrollerui_t *pFlexControllerUI( int i ) const { Assert( i >= 0 && i < numflexcontrollerui); return (mstudioflexcontrollerui_t *)(((byte *)this) + flexcontrolleruiindex) + i; }

	// external animations, models, etc.
	int	NumIncludeModels;
	int	IncludeModelIndex;

	int SurfacePropIndex;
	//inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropindex; }

	int BoneTableNameIndex;
	//inline const byte	*GetBoneTableSortedByName() const { return (byte *)this + bonetablebynameindex; }

 	struct StudioModelGroup *GetModelGroup( int Index ) const;
	//// implementation specific call to get a named model
	//const studiohdr_t	*FindModel( void **cache, char const *modelname ) const;

	int NumAnimblocks;
	int AnimblockIndex;
	//inline mstudioanimblock_t *pAnimBlock( int i ) const { Assert( i > 0 && i < numanimblocks); return (mstudioanimblock_t *)(((byte *)this) + animblockindex) + i; };
	//mutable void		*AnimblockModel;
	//byte *				GetAnimBlock( int i ) const;

	// used by tools only that don't cache, but persist mdl's peer data
	// engine uses virtualModel to back link to cache pointers
	//void				*pVertexBase;
	// Pointer to the VTX file data
	//void				*pIndexBase;

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting 
	// on static props
	byte ConstDirectionalLightDOT;

	// set during load of mdl data to track *desired* lod configuration (not actual)
	// the *actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	byte RootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	byte NumAllowedRootLODs;

	// Padding
	byte				unused[1];
	//int					unused4; // zero out if version < 47

	// Padding
	//int					unused3[2];

	// FIXME: Remove when we up the model version. Move all fields of studiohdr2_t into studiohdr_t.
	//int StudioHDR2Index;
	//studiohdr2_t*		pStudioHdr2() const { return (studiohdr2_t *)( ( (byte *)this ) + studiohdr2index ); }

	// Src bone transforms are transformations that will convert .dmx or .smd-based animations into .mdl-based animations
	//int					NumSrcBoneTransforms() const { return studiohdr2index ? pStudioHdr2()->numsrcbonetransform : 0; }
	//const mstudiosrcbonetransform_t* SrcBoneTransform( int i ) const { Assert( i >= 0 && i < NumSrcBoneTransforms()); return (mstudiosrcbonetransform_t *)(((byte *)this) + pStudioHdr2()->srcbonetransformindex) + i; }

	//inline int			IllumPositionAttachmentIndex() const { return studiohdr2index ? pStudioHdr2()->IllumPositionAttachmentIndex() : 0; }

	//inline float		MaxEyeDeflection() const { return studiohdr2index ? pStudioHdr2()->MaxEyeDeflection() : 0.866f; } // default to cos(30) if not set

	//inline mstudiolinearbone_t *pLinearBones() const { return studiohdr2index ? pStudioHdr2()->pLinearBones() : NULL; }

	// NOTE: No room to add stuff? Up the .mdl file format version 
	// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
	// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
	//int					unused2[1];
};

struct StudioModel;
// body part index
struct StudioBodyPart
{
	int	NameIndex;
	inline char* const GetName() { return ((char *)this) + NameIndex; }
	int NumModels;
	int	Base;
	int ModelIndex; // index into models array
	StudioModel *GetModel( int Index ) const;
};

// bones
struct StudioBone // struct size = 160 bytes
{
	int NameIndex;
	inline char * const GetName() { return ((char *)this) + NameIndex; }
	int ParentBone;

	int BoneController[6];

	Vector Position;
	Quaternion Quat;
	float AnimChannels[NUMANIMCHANNELS];

	matrix3x4_t PoseToBone;
	int Flags;
	int ProcType;
	int ProcIndex; // procedural rule
	int PhysicsBone; // index into physically simulated bone
	////inline void *pProcedure( ) const { if (procindex == 0) return 0; else return  (void *)(((byte *)this) + procindex); };
	
	int SurfacePropIndex; // index into string table for property name
	char* const GetSurfacePropName() { return ((char *)this) + SurfacePropIndex; }
	
	int Contents;

	// - BKH - Sep 7, 2012 - Alignment doesn't match - fixing to 32 bytes
	//int					sznameindex;
	//inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	//int		 			parent;		// parent bone
	//int					bonecontroller[6];	// bone controller index, -1 == none

	//// default values
	//Vector				pos;
	//Quaternion			quat;
	//RadianEuler			rot;
	//// compression scale
	//Vector				posscale;
	//Vector				rotscale;

	//matrix3x4_t			poseToBone;
	//Quaternion			qAlignment;
	//int					flags;
	//int					proctype;
	//int					procindex;		// procedural rule
	//mutable int			physicsbone;	// index into physically simulated bone
	//inline void *pProcedure( ) const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	//int					surfacepropidx;	// index into string tablefor property name
	//inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropidx; }
	//int					contents;		// See BSPFlags.h for the contents flags

	//int					unused[8];		// remove as appropriate
};

struct StudioBoneWeight;
struct StudioVertex;
struct StudioVertex2;
struct StudioVertex3;
// UNVERIFIED ALIGNMENT
struct StudioModelVertexData
{
	Vector *Position( int i ) const;
	Vector *Normal( int i ) const;
	Vector4D *TangentS( int i ) const;
	Vector2D *Texcoord( int i ) const;
	StudioBoneWeight *BoneWeights( int i ) const;
	StudioVertex *Vertex( int i ) const;
	bool HasTangentData( void ) const;
	int GetGlobalVertexIndex( int i ) const;
	int GetGlobalTangentIndex( int i ) const;

	// base of external vertex data stores
	const void *pVertexData;
	const void *pTangentData;
};

enum
{
	VLIST_TYPE_SKINNED = 0,
	VLIST_TYPE_UNSKINNED = 1,
	VLIST_TYPE_COMPRESSED = 2,
};

struct StudioMesh;
struct StudioEyeball;
struct VertexFileHeader;
struct ThinModelVertices;
// studio models
struct StudioModel
{
	inline const char* GetName() { return Name; }
	char Name[64];

	int Type;
	float BoundingRadius;

	// These functions are defined in application-specific code:
	VertexFileHeader *CacheVertexData( void *pModelData ) const;

	// Access thin/fat mesh vertex data (only one will return a non-NULL result)
	const StudioModelVertexData *GetVertexData( void *ModelData = 0 );
	const ThinModelVertices*GetThinVertexData(	void *ModelData = 0 );

	int NumAttachments;
	int AttachmentIndex;

	int NumEyeballs;
	int EyeballIndex;
	StudioEyeball *GetEyeball( int Index ) const;

	StudioModelVertexData VertexData;

	// - BKH - Sep 7, 2012 - Increased padding size to adjust for alignment issues
	int unused[10]; // remove as appropriate

	// - BKH - Sep 7, 2012 - Moved some variables around to match alignment on disk
	int NumMeshes;
	int	MeshIndex;
	StudioMesh *GetMesh( int Index ) const;

	// cache purposes
	int NumVertices; // number of unique vertices/normals/texcoords
	int VertexIndex; // vertex Vector
	int TangentsIndex; // tangents Vector

	// - BKH - Sep 9, 2012 - Added access functions to get at the embedded vertices
	int VertexListType;
	StudioVertex *GetSkinnedVertex( int Index ) const;
	StudioVertex2 *GetUnSkinnedVertex( int Index ) const;
	StudioVertex3 *GetCompressedVertex( int Index ) const;
};

// UNVERIFIED ALIGNMENT
struct MeshVertexData
{
	Vector *Position( int i ) const;
	Vector *Normal( int i ) const;
	Vector4D *TangentS( int i ) const;
	Vector2D *Texcoord( int i ) const;
	StudioBoneWeight *BoneWeights( int i ) const;
	StudioVertex *Vertex( int i ) const;
	bool HasTangentData( void ) const;
	int GetModelVertexIndex( int i ) const;
	int GetGlobalVertexIndex( int i ) const;

	// indirection to this mesh's model's vertex data
	const StudioModelVertexData *ModelVertexData;

	// used for fixup calcs when culling top level lods
	// expected number of mesh verts at desired lod
	int NumLODVertexes[MAX_NUM_LODS];
};

struct StudioFlex;
struct StudioMesh
{
	int	Material;

	int ModelIndex;
	StudioModel *GetModel() const; 

	// number of unique vertices/normals/texcoords
	int NumVertices;
	// vertex mstudiovertex_t
	int VertexOffset;

	// Access thin/fat mesh vertex data (only one will return a non-NULL result)
	const MeshVertexData *GetVertexData( void *ModelData = 0 );
	const ThinModelVertices *GetThinVertexData(	void *ModelData = 0 );

	// vertex animation
	int	NumFlexes;
	int FlexIndex;
	StudioFlex *GetFlex( int Index ) const;

	// - BKH - Sep 7, 2012 - Best guess at which vars aren't used?
	// special codes for material operations
	//int MaterialType;
	//int MaterialParam;

	// a unique ordinal for this mesh
	//int MeshID;

	//Vector Center;

	MeshVertexData VertexData;

	// BKH - Offsets don't match in data
	//int	unused[8]; // remove as appropriate
};

// eyeball
// UNVERIFIED ALIGNMENT
struct StudioEyeball
{
	int NameIndex;
	inline char* const GetName() { return ((char *)this) + NameIndex; }
	int Bone;
	Vector Org;
	float ZOffset;
	float Radius;
	Vector Up;
	Vector Forward;
	int Texture;

	int Unused1;
	float Iris_Scale;
	int Unused2;

	int UpperFlexDesc[3];	// index of raiser, neutral, and lowerer flexdesc that is set by flex controllers
	int LowerFlexDesc[3];
	float UpperTarget[3];	// angle (radians) of raised, neutral, and lowered lid positions
	float LowerTarget[3];

	int	UpperLidFlexDesc;	// index of flex desc that actual lid flexes look to
	int LowerLidFlexDesc;
	int Unused[4];			// These were used before, so not guaranteed to be 0
	bool m_bNonFACS;		// Never used before version 44
	char Unused3[3];
	int Unused4[7];
};

// 10 bytes
// UNVERIFIED ALIGNMENT (BKH - Needs export test)
struct StudioBoneWeight
{
	// - BKH - Sep 9, 2012
	// weight values are represented as 0-255
	byte	Weight[MAX_NUM_BONES_PER_VERT];
	short	Bone[MAX_NUM_BONES_PER_VERT]; 
	byte	NumBones;

	//	byte	material;
	//	short	firstref;
	//	short	lastref;
};

// NOTE: This is exactly 42 bytes
// UNVERIFIED ALIGNMENT (BKH - Needs export test)
struct StudioVertex
{
	StudioBoneWeight BoneWeights; // 10 bytes
	Vector VecPosition;
	Vector VecNormal;
	Vector2D VecTexCoord;
};

// - BKH - Sep 9, 2012 - No skinning information for these bones
struct StudioVertex2
{
	// - BKH - Jan 13, 2013 - Updating data... looks like compressed floats are not correct (using shorts which will be converted to fractional values and applied to the hull)
	unsigned short PosX, PosY, PosZ;
	unsigned short NormX, NormY, NormZ;

	// Textures are the same as NormX and NormY????? (This seems obviously wrong)
};

// - BKH - Sep 9, 2012 - Super compressed vertex info (probably only useful for very small objects - too much precision loss otherwise)
struct StudioVertex3
{
	// position values are ratio scalar (0-255) applied to the bounding box
	byte PosX, PosY, PosZ;
	// BKH - Feb 1, 2013 - Reordering UV parameters, maybe it's actually all screwy like this?
	byte NormX, NormY;
	byte TexX;
	byte NormZ;
	byte TexY;
};

// UNVERIFIED ALIGNMENT
struct VertexFileHeader
{
	// MODEL_VERTEX_FILE_ID
	int ID;
	// MODEL_VERTEX_FILE_VERSION
	int Version;
	// same as studiohdr_t, ensures sync
	long Checksum;						
	// num of valid lods
	int	NumLODs;
	// num verts for desired root lod
	int NumLODVertexes[MAX_NUM_LODS];
	// num of vertexFileFixup_t
	int	NumFixups;
	// offset from base to fixup table
	int	FixupTableStart;
	// offset from base to vertex block
	int VertexDataStart;
	// offset from base to tangent block
	int TangentDataStart;

public:

	// Accessor to fat vertex data
	const StudioVertex *GetVertexData() const
	{
		if( ID == MODEL_VERTEX_FILE_ID && VertexDataStart != 0 )
		{
			return (StudioVertex *)( VertexDataStart + (byte *)this );
		}

		return 0;
	}
	// Accessor to (fat) tangent vertex data (tangents aren't stored in compressed data)
	const Vector4D *GetTangentData() const
	{
		if( ID == MODEL_VERTEX_FILE_ID && TangentDataStart != 0 )
		{
			return (Vector4D *)( TangentDataStart + (byte *)this );
		}

		return 0;
	}
	// Accessor to thin vertex data
	const ThinModelVertices *GetThinVertexData() const
	{
		if( ID == MODEL_VERTEX_FILE_THIN_ID && VertexDataStart != 0 )
		{
			return (ThinModelVertices *)( VertexDataStart + (byte *)this );
		}

		return 0;
	}
};

// 'thin' vertex data, used to do model decals (see Studio_CreateThinVertexes())
// UNVERIFIED ALIGNMENT
struct ThinModelVertices
{
	// Number of bone influences per vertex, N
	int NumBoneInfluences;
	// This array stores (N-1) weights per vertex (unless N is zero)
	float *BoneWeights;
	// This array stores N indices per vertex
	char *BoneIndices;
	Vector *VecPositions;
	// Normals are compressed into 16 bits apiece (see PackNormal_UBYTE4() )
	unsigned short *VecNormals;
};

// this is the memory image of vertex anims (16-bit fixed point)
// UNVERIFIED ALIGNMENT
struct StudioVertAnim
{
	unsigned short Index;
	// 255/max_length_in_flex
	byte Speed;
	// 255/left_right
	byte Side;

protected:
	// JasonM changing this type a lot, to prefer fixed point 16 bit...
	union
	{
		short delta[3];
		short flDelta[3];
	};

	union
	{
		short ndelta[3];
		short flNDelta[3];
	};
};

// this is the memory image of vertex anims (16-bit fixed point)
// UNVERIFIED ALIGNMENT
struct StudioVertAnim_Wrinkle : public StudioVertAnim
{
	short WrinkleDelta;
};

// UNVERIFIED ALIGNMENT
struct StudioFlex
{
	// input value
	int FlexDesc;

	float Target0;	// zero
	float Target1;	// one
	float Target2;	// one
	float Target3;	// zero

	int NumVerts;
	int VertIndex;

	inline StudioVertAnim *GetVertAnim( int Index ) const 
	{ 
		assert( VertAnimType == STUDIO_VERT_ANIM_NORMAL ); 
		return (StudioVertAnim *)(((byte *)this) + VertIndex) + Index;
	};

	inline StudioVertAnim_Wrinkle *GetVertWrinkleANim( int Index ) const 
	{ 
		assert( VertAnimType == STUDIO_VERT_ANIM_WRINKLE ); 
		return (StudioVertAnim_Wrinkle *)(((byte *)this) + VertIndex) + Index; 
	};

	inline byte *GetBaseVertAnim() const { return ((byte *)this) + VertIndex; };
	inline int VertAnimSizeBytes() const { return ( VertAnimType == STUDIO_VERT_ANIM_NORMAL ) ? sizeof(StudioVertAnim) : sizeof(StudioVertAnim_Wrinkle); }

	// second flex desc
	int FlexPair;
	// See StudioVertAnimType_t
	unsigned char VertAnimType;
	unsigned char UnusedChar[3];
	int Unused[6];
};

struct StudioAnimDesc
{
	//int BasePtr;
	//inline MDLHeader* const GetBaseHeader() { return (MDLHeader *)(((byte *)this) + BasePtr); }

	int NameIndex;
	inline char* const GetName() { return ((char *)this) + NameIndex; }

	float FPS; // frames per second	
	int Flags; // looping/non-looping flags
	int NumFrames;

	// piecewise movement
	int NumMovements;
	int MovementIndex;
	struct StudioMovement* GetMovementInfo( int Index ) const;

	// - BKH - Sep 7, 2012 - Removed padding, to fit footprint
	//int unused1[6];			// remove as appropriate (and zero if loading older versions)	

	int AnimBlock;
	int AnimIndex;	 // non-zero when anim data isn't in sections
	struct StudioAnim *GetAnimBlock( int Block, int Index ) const; // returns pointer to a specific anim block (local or external)
	struct StudioAnim *GetAnim( int *piFrame, float &flStall ) const; // returns pointer to data and new frame index
	struct StudioAnim *GetAnim( int *piFrame ) const; // returns pointer to data and new frame index

	int NumIKRules;
	int	IKRuleIndex; // non-zero when IK data is stored in the mdl
	int AnimBlockIKRuleIndex; // non-zero when IK data is stored in animblock file
	struct StudioIKRule *GetIKRule( int Index ) const;

	int Value;
	int BoneChannelSetIndex;
	struct StudioAnimBoneChannelSet *GetBoneChannelSet( int Bone ) const;
	//int NumLocalHierarchy;
	//int LocalHierarchyIndex;
	//struct StudioLocalHierarchy* GetHierarchy( int Index ) const;

	int SectionIndex;
	int SectionFrames; // number of frames used in each fast lookup section, zero if not used
	struct StudioAnimSections* GetSection( int Index ) const;

	short ZeroFrameSpan; // frames per span
	short ZeroFrameCount; // number of spans
	int ZeroFrameIndex;
	byte *GetZeroFrameData() const 
	{ 
		if( ZeroFrameIndex )
		{
			return (((byte *)this) + ZeroFrameIndex); 
		}
		return 0; 
	};
	mutable float ZeroFrameStallTime; // saved during read stalls
};

// animation frames
// UNVERIFIED ALIGNMENT
union StudioAnimValue
{
	struct 
	{
		byte Valid;
		byte Total;
	} Num;
	short Value;
};

struct StudioAnim_ValuePtr
{
	short	offset[3];
	inline StudioAnimValue *GetAnimValue( int Index ) const 
	{ 
		if (offset[Index] > 0) 
		{
			return (StudioAnimValue *)(((byte *)this) + offset[Index]);
		}

		return 0; 
	};
};

// per bone per animation DOF and weight pointers
// UNVERIFIED ALIGNMENT
struct StudioAnim
{
	byte Bone;
	byte Flags; // weighing options

	// valid for animating data only
	inline byte *GetData() const 
	{ 
		return (((byte *)this) + sizeof(StudioAnim));
	}

	inline struct StudioAnim_ValuePtr *pRotV() const { return (StudioAnim_ValuePtr *)(GetData()); };
	inline struct StudioAnim_ValuePtr *pPosV() const { return (StudioAnim_ValuePtr *)(GetData()) + ((Flags & STUDIO_ANIM_ANIMROT) != 0); };

	// valid if animation unvaring over timeline
	inline Quaternion48 *GetQuat48() const { return (Quaternion48 *)(GetData()); };
	inline Quaternion64 *GetQuat64( void ) const { return (Quaternion64 *)(GetData()); };
	inline Vector48 *GetPos() const { return (Vector48 *)(GetData() + ((Flags & STUDIO_ANIM_RAWROT) != 0) * sizeof( *GetQuat48() ) + ((Flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof( *GetQuat64() ) ); };

	short NextOffset;
	inline StudioAnim *GetNext() const 
	{ 
		if( NextOffset != 0 ) 
		{
			return (StudioAnim *)(((byte *)this) + NextOffset);
		}

		return 0; 
	}
};

/** - BKH - Sep 7, 2012 - Structure for what looks like root motion, can probably be discarded */
// UNVERIFIED ALIGNMENT
struct StudioMovement
{
	int EndFrame;
	int MotionFlags;
	float v0;			// velocity at start of block
	float v1;			// velocity at end of block
	float angle;		// YAW rotation at end of this blocks movement
	Vector vector;		// movement vector relative to this blocks initial angle
	Vector position;	// relative to start of animation???
};

// attachment
struct StudioAttachment
{
	int NameIndex;
	inline const char *GetName() 
	{ 
		return ((char *)this) + NameIndex; 
	}
	unsigned int		Flags;
	int					LocalBone;
	matrix3x4_t			Local; // attachment point
	// - BKH - Fixing up alignment
	//int					Unused[8];
};

// demand loaded sequence groups
struct StudioModelGroup
{
	int FilenameIndex; // file name
	inline const char* GetFilename() 
	{ 
		return ((char *)this) + FilenameIndex; 
	}

	int LabelIndex; // textual name
	inline const char* GetLabel()
	{ 
		return ((char *)this) + LabelIndex; 
	}

	int Filler[27];
};

// UNVERIFIED ALIGNMENT
struct StudioIKError
{
	Vector pos;
	Quaternion	q;
};

// UNVERIFIED ALIGNMENT
struct StudioCompressedIKError
{
	float	scale[6];
	short	offset[6];
	StudioAnimValue *GetAnimValue( int Index ) const;
};

// UNVERIFIED ALIGNMENT
struct StudioIKRule
{
	int Index;

	int Type;
	int Chain;

	int Bone;

	int Slot; // iktarget slot.  Usually same as chain.
	float Height;
	float Radius;
	float Floor;
	Vector pos;
	Quaternion q;

	int	 CompressedIKErrorIndex;
	inline StudioCompressedIKError *GetCompressedError() const 
	{ 
		return (StudioCompressedIKError *)(((byte *)this) + CompressedIKErrorIndex); 
	};
	int unused2;

	int iStart;
	int IKErrorIndex;
	inline StudioIKError *GetError( int Index ) const 
	{ 
		return  (IKErrorIndex) ? (StudioIKError *)(((byte *)this) + IKErrorIndex) + (Index - iStart) : 0; 
	};

	float Start; // beginning of influence
	float Peak; // start of full influence
	float Tail; // end of full influence
	float End; // end of all influence

	float unused3;	// 
	float Contact;	// frame footstep makes ground concact
	float Drop; // how far down the foot should drop when reaching for IK
	float Top; // top of the foot box

	int unused6;
	int unused7;
	int unused8;

	int AttachmentIndex;		// name of world attachment
	inline char* const GetAttachment() 
	{ 
		return ((char *)this) + AttachmentIndex; 
	}

	int unused[7];
};

// UNVERIFIED ALIGNMENT
struct StudioLocalHierarchy
{
	int iBone; // bone being adjusted
	int iNewParent; // the bones new parent

	float start; // beginning of influence
	float peak; // start of full influence
	float tail; // end of full influence
	float end; // end of all influence

	int iStart; // first frame 

	int LocalAnimIndex;
	inline StudioCompressedIKError *GetLocalAnim() const 
	{ 
		return (StudioCompressedIKError *)(((byte *)this) + LocalAnimIndex); 
	};

	//int unused[4];
};

struct StudioAnimBoneChannel
{
	// How many keys follow?
	unsigned char NumKeys;
	// How many frames are the keys spread across?
	unsigned char NumFrames;
};

struct StudioAnimBoneChannelSet
{
	int ID;
	int Data[NUMANIMCHANNELS];
};

// UNVERIFIED ALIGNMENT
struct StudioAnimSections
{
	int	AnimBlock;
	int AnimIndex;
};

// skin info
struct StudioTexture
{
	int NameIndex;
	inline char * const GetName() { return ((char *)this) + NameIndex; }
	int Flags;
	int Used;
	// BKH - Alignment issues with file format
	//int Unused1;
	mutable void *material; // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	mutable void *clientmaterial; // gary, replace with client material pointer if used
	// BKH - Alignment issues with file format
	//int unused[10];
};

struct StudioHitBoxSet
{
	int NameIndex;
	inline char* const GetName() { return ((char *)this) + NameIndex; }
	int NumHitBoxes;
	int	HitBoxIndex;
	struct StudioBox *GetHitBox( int Index ) const;
};

// intersection boxes
struct StudioBox
{
	int Bone;
	int Group; // intersection group
	Vector BBMin; // bounding box
	Vector BBMax;
	// - BKH - Removed to fix alignment
	//int HitBoxNameIndex;
	//int unused[8];

	//char* GetName()
	//{
	//	if( HitBoxNameIndex == 0 )
	//		return "";

	//	return ((char*)this) + HitBoxNameIndex;
	//}
};

/** Lookup a specific bone */
inline StudioBone *MDLHeader::GetBone( int Index ) const 
{ 
	assert( Index >= 0 && Index < NumBones); 
	return (StudioBone *)(((byte *)this) + BoneIndex) + Index; 
}

inline StudioTexture *MDLHeader::GetTexture( int Index ) const
{
	assert( Index >= 0 && Index < NumTextures );
	return (StudioTexture *)(((byte *)this) + TextureIndex) + Index;
}

inline StudioAttachment *MDLHeader::GetLocalAttachment( int Index ) const 
{ 
	return (StudioAttachment *)(((byte *)this) + LocalAttachmentIndex) + Index; 
}

inline StudioModelGroup *MDLHeader::GetModelGroup( int Index ) const
{
	return (StudioModelGroup *)(((byte *)this) + IncludeModelIndex) + Index;
}

/** Lookup a specific body part */
inline StudioBodyPart *MDLHeader::GetBodyPart( int Index ) const
{
	assert( Index >= 0 && Index < NumBodyParts );
	return (StudioBodyPart *)(((byte *)this) + BodyPartIndex) + Index;
}

/** Lookup a specific anim */
inline StudioAnimDesc* MDLHeader::GetLocalAnim( int Index ) const
{
	assert( Index >= 0 && Index < NumLocalAnims );
	return (StudioAnimDesc *)(((byte *)this) + LocalAnimIndex) + Index; 
}

// Look up hitbox set by index
inline StudioHitBoxSet *MDLHeader::GetHitBoxSet( int Index ) const 
{ 
	assert( Index >= 0 && Index < NumHitBoxSets ); 
	return (StudioHitBoxSet *)(((byte *)this) + HitBoxSetIndex ) + Index;
}

/** Look up a model from a body part */
inline StudioModel *StudioBodyPart::GetModel( int Index ) const
{ 
	return (StudioModel *)(((byte *)this) + ModelIndex) + Index; 
}

inline StudioMesh *StudioModel::GetMesh( int Index ) const
{
	return (StudioMesh *)(((byte *)this) + MeshIndex) + Index;
}

/** Look up an eyeball */
inline StudioEyeball *StudioModel::GetEyeball( int Index ) const
{ 
	return (StudioEyeball *)(((byte *)this) + EyeballIndex) + Index;
}

// model vertex data accessor (defined here so VertexFileHeader can be used)
inline const StudioModelVertexData* StudioModel::GetVertexData( void *ModelData )
{
	const VertexFileHeader* VertexHDR = CacheVertexData( ModelData );
	if ( !VertexHDR )
	{
		return 0;
	}

	VertexData.pVertexData = VertexHDR->GetVertexData();
	VertexData.pTangentData = VertexHDR->GetTangentData();

	if ( !VertexData.pVertexData )
	{
		return 0;
	}

	return &VertexData;
}

// model thin vertex data accessor (defined here so vertexFileHeader_t can be used)
inline const ThinModelVertices* StudioModel::GetThinVertexData( void *ModelData )
{
	const VertexFileHeader* VertexHDR = CacheVertexData( ModelData );
	if ( !VertexHDR )
	{
		return 0;
	}

	return VertexHDR->GetThinVertexData();
}

// - BKH - Get a skinned vertex, the vertex structures in the files can be of differing sizes
inline StudioVertex *StudioModel::GetSkinnedVertex( int Index ) const
{
	// - BKH - Sep 9, 2012
	// Can only get this structure if we're dealing with the right kind of list
	if( VertexListType == VLIST_TYPE_SKINNED )
	{
		assert( Index >= 0 && Index < NumVertices );
		return (StudioVertex *)(((byte *)this) + VertexIndex) + Index;
	}

	return 0;
}

inline StudioVertex2 *StudioModel::GetUnSkinnedVertex( int Index )const
{
	// - BKH - Sep 9, 2012
	// Can only get this structure if we're dealing with the right kind of list
	if( VertexListType == VLIST_TYPE_UNSKINNED )
	{
		assert( Index >= 0 && Index < NumVertices );
		return (StudioVertex2 *)(((byte *)this) + VertexIndex) + Index;
	}

	return 0;
}

inline StudioVertex3 *StudioModel::GetCompressedVertex( int Index ) const
{
	// - BKH - Sep 9, 2012
	// Can only get this structure if we're dealing with the right kind of list
	if( VertexListType == VLIST_TYPE_COMPRESSED )
	{
		assert( Index >= 0 && Index < NumVertices );
		return (StudioVertex3 *)(((byte *)this) + VertexIndex) + Index;
	}

	return 0;
}

inline StudioModel *StudioMesh::GetModel() const 
{ 
	return (StudioModel *)(((byte *)this) + ModelIndex); 
}

inline StudioFlex *StudioMesh::GetFlex( int Index ) const
{ 
	return (StudioFlex *)(((byte *)this) + FlexIndex) + Index;
}

inline const MeshVertexData *StudioMesh::GetVertexData( void *ModelData )
{
	// get this mesh's model's vertex data (allow for StudioModel::GetVertexData returning NULL if the data has been converted to 'thin' vertices)
	GetModel()->GetVertexData( ModelData );
	VertexData.ModelVertexData = &( GetModel()->VertexData );

	if( VertexData.ModelVertexData->pVertexData == 0 )
	{
		return 0;
	}

	return &VertexData;
}

inline const ThinModelVertices *StudioMesh::GetThinVertexData( void *ModelData )
{
	// get this mesh's model's thin vertex data
	return GetModel()->GetThinVertexData( ModelData );
}

inline int MeshVertexData::GetModelVertexIndex( int Index ) const
{
	StudioMesh *Mesh = (StudioMesh *)((byte *)this - offsetof(StudioMesh,VertexData)); 
	return Mesh->VertexOffset + Index;
}

inline int MeshVertexData::GetGlobalVertexIndex( int Index ) const
{
	return ModelVertexData->GetGlobalVertexIndex( GetModelVertexIndex(Index) );
}

inline Vector *MeshVertexData::Position( int Index ) const 
{
	return ModelVertexData->Position( GetModelVertexIndex(Index) ); 
};

inline Vector *MeshVertexData::Normal( int Index ) const 
{
	return ModelVertexData->Normal( GetModelVertexIndex(Index) ); 
};

inline Vector4D *MeshVertexData::TangentS( int Index ) const
{
	return ModelVertexData->TangentS( GetModelVertexIndex(Index) );
}

inline Vector2D *MeshVertexData::Texcoord( int Index ) const 
{
	return ModelVertexData->Texcoord( GetModelVertexIndex(Index) ); 
};

inline StudioBoneWeight *MeshVertexData::BoneWeights( int Index ) const 
{
	return ModelVertexData->BoneWeights( GetModelVertexIndex(Index) ); 
};

inline StudioVertex *MeshVertexData::Vertex( int Index ) const
{
	return ModelVertexData->Vertex( GetModelVertexIndex(Index) );
}

inline StudioMovement* StudioAnimDesc::GetMovementInfo( int Index ) const
{ 
	return (StudioMovement *)(((byte *)this) + MovementIndex) + Index; 
}

inline StudioAnimSections *StudioAnimDesc::GetSection( int Index ) const
{
	return (StudioAnimSections *)(((byte *)this) + SectionIndex) + Index;
}

inline struct StudioAnimBoneChannelSet *StudioAnimDesc::GetBoneChannelSet( int Bone ) const
{
	return (StudioAnimBoneChannelSet *)(((byte *)this) + BoneChannelSetIndex) + Bone;
}

//inline StudioLocalHierarchy *StudioAnimDesc::GetHierarchy( int Index ) const
//{
//	if( LocalHierarchyIndex )
//	{
//		//if( AnimBlock == 0 )
//		{
//			return (StudioLocalHierarchy *)(((byte *)this) + LocalHierarchyIndex) + Index;
//		}
//		//else
//		{
//			//byte *pAnimBlocks = this->
//		}
//	}
//}

//mstudiolocalhierarchy_t *mstudioanimdesc_t::pHierarchy( int i ) const
//{
//	if (localhierarchyindex)
//	{
//		if (animblock == 0)
//		{
//			return  (mstudiolocalhierarchy_t *)(((byte *)this) + localhierarchyindex) + i;
//		}
//		else
//		{
//			byte *pAnimBlocks = pStudiohdr()->GetAnimBlock( animblock );
//			
//			if ( pAnimBlocks )
//			{
//				return (mstudiolocalhierarchy_t *)(pAnimBlocks + localhierarchyindex) + i;
//			}
//		}
//	}
//
//	return NULL;
//}


inline StudioAnimValue *StudioCompressedIKError::GetAnimValue( int Index ) const
{
	if ( offset[Index] > 0) 
	{
		return  (StudioAnimValue *)(((byte *)this) + offset[Index]);
	}

	return 0; 
}

inline StudioBox *StudioHitBoxSet::GetHitBox( int Index ) const
{ 
	return (StudioBox *)(((byte *)this) + HitBoxIndex) + Index; 
}

#endif // STUDIO_H