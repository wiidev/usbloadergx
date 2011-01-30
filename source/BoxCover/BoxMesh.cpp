#include <gccore.h>
#include "BoxMesh.hpp"

// Quick and dirty hardcoded DVD box mesh

static const guVector g_coverBL = { -0.65f, -0.915f, 0.f };
static const guVector g_coverTR = { 0.65f, 0.915f, 0.f };
static const float g_boxCoverY = 0.05f;
static const float g_boxBorderWidth = 0.022f;
static const guVector g_frontCoverBL = { g_coverBL.x, g_coverBL.y + g_boxCoverY, g_coverBL.z };
static const guVector g_frontCoverTR = { g_coverTR.x, g_coverTR.y + g_boxCoverY, g_coverTR.z };
static const guVector g_backCoverBL = { g_frontCoverBL.x, g_frontCoverBL.y, g_frontCoverBL.z - 0.16f };
static const guVector g_backCoverTR = { g_frontCoverTR.x, g_frontCoverTR.y, g_frontCoverTR.z - 0.16f };
const float g_boxCoverYCenter = (g_frontCoverTR.y - g_frontCoverBL.y) * 0.5f;
const float g_coverYCenter = (g_coverTR.y - g_coverBL.y) * 0.5f;

#define w(x)	((float)x / 64.0f)
#define h(y)	((float)y / 256.0f)


const SMeshVert g_boxMeshQ[] ATTRIBUTE_ALIGN(32) = {	// Quads
	// Bordure du bas devant
	{ { g_frontCoverBL.x, g_frontCoverBL.y,						g_frontCoverBL.z },						CTexCoord(w(0), h(256)) },
	{ { g_frontCoverBL.x, g_frontCoverBL.y - g_boxBorderWidth,	g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y - g_boxBorderWidth,	g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y,						g_frontCoverBL.z },						CTexCoord(w(0), h(224)) },

	// Bordure du haut devant
	{ { g_frontCoverBL.x, g_frontCoverTR.y + g_boxBorderWidth,	g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(1)) },
	{ { g_frontCoverBL.x, g_frontCoverTR.y, 					g_frontCoverBL.z },						CTexCoord(w(0), h(1)) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y,						g_frontCoverBL.z },						CTexCoord(w(0), h(32)) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y + g_boxBorderWidth,	g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(32)) },

	// Bordure du bas derrière
	{ { g_backCoverBL.x, g_backCoverBL.y - g_boxBorderWidth,	g_backCoverBL.z + g_boxBorderWidth },	CTexCoord(w(54), h(256)) },
	{ { g_backCoverBL.x, g_backCoverBL.y,						g_backCoverBL.z },						CTexCoord(w(64), h(224)) },
	{ { g_backCoverTR.x, g_backCoverBL.y,						g_backCoverBL.z },						CTexCoord(w(64), h(224)) },
	{ { g_backCoverTR.x, g_backCoverBL.y - g_boxBorderWidth,	g_backCoverBL.z + g_boxBorderWidth },	CTexCoord(w(54), h(224)) },

	// Bordure du haut derrière
	{ { g_backCoverBL.x, g_backCoverTR.y,						g_backCoverBL.z },						CTexCoord(w(64), h(1)) },
	{ { g_backCoverBL.x, g_backCoverTR.y + g_boxBorderWidth,	g_backCoverBL.z + g_boxBorderWidth },	CTexCoord(w(54), h(1)) },
	{ { g_backCoverTR.x, g_backCoverTR.y + g_boxBorderWidth,	g_backCoverBL.z + g_boxBorderWidth },	CTexCoord(w(54), h(32)) },
	{ { g_backCoverTR.x, g_backCoverTR.y,						g_backCoverBL.z },						CTexCoord(w(64), h(32)) },

	// Bordure de droite devant
	{ { g_frontCoverTR.x,						g_frontCoverBL.y, g_frontCoverBL.z },						CTexCoord(w(0), h(256)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth,	g_frontCoverBL.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(256)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth,	g_frontCoverTR.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR.x,						g_frontCoverTR.y, g_frontCoverBL.z },						CTexCoord(w(0), h(0)) },

	// Bordure de droite derrière
	{ { g_backCoverTR.x + g_boxBorderWidth,	g_backCoverBL.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR.x,					g_backCoverBL.y, g_backCoverBL.z },							CTexCoord(w(64), h(256)) },
	{ { g_backCoverTR.x,					g_backCoverTR.y, g_backCoverBL.z },							CTexCoord(w(64), h(0)) },
	{ { g_backCoverTR.x + g_boxBorderWidth,	g_backCoverTR.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(0)) },

	// Face du haut
	{ { g_frontCoverBL.x, g_frontCoverTR.y + g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(1)) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y + g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(32)) },
	{ { g_backCoverTR.x, g_backCoverTR.y + g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(32)) },
	{ { g_backCoverBL.x, g_backCoverTR.y + g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(1)) },

	// Angle face du haut / face de droite
	{ { g_frontCoverTR.x, g_frontCoverTR.y + g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(32)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverTR.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(0)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverTR.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(0)) },
	{ { g_backCoverTR.x, g_backCoverTR.y + g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(32)) },

	// Face de droite
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverTR.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverBL.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(256)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverBL.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverTR.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(0)) },

	// Angle face de droite / face du bas
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverBL.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(256)) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y - g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_backCoverTR.x, g_backCoverBL.y - g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(224)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverBL.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(256)) },

	// Face du bas
	{ { g_frontCoverTR.x, g_frontCoverBL.y - g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverBL.x, g_frontCoverBL.y - g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_backCoverBL.x, g_backCoverBL.y - g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR.x, g_backCoverBL.y - g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(224)) },

	// Face de gauche en haut
	{ { g_frontCoverBL.x, g_frontCoverTR.y, g_frontCoverBL.z },											CTexCoord(w(1), h(1)) },
	{ { g_frontCoverBL.x, g_frontCoverTR.y + g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(1), h(1)) },
	{ { g_backCoverBL.x, g_backCoverTR.y + g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(1), h(1)) },
	{ { g_backCoverBL.x, g_backCoverTR.y, g_backCoverBL.z},												CTexCoord(w(1), h(1)) },

	// Face de gauche en bas
	{ { g_frontCoverBL.x, g_frontCoverBL.y - g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(1), h(1)) },
	{ { g_frontCoverBL.x, g_frontCoverBL.y, g_frontCoverBL.z },											CTexCoord(w(1), h(1)) },
	{ { g_backCoverBL.x, g_backCoverBL.y, g_backCoverBL.z },											CTexCoord(w(1), h(1)) },
	{ { g_backCoverBL.x, g_backCoverBL.y - g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(1), h(1)) },
};

