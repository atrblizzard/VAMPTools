#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API
#endif

#include <fbxsdk.h>

#include <string>

#include "external\studio.h"
#include "basicmath.h"

#include "MDLParser.h"
#include "MDLFile.h"
#include "MDLExporter.h"
#include "Utility.h"
#include "Logging.h"

extern FbxScene *g_pFbxScene;
extern void InitializePruneBones();

using namespace std;

bool QuatToEuler( const FbxQuaternion& InQuat, FbxDouble3 &OutVect )
{
	// Ensure that the quaternion is normalized
	FbxQuaternion Q( InQuat );

	double x( Q[0] ), y( Q[1] ), z( Q[2] ), w( Q[3] );

	double f = x*x + y*y + z*z + w*w;
	// Don't try to normalize 0 quaternions
	if( f != 0.0 )
	{
		Q.Normalize();
	}

	x = Q[0];
	y = Q[1];
	z = Q[2];
	w = Q[3];

	// Convert the quaternion into a nice euler
	// Rotation around X (Roll) = atan2( 2( q0q1 + q2q3 ), 1 - 2( q1q1 + q2q2 ) )
	double fX = atan2( 2.0 * (w*x + y*z), 1.0 - (2.0 * (x*x + y*y)) );
	// Rotation around Y (Pitch) = arcsin( 2( q0q2 - q3q1 ) )
	double fY = asin( 2.0 * (w*y - z*x) );
	// Rotation around Z (Yaw) = atan2( 2( q0q3 + q1q2 ), 1 - 2( q2q2 + q3q3 ) )
	double fZ = atan2( 2.0 * (w*z + x*y), 1.0 - (2.0 * (y*y + z*z)) );

	OutVect[0] = fX;
	OutVect[1] = fY;
	OutVect[2] = fZ;
	return true;
}

bool InvQuatToEuler( const FbxQuaternion& InQuat, FbxDouble3 &OutVect )
{
	double x( InQuat[0] ), y( InQuat[1] ), z( InQuat[2] ), w( InQuat[3] );

	FbxQuaternion Q( x, y, z, -w );

	// Ensure that the quaternion is normalized
	double f = x*x + y*y + z*z + w*w;
	// Don't try to normalize 0 quaternions
	if( f != 0.0 )
	{
		Q.Normalize();
	}

	x = Q[0];
	y = Q[1];
	z = Q[2];
	w = Q[3];

	// Convert the quaternion into a nice euler
	// Rotation around X (Roll) = atan2( 2( q0q1 + q2q3 ), 1 - 2( q1q1 + q2q2 ) )
	double fX = atan2( 2.0 * (w*x + y*z), 1.0 - (2.0 * (x*x + y*y)) );
	// Rotation around Y (Pitch) = arcsin( 2( q0q2 - q3q1 ) )
	double fY = asin( 2.0 * (w*y - z*x) );
	// Rotation around Z (Yaw) = atan2( 2( q0q3 + q1q2 ), 1 - 2( q2q2 + q3q3 ) )
	double fZ = atan2( 2.0 * (w*z + x*y), 1.0 - (2.0 * (y*y + z*z)) );

	OutVect[0] = fX;
	OutVect[1] = fY;
	OutVect[2] = fZ;
	return true;

}

/** Extracts all the skeletal information from this model and stores the result on the model */
bool MDLParser::ExtractSkeleton( MDLFile *MDL )
{
	// Reset the prune bones
	InitializePruneBones();

	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Verbose
		VLOG( "  Found %d bone(s)\n", HDR->NumBones );

		// Make some bones!
		for( int Idx = 0; Idx < HDR->NumBones; Idx++ )
		{
			StudioBone *BoneData = HDR->GetBone( Idx );
			if( BoneData )
			{
				string BoneName = BoneData->GetName();
				Utility::FixupBoneName( BoneName );

				// Check to see if we should prune this bone
				if( Utility::IsPruneBone(BoneName) )
				{
					// Skip this bone - make it an empty reference
					MDL->Bones.push_back( 0 );
					continue;
				}

				// Keep a list of the bones
				MDLBone *NewBone = new MDLBone( Idx );

				// Copy the params
				for( int ParamIdx = 0; ParamIdx < NUMANIMCHANNELS; ParamIdx++ )
				{
					NewBone->AnimChannels[ParamIdx] = BoneData->AnimChannels[ParamIdx];
				}

				// Store the flags
				NewBone->m_nFlags = BoneData->Flags;
				NewBone->Name = BoneName;
				MDL->Bones.push_back( NewBone );
			}
		}
	}

	return true;
}

