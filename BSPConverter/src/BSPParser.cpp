#include <vector>

#include "Logging.h"
#include "Utility.h"

#include "studiogamebspfile.h"
#include "zip_uncompressed.h"

#include "BSPParser.h"
#include "BSPFile.h"
#include "BSPExporter.h"

#define ENTITY_BUFFER_SIZE 256

using namespace std;

/** Extracts all of the data from the model file and stores the resulting translated data back into specialized structures on that class */
bool BSPParser::ExtractData( BSPFile *BSP )
{
	if( BSP )
	{
		// Verbose
		VLOG( "Extracting data from file: %s\n", BSP->Filename.c_str() );
		ExtractLumps( BSP );
	}

	// Export all the data!
	BSPExporter::ExportData( BSP );

	return true;
}

/** Extract the lump data */
bool BSPParser::ExtractLumps( BSPFile *BSP )
{
	if( BSP && BSP->Buffer )
	{
		dheader_t *HDR = BSP->GetHeader();
		if( !HDR || HDR->ident != IDBSPHEADER )
		{
			// Verbose
			VLOGWARN( "Warning! Header does not match VBSP!\n" );
			return false;
		}

		// Verbose
		VLOG( "BSP File Version: %d\n\n", HDR->version );

		// Parse all the lumps
		for( int Idx = 0; Idx < HEADER_LUMPS; Idx++ )
		{
			const lump_t &Lump = HDR->lumps[Idx];

			// Skip if there's no data
			if( Lump.filelen <= 0 )
			{
				continue;
			}

			// Verbose
			VLOG( "lump %d:  size:%d byte(s)  type:%s\n", Idx, Lump.filelen, GetType(Idx).c_str(), Lump.fileofs, Lump.version );

			switch( Idx )
			{
			case LUMP_ENTITIES:
				{
					// Verbose
					VLOG( "  Extracting entity lump data...\n" );
					ExtractEntities( BSP, Lump );
				}
				break;
			case LUMP_PLANES:
				{
					// Verbose
					VLOG( "  Extracting plane lump data...\n" );
					ExtractTypedData<dplane_t>( BSP, Idx, BSP->m_lstPlanes );
				}
				break;
			case LUMP_TEXDATA:
				{
					// Verbose
					VLOG( "  Extracting texture data...\n" );
					ExtractTypedData<dtexdata_t>( BSP, Idx, BSP->m_lstTextureData );
				}
				break;
			case LUMP_VERTEXES:
				{
					// Verbose
					VLOG( "  Extracting vertex lump data...\n" );
					ExtractTypedData<dvertex_t>( BSP, Idx, BSP->m_lstVertices );
				}
				break;
			case LUMP_TEXINFO:
				{
					// Verbose
					VLOG( "  Extracting texture info...\n" );
					ExtractTypedData<texinfo_t>( BSP, Idx, BSP->m_lstTexInfo );
				}
				break;
			case LUMP_FACES:
				{
					 // Verbose
					VLOG( "  Extracting face info...\n" );
					ExtractTypedData<dface_bsp17_t>( BSP, Idx, BSP->m_lstFaces );
				}
				break;
			case LUMP_EDGES:
				{
					// Verbose
					VLOG( "  Extracting edge data...\n" );
					ExtractTypedData<dedge_t>( BSP, Idx, BSP->m_lstEdges );
				}
				break;
			case LUMP_SURFEDGES:
				{
					// Verbose
					VLOG( "  Extracting surf edge data...\n" );
					ExtractTypedData<int>( BSP, Idx, BSP->m_lstSurfEdges );
				}
				break;
			case LUMP_MODELS:
				{
					// Verbose
					VLOG( "  Extracting brush model data...\n" );
					ExtractTypedData<dmodel_t>( BSP, Idx, BSP->m_lstModels );
				}
				break;
			case LUMP_BRUSHES:
				{
					// Verbose
					VLOG( "  Extracting brush data...\n" );
					ExtractTypedData<dbrush_t>( BSP, Idx, BSP->m_lstBrushes );
				}
				break;
			case LUMP_BRUSHSIDES:
				{
					// Verbose
					VLOG( "  Extracting brush side data...\n" );
					ExtractTypedData<dbrushside_t>( BSP, Idx, BSP->m_lstBrushSides );
				}
				break;
			case LUMP_DISPINFO:
				{
					// Verbose
					VLOG( "  Extracting displacement info...\n" );
					ExtractTypedData<ddispinfo_t>( BSP, Idx, BSP->m_lstDispInfo );
				}
				break;
			case LUMP_ORIGINALFACES:
				{
					// Verbose
					VLOG( "  Extracting original faces...\n" );
					ExtractTypedData<dface_bsp17_t>( BSP, Idx, BSP->m_lstOrigFaces );
				}
				break;
			case LUMP_GAME_LUMP:
				{
					// Verbose
					VLOG( "  Extracting game specific lump data...\n" );
					ExtractGameLumpData( BSP, Lump );
				}
				break;
			case LUMP_TEXDATA_STRING_DATA:
				{
					// Verbose
					VLOG( "  Extracting string table data...\n" );
					ExtractStringTable( BSP, Lump );
				}
				break;
			case LUMP_TEXDATA_STRING_TABLE:
				{
					// Verbose
					VLOG( "  Extracting string data...\n" );
					ExtractTypedData<int>( BSP, Idx, BSP->m_lstTexStringOffsets );
				}
				break;
			case LUMP_PAKFILE:
				{
					// Verbose
					VLOG( "  Extracting pakfile data...\n" );
					ExtractPakFiles( BSP, Lump );
				}
				break;
			}
		}
	}

	return true;
}

