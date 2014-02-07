#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif
#include <fbxsdk.h>

#include <string>
#include <io.h>

#include "external\studio.h"
#include "basicmath.h"

#include "MDLParser.h"
#include "MDLFile.h"
#include "MDLExporter.h"
#include "Animation.h"
#include "Utility.h"
#include "Logging.h"

#define FPS30 0.03333333333333333333333333333333

extern FbxManager *g_pFbxManager;
extern FbxScene *g_pFbxScene;
extern bool g_bForceOverwrite;
extern bool QuatToEuler( const FbxQuaternion& InQuat, FbxDouble3 &OutVect );
extern bool InvQuatToEuler( const FbxQuaternion& InQuat, FbxDouble3 &OutVect );

using namespace std;

Animation::Animation()
{
}

Animation::Animation( const char *InName, int InFrames, float InFPS ) : 
	m_nFrames(InFrames), 
	m_fFPS(InFPS)
{
	// Convert the name to a "friendly" one
	MakeFriendlyName( InName, Name );
}

Animation::~Animation()
{
	for( int nIdx = 0; nIdx < m_lstBoneChannels.size(); nIdx++ )
	{
		delete m_lstBoneChannels[nIdx];
	}
}

void Animation::MakeFriendlyName( const char *Source, string &OutName )
{
	string RetString = Source;

	// Strip everything up to the first '_'
	int nPos = RetString.find_first_of( "_" );
	if( nPos != string::npos )
	{
		// Include the actual _ 
		nPos++;
		RetString = RetString.substr( nPos, RetString.size() - nPos );
	}

	OutName = RetString;
}

// Add the specified node to the node array. Also, add recursively
// all the parent node of the specified node to the array.
static void AddNodeRecursively(FbxArray<FbxNode*>& pNodeArray, FbxNode* pNode)
{
    if (pNode)
    {
        AddNodeRecursively(pNodeArray, pNode->GetParent());

        if (pNodeArray.Find(pNode) == -1)
        {
            // Node not in the list, add it
            pNodeArray.Add(pNode);
        }
    }
}

static void AddMeshNodeRecursiveDown( FbxArray<FbxNode *>& pNodeArray, FbxNode *pNode )
{
	if( pNode )
	{
		// Walk across the entire scene
		for( int nChildIdx = 0; nChildIdx < pNode->GetChildCount(); nChildIdx++ )
		{
			AddMeshNodeRecursiveDown( pNodeArray, pNode->GetChild(nChildIdx) );
		}

		// We're only interested in nodes that contain mesh
		FbxNodeAttribute *pAttr = pNode->GetNodeAttribute();
		if( pAttr )
		{
			switch( pAttr->GetAttributeType() )
			{
			case FbxNodeAttribute::eMesh:
			case FbxNodeAttribute::eNurbs:
			case FbxNodeAttribute::ePatch:
				{
					if( pNodeArray.Find(pNode) == -1 )
					{
						pNodeArray.Add( pNode );
					}
				}
				break;
			}
		}
	}
}

static double GetClampedDouble( double InDbl )
{
	if( InDbl > PI )
	{
		return InDbl - 2*PI;
	}
	if( InDbl < -PI )
	{
		return InDbl + 2*PI;
	}

	return InDbl;
}

static float GetChunkData( AnimationChunk *Chunk, int Index )
{
	if( Chunk->Values.size() == 0 )
	{
		return 0.f;
	}

	// If the index is for data we don't have... we just want to return the last value
	if( Index >= Chunk->Values.size() && Index <= Chunk->Frames )
	{
		return Chunk->Values[Chunk->Values.size() - 1];
	}

	// Otherwise, just return the frame info
	return Chunk->Values[Index];
}

