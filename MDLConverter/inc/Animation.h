#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include "basicmath.h"

// Defines a channel of animation data over a set number of frames
struct AnimationChunk
{
	AnimationChunk(){}
	AnimationChunk( int InFrames ) : Frames( InFrames ) {}
	~AnimationChunk(){}

	int Frames;
	std::vector<float> Values;
};

// Container class for lists
struct BoneChannel
{
	std::vector<AnimationChunk *> Channels[NUMANIMCHANNELS];

	inline AnimationChunk *CreateChunk( int nChannel, int nFrames )
	{
		AnimationChunk *p = new AnimationChunk( nFrames );
		Channels[nChannel].push_back( p );
		return p;
	}
};

struct BonePose
{
	// Location of the bone
	Vector Pos;
	// Quaternion rotation
	Quaternion QRot;
	// Euler rotation
	Vector RRot;
};

class Animation
{
public:
	
	Animation();
	Animation( const char *InName, int InFrames, float InFPS );
	~Animation();

	// Convert the specified name into a "friendly" one!
	void MakeFriendlyName( const char *Source, std::string &OutName );

	int m_nFrames;
	float m_fFPS;
	std::string Name;
	
	// Bone Key Data, should be one per bone
	std::vector<BoneChannel *> m_lstBoneChannels;
};


#endif // ANIMATION_H