/** Extracts entity data */
bool BSPParser::ExtractEntities( BSPFile *BSP, const lump_t& Lump )
{
	dheader_t *HDR = BSP->GetHeader();
	unsigned char *pLine, *pEnd, *pCurr;

	EntityData *NewEntity = 0;

	pLine = (unsigned char *)HDR + Lump.fileofs;
	pCurr = pLine;
	pEnd = pLine + Lump.filelen;

	char Buffer[ENTITY_BUFFER_SIZE];

	do 
	{
		if( (*pCurr) == '{' )
		{
			// Make a new entity!
			NewEntity = new EntityData;

			// Pass {
			pCurr++;
			// Pass newline
			pCurr++;

			// Advance line
			pLine = pCurr;
		}

		// Advance to newline
		while( (*pCurr) != '\n' )
			++pCurr;

		// If we're at the end of the line... copy it to the buffer
		if( (*pCurr) == '\n' )
		{
			int Len = pCurr - pLine;
			// Copy the line
			memcpy_s( Buffer, ENTITY_BUFFER_SIZE, pLine, Len );
			Buffer[Len] = '\0';

			string Temp = Buffer;
			vector<string> Values;
			
			// Trash the quotes
			Utility::CleanupQuotes( Temp );
			// Parse the values out into some arrays
			Utility::ParseIntoArray( Temp, '"', Values );

			// Add a new key/value pair
			if( NewEntity )
			{
				NewEntity->Values.push_back( EntityValue(Values[0],Values[1]) ); 
			}
		}

		// Next!
		++pCurr;

		// Done current entity
		if( (*pCurr) == '}' )
		{
			// Add the full entity to the list
			BSP->m_lstEntities.push_back( NewEntity );

			// Advance the current position past the end brace and the newline
			++pCurr;
			++pCurr;
		}

		// Test to see if this is the end
		if( (*pCurr) == '\0' )
		{
			break;
		}

		// Advance the line!
		pLine = pCurr;
	} 
	while ( pCurr < pEnd );

	// Extracted a bunch of entities!
	if( BSP->m_lstEntities.size() > 0 )
	{
		// Verbose
		VLOG( "  Extracted %d entities\n", BSP->m_lstEntities.size() );
		return true;
	}

	// Verbose
	VLOG( "  No entity data found\n" );
	return false;
}

/** Template function to extract data! */
template<class T> bool BSPParser::ExtractTypedData( class BSPFile *BSP, int TypeIdx, std::vector<T *>& lstData )
{
	dheader_t *HDR = BSP->GetHeader();
	const lump_t &Lump = HDR->lumps[TypeIdx];

	// Get a pointer to the data type
	T *CurVal = (T *)((char *)HDR + Lump.fileofs);
	T *End = (T *)((char *)HDR + Lump.fileofs + Lump.filelen);

	while( CurVal < End )
	{
		T *NewData = new T( (*CurVal) );
		lstData.push_back( NewData );

		// Get next value
		++CurVal;
	}

	// Did we extract anything?
	if( lstData.size() > 0 )
	{
		// Verbose
		VLOG( "  Extracted %d %s\n", lstData.size(), GetType(TypeIdx).c_str() );
		return true;
	}

	return false;
}