static void ConvertToEuler( MDLBone *BoneRef, const FbxQuaternion& QuatVal, FbxDouble3& Euler )
{
	if( BoneRef )
	{
		// If the orientation flag is set... this bone is oriented on the wrong axis
		if( (BoneRef->m_nFlags & BONEFLAG_ORIENTATION) == 0 )
		{
			// Compute a euler from the quat
			QuatToEuler( QuatVal, Euler );
		}
		else
		{
			// Get the euler, based on a negative rotation
			FbxDouble3 InvRot;
			InvQuatToEuler( QuatVal, InvRot );

			// Convert the Euler to matrix form
			double sinA = sin( InvRot[0] ); double cosA = cos( InvRot[0] );
			double sinB = sin( InvRot[1] ); double cosB = cos( InvRot[1] );
			double sinC = sin( InvRot[2] ); double cosC = cos( InvRot[2] );

			// General rotation matrix Z, Y, X (Roll, Pitch, Yaw)
			double Mat[3][3];
			Mat[0][0] = cosA * cosB;	Mat[0][1] = -(sinB * cosC) + (sinA * cosB * sinC);	Mat[0][2] = (sinB * sinC) + (sinA * cosB * cosC);
			Mat[1][0] = cosA * sinB;	Mat[1][1] = (cosB * cosC) + (sinA * sinB * sinC);	Mat[1][2] = -(cosB * sinC) + (sinA * sinB * cosC);
			Mat[2][0] = -sinA;			Mat[2][1] = cosA * sinC;							Mat[2][2] = cosA * cosC;

			// Swap axes -> X to Y, Y to -Z, Z to -X
			double Ori[3][3];
			Ori[0][0] = 0.0;		Ori[0][1] = 0.0;		Ori[0][2] = -1.0;
			Ori[1][0] = 1.0;		Ori[1][1] = 0.0;		Ori[1][2] = 0.0;
			Ori[2][0] = 0.0;		Ori[2][1] = -1.0;		Ori[2][2] = 0.0;

			// Theoretically this would normally be a matrix mult... but since it's always the same type of operation, we can just do it in-place
			double Det[3][3];

			Det[0][0] = Mat[0][1];	Det[0][1] = -Mat[0][2];		Det[0][2] = -Mat[0][0];
			Det[1][0] = Mat[1][1];	Det[1][1] = -Mat[1][2];		Det[1][2] = -Mat[1][0];
			Det[2][0] = Mat[2][1];	Det[2][1] = -Mat[2][2];		Det[2][2] = -Mat[2][0];

			// Convert back to euler values
			Euler[0] = asin( -Det[2][0] );
			Euler[1] = atan2( Det[1][0], Det[0][0] );
			Euler[2] = atan2( Det[2][1], Det[2][2] );

			Euler[0] = GetClampedDouble( Euler[0] );
			Euler[1] = GetClampedDouble( Euler[1] );
			Euler[2] = GetClampedDouble( Euler[2] );
		}

		// Correct for radians
		Euler[0] *= RAD2DEG;
		Euler[1] *= RAD2DEG;
		Euler[2] *= RAD2DEG;
	}
}

static void CreateTranslationKey( FbxAnimLayer *Layer, FbxNode *SkelNode, unsigned int PropIdx, int FrameIdx, double KeyVal )
{
	if( Layer && SkelNode )
	{
		// Create a curve node if one doesn't exist (don't auto create, since we want to use the lazy functions that auto-create appropriate #s of channels for us)
		FbxAnimCurveNode *pCurveNode = SkelNode->LclTranslation.GetCurveNode( Layer );
		if( pCurveNode == 0 )
		{
			// Create a curve with 3 channels (x, y, z)
			pCurveNode = FbxAnimCurveNode::CreateTypedCurveNode( SkelNode->LclTranslation, g_pFbxScene );

			// And the curve node to the layer
			Layer->AddMember( pCurveNode );

			// Connect the translation propery to the curve node
			SkelNode->LclTranslation.ConnectSrcObject( pCurveNode );
		}

		FbxAnimCurve *pCurve = pCurveNode->GetCurve( PropIdx );
		if( pCurve == 0 )
		{
			// Create curves for each parameter (LclTranslation is a composite var, so we need to index into them)
			switch( PropIdx )
			{
			case 0:
				pCurve = SkelNode->LclTranslation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_X, true );
				break;
			case 1:
				pCurve = SkelNode->LclTranslation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_Y, true );
				break;
			case 2:
				pCurve = SkelNode->LclTranslation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_Z, true );
				break;
			}
			
			pCurveNode->ConnectToChannel( pCurve, PropIdx );
		}

		if( pCurve )
		{
			FbxTime Time;
			int nKeyIdx;
			FbxAnimCurveKey Key;

			// Add the keyframe
			pCurve->KeyModifyBegin();
			Time.SetSecondDouble( FPS30 * (FrameIdx+1) );
			nKeyIdx = pCurve->KeyAdd( Time );
			pCurve->KeySetValue( nKeyIdx, KeyVal );
			pCurve->KeyModifyEnd();
		}
	}
}