/** Adds skeleton information to the scene */
bool MDLExporter::ExportSkeleton( MDLFile *MDL )
{
	bool bCreatedSkeleton = false;

	MDLHeader *HDR = MDL ? MDL->GetHeader() : 0;
	if( HDR )
	{
		// Create a root skeleton node
		FbxNode *SkeletonNode = 0;

		for( vector<MDLBone *>::size_type Idx = 0; Idx < MDL->Bones.size(); Idx++ )
		{
			MDLBone *Bone = MDL->Bones[Idx];
			if( Bone )
			{
				// We've at least created the skeleton node in the FBX file
				bCreatedSkeleton = true;

				// Create a root skeleton node if it doesn't exist yet, we create it in this way because we may have a situation where
				// the only bones have been pruned
				if( SkeletonNode == 0 )
				{
					SkeletonNode = FbxNode::Create( g_pFbxScene, "skeleton" );
					g_pFbxScene->GetRootNode()->AddChild( SkeletonNode );
				}

				// Create the skeleton and link it to the node
				FbxSkeleton *Skeleton = FbxSkeleton::Create( g_pFbxScene, Bone->Name.c_str() );

				// Set it up as a root bone, if this is the first bone
				if( Bone == (*MDL->Bones.begin()) )
				{
					Skeleton->SetSkeletonType( FbxSkeleton::eRoot );
				}
				else
				{
					Skeleton->SetSkeletonType( FbxSkeleton::eLimbNode );
				}

				// Create the bone
				FbxNode *Node = FbxNode::Create( g_pFbxScene, Bone->Name.c_str() );
				Node->SetNodeAttribute( Skeleton );

				// Get the bone data
				StudioBone *BoneData = HDR->GetBone( Idx );

				// Initialize the node translation and rotation
				Node->LclTranslation.Set( FbxVector4(BoneData->Position.x,BoneData->Position.y,BoneData->Position.z) );

				// Look up the quaternion rotation of the bone, and convert it to a nice euler angle... and use that for the rotation of the bone
				FbxQuaternion Q( BoneData->Quat.x, BoneData->Quat.y, BoneData->Quat.z, BoneData->Quat.w );

				// Convert Q to a euler
				FbxDouble3 R;
				QuatToEuler( Q, R );

				// Convert to degrees!
				FbxDouble3 NewR( R[0] * RAD2DEG, R[1] * RAD2DEG, R[2] * RAD2DEG );

				// Set the local rotation, rotations are in degrees!
				Node->LclRotation.Set( NewR );

				// No pivots on bones
				FbxVector4 ZeroVect;
				Node->SetRotationPivot( FbxNode::eDestinationPivot, ZeroVect );
				Node->SetScalingPivot( FbxNode::eDestinationPivot, ZeroVect );
				Node->SetRotationOffset( FbxNode::eDestinationPivot, ZeroVect );
				Node->SetScalingOffset( FbxNode::eDestinationPivot, ZeroVect );

				// Store a reference to the node
				Bone->m_pNode = Node;
				Bone->m_pSkeleton = Skeleton;

				// If there's a parent bone... try to link to that, otherwise just link to the "root" skeleton bone
				if( BoneData->ParentBone >= 0 && BoneData->ParentBone < MDL->Bones.size() )
				{
					// Add our node as a child of the parent bone (yay, hierarchy!)
					MDL->Bones[BoneData->ParentBone]->m_pNode->AddChild( Node );
				}
				else
				{
					SkeletonNode->AddChild( Node );
				}
			}
		}
	}

	return bCreatedSkeleton;
}