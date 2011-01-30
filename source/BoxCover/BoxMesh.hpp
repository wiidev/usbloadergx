#ifndef BOXMESH_HPP_
#define BOXMESH_HPP_

//Box mesh from hibern

// Quick and dirty hardcoded DVD box mesh
// Should be replaced by a true mesh loader
// Lacks normals

class CTexCoord
{
public:
	float x;
	float y;
public:
	CTexCoord(void) { x = 0.f; y = 0.f; }
	CTexCoord(float px, float py) { x = px; y = py; }
};

struct SMeshVert
{
	guVector pos;
	CTexCoord texCoord;
};

// Flat cover
extern const SMeshVert g_flatCoverMesh[];
extern const u32 g_flatCoverMeshSize;

// Box
extern const SMeshVert g_boxMeshQ[];	// Quads
extern const u32 g_boxMeshQSize;
extern const SMeshVert g_boxMeshT[];	// Triangles
extern const u32 g_boxMeshTSize;
// Box cover
extern const SMeshVert g_boxBackCoverMesh[];
extern const u32 g_boxBackCoverMeshSize;
extern const SMeshVert g_boxCoverMesh[];
extern const u32 g_boxCoverMeshSize;
//
extern const float g_boxCoverYCenter;
extern const float g_coverYCenter;


#endif
