#ifndef MDLFILE_H
#define MDLFILE_H

#include <string>
#include <vector>

#include "external\studio.h"

// Also defined in studio.h
#define NUMANIMCHANNELS 7

#define BONEFLAG_ORIENTATION 0x2

namespace OptimizedModel
{ 
	struct FileHeader_t;
};

class MDLMaterial
{
public:
	MDLMaterial() {}
	MDLMaterial( int ID ) : m_nID( ID ) {}
	~MDLMaterial() {}

	std::vector<std::string> m_lstTextureNames;
	std::vector<std::string> m_lstTexturePaths;
	int m_nID;
};

class MDLBone
{
public:
	MDLBone() : m_nID(-1), m_nFlags(0), m_pSkeleton(0), m_pNode(0) {}
	MDLBone( int ID ) : m_nID(ID), m_nFlags(0), m_pSkeleton(0), m_pNode(0) {}
	~MDLBone() {}

	int m_nID;
	int m_nFlags;
	float AnimChannels[NUMANIMCHANNELS];
	class FbxSkeleton *m_pSkeleton;
	class FbxNode *m_pNode;
	std::string Name;
	std::vector<class FbxCluster *> m_lstClusters;
};

class MDLFile
{
public:
	MDLFile();
	MDLFile( std::string InFilename ) : Filename(InFilename) {}
	~MDLFile();

	// Try to find the model at the specified file path
	static MDLFile *FindModel( const char *InFilename );

	// Returns a pointer to the MDLHeader
	inline struct MDLHeader *GetHeader() { return Buffer ? (MDLHeader *)Buffer : 0; }
	// Returns a pointer to the VTXHeader
	inline struct OptimizedModel::FileHeader_t *GetVTXHeader() { return VTXBuffer ? (OptimizedModel::FileHeader_t *)VTXBuffer : 0; }

	// Model file version
	static const int VTMB_MODEL_FILE_VERSION = 107;

	std::string Filename;
	std::vector<MDLMaterial *> Materials;
	std::vector<MDLBone *> Bones;
	std::vector<class Animation *> Animations;

private:
	unsigned char *Buffer, *VTXBuffer;
};

#endif // MDLFILE_H