static void CreateRotationKeys( FbxAnimLayer *Layer, FbxNode *SkelNode, int FrameIdx, const FbxDouble3& Euler )
{
	if( Layer && SkelNode )
	{
		// Create a curve node if one doesn't exist (don't auto create, since we want to use the lazy functions that auto-create appropriate #s of channels for us)
		FbxAnimCurveNode *pCurveNode = SkelNode->LclRotation.GetCurveNode( Layer );
		if( pCurveNode == 0 )
		{
			// Create a curve with 3 layers (x, y, z) - Euler rotations
			pCurveNode = FbxAnimCurveNode::CreateTypedCurveNode( SkelNode->LclRotation, g_pFbxScene );

			// And the curve node to the layer
			Layer->AddMember( pCurveNode );

			// Connect the rotation property to the curve node
			SkelNode->LclRotation.ConnectSrcObject( pCurveNode );
		}

		for( int nPropIdx = 0; nPropIdx < 3; nPropIdx++ )
		{
			FbxAnimCurve *pCurve = pCurveNode->GetCurve( nPropIdx );
			if( pCurve == 0 )
			{
				// Create curves for each parameter (LclRotation is a composite var, so we need to index into them)
				switch( nPropIdx )
				{
				case 0:
					pCurve = SkelNode->LclRotation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_X, true );
					break;
				case 1:
					pCurve = SkelNode->LclRotation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_Y, true );
					break;
				case 2:
					pCurve = SkelNode->LclRotation.GetCurve( Layer, FBXSDK_CURVENODE_COMPONENT_Z, true );
					break;
				}
			
				pCurveNode->ConnectToChannel( pCurve, nPropIdx );
			}

			if( pCurve )
			{
				FbxTime Time;
				int nKeyIdx;
				FbxAnimCurveKey Key;

				// Add the keyframe
				pCurve->KeyModifyBegin();
				Time.SetSecondDouble( FPS30 * (FrameIdx+1) );
				nKeyIdx = pCurve->KeyAdd( Time );
				pCurve->KeySetValue( nKeyIdx, Euler[nPropIdx] );
				pCurve->KeyModifyEnd();
			}
		}
	}
}

/** Creates a bind pose for the mesh */
void MDLExporter::ComputeBindPose( MDLFile *MDL )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// For the bind pose, we need to walk through all of the skeleton nodes and store their global transform

		FbxPose *NewPose = FbxPose::Create( g_pFbxScene, "BindPose" );
		// By default a new pose is a rest pose, so make sure this is a bind pose
		NewPose->SetIsBindPose( true );

		FbxArray<FbxNode *> ClusterNodes;

		// Walk across the scene and collect all geometry
		FbxNode *pRoot = g_pFbxScene->GetRootNode();

		// Recursively collect all the mesh nodes in the scene, they need to be part of the bind pose
		AddMeshNodeRecursiveDown( ClusterNodes, pRoot );

		// Walk over all the bones in the skeleton and look up each node
		for( int nBoneIdx = 0; nBoneIdx < MDL->Bones.size(); nBoneIdx++ )
		{
			MDLBone *Bone = MDL->Bones[nBoneIdx];
			if( Bone )
			{
				FbxNode *Node = Bone->m_pNode;
		
				// Recursively add nodes!
				AddNodeRecursively( ClusterNodes, Node );
			}
		}

		for( int nIdx = 0; nIdx < ClusterNodes.GetCount(); nIdx++ )
		{
			FbxNode *Node = ClusterNodes.GetAt( nIdx );
			FbxMatrix BindMatrix = Node->EvaluateGlobalTransform();

			NewPose->Add( Node, BindMatrix );
		}

		// Add the pose
		g_pFbxScene->AddPose( NewPose );
	}
}