/** Extracts game specific data */
bool BSPParser::ExtractGameLumpData( BSPFile *BSP, const lump_t& Lump )
{
	dheader_t *HDR = BSP->GetHeader();
	dgamelumpheader_t *LumpHeader = (dgamelumpheader_t *)((char *)HDR + Lump.fileofs);

	dgamelump_t *GameLump = (dgamelump_t *)((char *)HDR + Lump.fileofs + sizeof(dgamelumpheader_t));
	dgamelump_t *End = (dgamelump_t *)((char *)HDR + Lump.fileofs + Lump.filelen);

	for( int Idx = 0; Idx < LumpHeader->lumpCount; Idx++, GameLump++ )
	{
		if( GameLump->id == GAMELUMP_STATIC_PROPS )
		{
			// Verbose
			VLOG( "    Found static props game lump\n" );

			// First value is the number of elements in the dictionary
			int *NumDict = (int *)((char *)HDR + GameLump->fileofs);

			// Dictionary lump always comes first in the static prop list
			StaticPropDictLump_t *Dict;
			Dict = (StaticPropDictLump_t *)((char *)NumDict + sizeof(int));

			// Iterate over the entries and add them
			for( int NameIdx = 0; NameIdx < (*NumDict); NameIdx++ )
			{
				// Add the name
				BSP->m_lstStaticPropNameTable.push_back( Dict->m_Name );
				++Dict;
			}
		}
		else if( GameLump->id == GAMELUMP_DETAIL_PROPS )
		{
			// Verbose
			VLOG( "    Found detail props game lump\n" );
		}
		else if( GameLump->id == GAMELUMP_DETAIL_PROP_LIGHTING )
		{
			// Verbose
			VLOG( "    Found detail props for lighting\n" );
		}
		else
		{
			// Verbose
			VLOGWARN( "    Warning! Found unknown type of game lump! (%d)\n", GameLump->id );
		}
	}

	return false;
}

/** Extracts string table data */
bool BSPParser::ExtractStringTable( BSPFile *BSP, const lump_t& Lump )
{
	dheader_t *HDR = BSP->GetHeader();
	char *pEnd, *pCurr;

	pCurr = (char *)HDR + Lump.fileofs;
	pEnd = pCurr + Lump.filelen;

	char Buffer[ENTITY_BUFFER_SIZE];

	while( pCurr < pEnd )
	{
		// get a string from the buffer
		sscanf( pCurr, "%s", &Buffer );

		// Increment the current value by the length of the string
		int Len = strlen(Buffer);		
		if( Len > 0 )
		{
			BSP->m_lstStringTable.push_back( Buffer );
		}
		pCurr += Len + 1;
	}

	// Extracted a bunch of entities!
	if( BSP->m_lstStringTable.size() > 0 )
	{
		// Verbose
		VLOG( "  Extracted %d strings\n", BSP->m_lstStringTable.size() );
		return true;
	}

	// Verbose
	VLOG( "  No string data found\n" );
	return false;
}

/** Extracts pak file data */
bool BSPParser::ExtractPakFiles( BSPFile *BSP, const lump_t& Lump )
{
	dheader_t *HDR = BSP->GetHeader();
	char *PackFile = ((char *)HDR + Lump.fileofs );

	BSP->m_pPackFileLump = new unsigned char[Lump.filelen];
	BSP->m_nPackFileSize = Lump.filelen;

	memcpy_s( BSP->m_pPackFileLump, Lump.filelen, PackFile, Lump.filelen );

	// Record is at the very end of the lump... so grab the last n bytes and jam it in
	//ZIP_EndOfCentralDirRecord *EndRecord = (ZIP_EndOfCentralDirRecord *)((char *)HDR + Lump.fileofs + Lump.filelen - sizeof(ZIP_EndOfCentralDirRecord));

	//if( EndRecord )
	//{
	//	printf( "fileofs: %d\n", Lump.fileofs );
	//	printf( "filelen: %d\n", Lump.filelen );
	//	printf( "fileend: %d\n", Lump.fileofs + Lump.filelen );
	//	printf( "ziphdr: %d\n", sizeof(ZIP_EndOfCentralDirRecord) );
	//	printf( "zipstart: %d\n", Lump.fileofs + Lump.filelen - sizeof(ZIP_EndOfCentralDirRecord) );

	//	printf( "%d\n", EndRecord->centralDirectorySize );
	//	printf( "%d\n", EndRecord->commentLength );
	//	printf( "%d\n", EndRecord->nCentralDirectoryEntries_ThisDisk );
	//	printf( "%d\n", EndRecord->nCentralDirectoryEntries_Total );
	//	printf( "%d\n", EndRecord->numberOfTheDiskWithStartOfCentralDirectory );
	//	printf( "%d\n", EndRecord->numberOfThisDisk );
	//	printf( "%d\n", EndRecord->signature );
	//	printf( "%d\n", EndRecord->startOfCentralDirOffset );

	//	int j = 3;
	//}


	return true;
}

