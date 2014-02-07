#pragma once

#include <string>
#include <vector>

#include "studiobspfile.h"

struct EntityValue
{
	EntityValue() {};
	EntityValue( std::string InKey, std::string InValue ) : Key(InKey), Value(InValue) {};

	std::string Key;
	std::string Value;
};

struct EntityData
{
	std::vector<EntityValue> Values;
};

class BSPFile
{
public:
	BSPFile();
	BSPFile( std::string InFilename );
	~BSPFile();

	// Try to find the model at the specified file path
	static BSPFile *FindBSP( const char *InFilename );

	// Returns a pointer to the BSP Header
	inline dheader_t *GetHeader() { return Buffer ? (dheader_t *)Buffer : 0; }

	std::string Filename;
	unsigned char *Buffer;

	// List of entities
	std::vector<EntityData *> m_lstEntities;
	// List of entity types we don't care about
	std::vector<std::string> m_lstCullEntities;
	// List of planes
	std::vector<dplane_t *> m_lstPlanes;
	// List of texture data
	std::vector<dtexdata_t *> m_lstTextureData;
	// List of vertices
	std::vector<dvertex_t *> m_lstVertices;
	// List of texture info
	std::vector<texinfo_t *> m_lstTexInfo;
	// List of faces
	std::vector<dface_bsp17_t *> m_lstFaces;
	// List of edges
	std::vector<dedge_t *> m_lstEdges;
	// List of surface edges
	std::vector<int *> m_lstSurfEdges;
	// List of brush models
	std::vector<dmodel_t *> m_lstModels;
	// List of brushes
	std::vector<dbrush_t *> m_lstBrushes;
	// List of brush sides
	std::vector<dbrushside_t *> m_lstBrushSides;
	// List of displacement info
	std::vector<ddispinfo_t *> m_lstDispInfo;
	// List of original faces
	std::vector<dface_bsp17_t *> m_lstOrigFaces;
	// List of texture name offsets
	std::vector<int *> m_lstTexStringOffsets;
	// List of strings
	std::vector<std::string> m_lstStringTable;

	// Pakfile data
	unsigned char *m_pPackFileLump;
	unsigned int m_nPackFileSize;

	// Static prop data
	std::vector<std::string> m_lstStaticPropNameTable;
};