// Extracts animation data and stores it in the MDLFile
bool MDLParser::ExtractAnimations( class MDLFile *MDL )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Verbose
		VLOG( "  Found %d animation(s)\n", HDR->NumLocalAnims );

		// Extract information from each animation
		for( int AnimIdx = 0; AnimIdx < HDR->NumLocalAnims; AnimIdx++ )
		{
			StudioAnimDesc *AnimData = HDR->GetLocalAnim( AnimIdx );

			// Create a new animation
			Animation *NewAnim = new Animation( AnimData->GetName(), AnimData->NumFrames, AnimData->FPS );
			MDL->Animations.push_back( NewAnim );

			// Pre-allocate the bone channels
			NewAnim->m_lstBoneChannels.reserve( HDR->NumBones );
			for( int nIdx = 0; nIdx < HDR->NumBones; nIdx++ )
			{
				BoneChannel *p = new BoneChannel;
				NewAnim->m_lstBoneChannels.push_back( p );
			}

			// Read in data for each bone...
			for( int BoneIdx = 0; BoneIdx < HDR->NumBones; BoneIdx++ )
			{
				// Get the bone channel
				BoneChannel *pNewBoneChannel = NewAnim->m_lstBoneChannels[BoneIdx];

				MDLBone *Bone = MDL->Bones[BoneIdx];
				if( Bone )
				{
					// Get the animblock data for this bone
					StudioAnimBoneChannelSet *ChannelSet = AnimData->GetBoneChannelSet( BoneIdx );

					// Each anim is split up into up to 7 channels (pX, pY, pZ, rX, rY, rZ, rW)
					for( int ChannelIdx = 0; ChannelIdx < NUMANIMCHANNELS; ChannelIdx++ )
					{
						int Offset = ChannelSet->Data[ChannelIdx];
						if( Offset == 0 )
						{
							// There's no data for this channel, so specify a zero key and skip it
							AnimationChunk *Chunk = pNewBoneChannel->CreateChunk( ChannelIdx, AnimData->NumFrames );
							Chunk->Values.push_back( 0.f );
							continue;
						}

						// Keep reading channel data until we've got enough data for the number of frames in the anim
						int FrameCount = 0;
						while( FrameCount < AnimData->NumFrames )
						{
							StudioAnimBoneChannel *BoneChannelData = (StudioAnimBoneChannel *)(((byte *)ChannelSet + Offset));

							// How many keys can we read?
							byte Keys = BoneChannelData->NumKeys;
							// How many frames is this chunk over?
							byte Frames = BoneChannelData->NumFrames;

							// Move the offset pointer
							Offset += sizeof(byte) * 2;

							// Create a new animation chunk
							AnimationChunk *Chunk = pNewBoneChannel->CreateChunk( ChannelIdx, Frames );
							// Pre-allocate a set number of keys
							Chunk->Values.resize( Keys );

							// Pull out all the keyframe data
							for( int nDataIdx = 0; nDataIdx < Keys; nDataIdx++ )
							{
								short *pValue = (short *)((byte *)ChannelSet + Offset);

								// To explain what's going on here... we get some kind of encoded short value from the stream which we want to 
								// multiply by the float value extracted when we parsed the skeleton... this should in theory give us a delta value for this channel / key pair
								// - BKH - Sep 16, 2012
								Chunk->Values[nDataIdx] = ( (*pValue) * MDL->Bones[BoneIdx]->AnimChannels[ChannelIdx] );

								// Move the offset pointer!
								Offset += sizeof( short );
							}

							// Keep track of how many anim frames we've traversed
							FrameCount += Frames;
						}
					}
				}
			}
		}
	}

	return true;
}

/** Exports one FBX scene per animation */
bool MDLExporter::ExportAnimations( MDLFile *MDL )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		for( vector<Animation *>::iterator It = MDL->Animations.begin(); It != MDL->Animations.end(); ++It )
		{
			// Construct a new animstack for each anim, then export it to a unique file
			Animation *Anim = (*It);

			string AnimOutputName;
			Utility::MakeAnimOutputPath( MDL->Filename, Anim->Name, AnimOutputName );

			VLOG( "Anim:: %s\n", AnimOutputName.c_str() );

			// If the file exists
			if( _access(AnimOutputName.c_str(),0) == 0 )
			{
				// If we're in overwrite mode...
				if( g_bForceOverwrite && Utility::ContainsStr(ExportedAnimations,AnimOutputName) == false )
				{
					// Nuke the file if it exists
					remove( AnimOutputName.c_str() );
				}
				// Otherwise, skip processing this file
				else
				{
					// Verbose
					VLOG( "   Skipping %s, found existing file at: %s\n", Anim->Name.c_str(), AnimOutputName.c_str() );
					continue;
				}
			}

			// Create the FBX data
			FbxAnimStack *AnimStack = CreateFbxAnimation( MDL, Anim );

			// Export an animation
			ExportScene( AnimOutputName.c_str() );

			// Destroy the animstack
			AnimStack->Destroy( true );

			// Add the anim name to the export name list
			ExportedAnimations.push_back( AnimOutputName );
		}
	}	

	return true;
}