const SMeshVert g_boxMeshT[] ATTRIBUTE_ALIGN(32) = {	// Triangles
	// Haut devant
	{ { g_frontCoverTR.x, g_frontCoverTR.y, g_frontCoverBL.z },											CTexCoord(w(0), h(16)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverTR.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y + g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(32)) },

	// Haut derrière
	{ { g_backCoverTR.x, g_backCoverTR.y, g_backCoverBL.z },											CTexCoord(w(64), h(16)) },
	{ { g_backCoverTR.x, g_backCoverTR.y + g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(32)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverTR.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(0)) },

	// Bas devant
	{ { g_frontCoverTR.x, g_frontCoverBL.y, g_frontCoverBL.z },											CTexCoord(w(0), h(240)) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y - g_boxBorderWidth, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverTR.x + g_boxBorderWidth, g_frontCoverBL.y, g_frontCoverBL.z - g_boxBorderWidth },	CTexCoord(w(10), h(256)) },

	// Bas derrière
	{ { g_backCoverTR.x, g_backCoverBL.y, g_backCoverBL.z },											CTexCoord(w(64), h(240)) },
	{ { g_backCoverTR.x + g_boxBorderWidth, g_backCoverBL.y, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR.x, g_backCoverBL.y - g_boxBorderWidth, g_backCoverBL.z + g_boxBorderWidth },		CTexCoord(w(54), h(224)) }
};

#undef h
#undef w

const SMeshVert g_flatCoverMesh[] ATTRIBUTE_ALIGN(32) = {
	{ { g_frontCoverBL.x, g_frontCoverBL.y, g_frontCoverBL.z }, CTexCoord(0.f, 1.f) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y, g_frontCoverBL.z }, CTexCoord(1.f, 1.f) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y, g_frontCoverBL.z }, CTexCoord(1.f, 0.f) },
	{ { g_frontCoverBL.x, g_frontCoverTR.y, g_frontCoverBL.z }, CTexCoord(0.f, 0.f) },
};

const SMeshVert g_boxBackCoverMesh[] ATTRIBUTE_ALIGN(32) = {
	{ { g_backCoverTR.x, g_backCoverBL.y, g_backCoverBL.z }, CTexCoord(0.f, 1.f) },
	{ { g_backCoverBL.x, g_backCoverBL.y, g_backCoverBL.z }, CTexCoord(1.3f / 2.76f, 1.f) },
	{ { g_backCoverBL.x, g_backCoverTR.y, g_backCoverBL.z }, CTexCoord(1.3f / 2.76f, 0.f) },
	{ { g_backCoverTR.x, g_backCoverTR.y, g_backCoverBL.z }, CTexCoord(0.f, 0.f) },

	{ { g_frontCoverBL.x, g_frontCoverBL.y, g_frontCoverBL.z }, CTexCoord(1.46f / 2.76f, 1.f) },
	{ { g_frontCoverBL.x, g_frontCoverTR.y, g_frontCoverBL.z }, CTexCoord(1.46f / 2.76f, 0.f) },
	{ { g_backCoverBL.x, g_backCoverTR.y, g_backCoverBL.z }, CTexCoord(1.3f / 2.76f, 0.f) },
	{ { g_backCoverBL.x, g_backCoverBL.y, g_backCoverBL.z }, CTexCoord(1.3f / 2.76f, 1.f) },
};

const SMeshVert g_boxCoverMesh[] ATTRIBUTE_ALIGN(32) = {
	{ { g_frontCoverBL.x, g_frontCoverBL.y, g_frontCoverBL.z }, CTexCoord(1.46f / 2.76f, 1.f) },
	{ { g_frontCoverTR.x, g_frontCoverBL.y, g_frontCoverBL.z }, CTexCoord(1.f, 1.f) },
	{ { g_frontCoverTR.x, g_frontCoverTR.y, g_frontCoverBL.z }, CTexCoord(1.f, 0.f) },
	{ { g_frontCoverBL.x, g_frontCoverTR.y, g_frontCoverBL.z }, CTexCoord(1.46f / 2.76f, 0.f) }
};

const u32 g_flatCoverMeshSize = sizeof g_flatCoverMesh / sizeof g_flatCoverMesh[0];
const u32 g_boxMeshQSize = sizeof g_boxMeshQ / sizeof g_boxMeshQ[0];
const u32 g_boxMeshTSize = sizeof g_boxMeshT / sizeof g_boxMeshT[0];
const u32 g_boxCoverMeshSize = sizeof g_boxCoverMesh / sizeof g_boxCoverMesh[0];
const u32 g_boxBackCoverMeshSize = sizeof g_boxBackCoverMesh / sizeof g_boxBackCoverMesh[0];