/** Returns the type based on the index */
string BSPParser::GetType( int TypeIdx )
{
	switch( TypeIdx )
	{
	case LUMP_ENTITIES:
		return "entities";
	case LUMP_PLANES:
		return "planes";
	case LUMP_TEXDATA:
		return "texture data";
	case LUMP_VERTEXES:
		return "vertices";
	case LUMP_VISIBILITY:
		return "visibility data";
	case LUMP_NODES:
		return "nodes";
	case LUMP_TEXINFO:
		return "texinfo";
	case LUMP_FACES:
		return "faces";
	case LUMP_LIGHTING:
		return "lighting info";
	case LUMP_OCCLUSION:
		return "occlusion info";
	case LUMP_LEAFS:
		return "leaves";
	case LUMP_FACEIDS:
		return "faceids";
	case LUMP_EDGES:
		return "edges";
	case LUMP_SURFEDGES:
		return "surface edges";
	case LUMP_MODELS:
		return "models";
	case LUMP_WORLDLIGHTS:
		return "worldlights";
	case LUMP_LEAFFACES:
		return "leaf faces";
	case LUMP_LEAFBRUSHES:
		return "leaf brushes";
	case LUMP_BRUSHES:
		return "brushes";
	case LUMP_BRUSHSIDES:
		return "brush sides";
	case LUMP_AREAS:
		return "areas";
	case LUMP_AREAPORTALS:
		return "area portals";
	case LUMP_UNUSED0:
	case LUMP_UNUSED1:
	case LUMP_UNUSED2:
	case LUMP_UNUSED3:
		return "UNUSED";
	case LUMP_DISPINFO:
		return "displacement surface info";
	case LUMP_ORIGINALFACES:
		return "original faces";
	case LUMP_PHYSDISP:
		return "physics displacement info";
	case LUMP_PHYSCOLLIDE:
		return "physics collision";
	case LUMP_VERTNORMALS:
		return "face plane normals";
	case LUMP_VERTNORMALINDICES:
		return "face plane normal indices";
	case LUMP_DISP_LIGHTMAP_ALPHAS:
		return "displacement lightmap alphas";
	case LUMP_DISP_VERTS:
		return "displacement vertices";
	case LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS:
		return "displacement lightmap sample positions";
	case LUMP_GAME_LUMP:
		return "game specific data lumps";
	case LUMP_LEAFWATERDATA:
		return "data for leaf nodes in water";
	case LUMP_PRIMITIVES:
		return "water polygon data";
	case LUMP_PRIMVERTS:
		return "water polygon vertices";
	case LUMP_PRIMINDICES:
		return "water polygon vertex index array";
	case LUMP_PAKFILE:
		return "pakfiles";
	case LUMP_CLIPPORTALVERTS:
		return "clipped portal polygon vertices";
	case LUMP_CUBEMAPS:
		return "env_cubemaps locations";
	case LUMP_TEXDATA_STRING_DATA:
		return "texture name data";
	case LUMP_TEXDATA_STRING_TABLE:
		return "indices into texture names";
	case LUMP_OVERLAYS:
		return "info overlays";
	case LUMP_LEAFMINDISTTOWATER:
		return "distance from leaves to water";
	case LUMP_FACE_MACRO_TEXTURE_INFO:
		return "macro texture infos";
	case LUMP_DISP_TRIS:
		return "displacement surface triangles";
	case LUMP_PHYSCOLLIDESURFACE:
		return "compressed havok surface terrain data";
	}

	return "UNHANDLED TYPE";
}