/** Craetes an animstack for export to the FBX scene */
FbxAnimStack *MDLExporter::CreateFbxAnimation( MDLFile *MDL, Animation *AnimData )
{
	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		FbxAnimStack *AnimStack = FbxAnimStack::Create( g_pFbxScene, AnimData->Name.c_str() );
		AnimStack->Description = "This is a test description!";

		// Create the default layer
		FbxAnimLayer *BaseLayer = FbxAnimLayer::Create( g_pFbxScene, "BaseLayer" );
		AnimStack->AddMember( BaseLayer );

		// For each bone...
		for( unsigned int BoneIdx = 0; BoneIdx < MDL->Bones.size(); BoneIdx++ )
		{
			// We may have pruned some of the bones... skip those ones
			MDLBone *Bone = MDL->Bones[BoneIdx];
			if( Bone == 0 )
			{
				continue;
			}

			// Look up the bone data
			StudioBone *BoneData = HDR->GetBone( BoneIdx );

			// Get the bind pose for this bone
			FbxDouble3 BoneRef( BoneData->Position.x, BoneData->Position.y, BoneData->Position.z );

			// Get the node that holds this bone, for export - we always assume we're mapping animations onto the
			// root model. As it should be a super-set of all import models.
			// - BKH - Sep 22, 2012
			FbxNode *BoneNode = MDL->Bones[BoneIdx]->m_pNode;

			BoneChannel *BoneChannel = AnimData->m_lstBoneChannels[BoneIdx];

			// Loop over all the frames for this bone
			for( int FrameIdx = 0; FrameIdx < AnimData->m_nFrames; FrameIdx++ )
			{
				double QuatKeys[NUMANIMCHANNELS];

				// For frame n, check to see if there is a key for each element in this bone channel...
				for( int ChannelIdx = 0; ChannelIdx < NUMANIMCHANNELS; ChannelIdx++ )
				{
					int ChunkIdx(0), FrameOffset(0);
					QuatKeys[ChannelIdx] = 0.f;

					// Since we may not have bone data for every index (some may have been pruned), we also may not have channel data for every bone
					const vector<AnimationChunk *> &ChannelData = BoneChannel->Channels[ChannelIdx];
					if( ChannelData.size() > 0 )
					{
						AnimationChunk *Chunk = ChannelData[ChunkIdx];

						// Find the bucket that this frame index falls into
						while( FrameIdx >= (FrameOffset + Chunk->Frames) )
						{
							// Update the frame offset
							FrameOffset += Chunk->Frames;

							// Increment the chunk index, we want the next chunk
							ChunkIdx++;
							Chunk = ChannelData[ChunkIdx];
						}

						// In theory we're in the right bucket... only create a keyframe if the value actually exists for this element at this frame
						// if it doesn't exist... that means we'll just be using the keyframe value from earlier (less frames == good!)
						if( ChannelIdx < 3 )
						{
							int nIdx = FrameIdx - FrameOffset;
							if( Chunk->Values.size() > 0 && nIdx >= 0 && nIdx < Chunk->Values.size() )
							{
								double KeyVal = Chunk->Values[nIdx] + BoneRef[ChannelIdx];
							
								// Create a translation key
								CreateTranslationKey( BaseLayer, BoneNode, ChannelIdx, FrameIdx, KeyVal );
							}
						}
						else
						{
							// Kind of gross... but basically we want to store all of the quaternion data
							QuatKeys[ChannelIdx] = GetChunkData( Chunk, FrameIdx - FrameOffset );
						}
					}
				}

				// After we're done processing all of the channels for this bone for this frame... get the final rotation transform and convert it
				// to a euler
				FbxQuaternion qQuatVal( QuatKeys[3], QuatKeys[4], QuatKeys[5], QuatKeys[6] );

				// If there is no rotation about the axis
				if( qQuatVal[3] == 0.0 )
				{
					// use the default bone rotation instead, since it's a 0 rotation
					qQuatVal[0] = BoneData->Quat.x;
					qQuatVal[1] = BoneData->Quat.y;
					qQuatVal[2] = BoneData->Quat.z;
					qQuatVal[3] = BoneData->Quat.w;
				}

				// Convert the quaternion to a euler value
				FbxDouble3 vEuler;
				ConvertToEuler( Bone, qQuatVal, vEuler );

				// Create a rotation key (there's most likely always going to be some change in the rotations... we could probably do some tests to reduce
				// the number of keys generated, but it might just be better to try to bake it down after we're done)
				CreateRotationKeys( BaseLayer, BoneNode, FrameIdx, vEuler );
			}
		}

		return AnimStack;
	}

	return 0;
}