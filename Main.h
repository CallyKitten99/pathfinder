

#ifndef _PATHFINDER_INC
#define _PATHFINDER_INC

#include "resource.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define METHOD_DIJKSTRA				0
#define METHOD_ASTAR				1
#define METHOD_ILLUSTATE_NONE		0
#define METHOD_ILLUSTATE_DELAY		1
#define METHOD_ILLUSTATE_TIMEOUT	2

#define METHOD_GET_METHOD(param)			(param & 1)
#define METHOD_GET_ILLLUSTRATE(param)		((param >> 1) & 3)
#define METHOD_GET_TIME_PARAM(param)		(param >> 3)
#define METHOD_SET_METHOD(param, m)			param = (param & (~1)) | m
#define METHOD_SET_TIME_PARAM(param, t)		param = (param & 7) | (t << 3)
#define METHOD_SET_ILLLUSTRATE(param, i)	param = (param & (~6)) | ((i & 3) << 1)

/* Structures */

typedef struct _MAP			MAP;
typedef struct _NODE		NODE;
typedef struct _PATH		PATH;
typedef struct _PATHNODE	PATHNODE;
typedef struct _BACKBUFFER	BACKBUFFER;

struct _NODE {
	UINT x, y, s,
		f, g, h;
	NODE * pParent;
};

struct _PATHNODE {
	UINT x, y;
};

struct _PATH {
	UINT uiNumNodes;
	PATHNODE * pNodes;
};

struct _MAP {
	// Attributions
	UINT uiMapWidth;
	UINT uiMapHeight;
	UINT uiNumNodes;
	
	// Data
	NODE *	pNodes;
	NODE *	pStart;
	NODE *	pEnd;
	NODE **	ppOpen;
	
	// Render parameters
	RECT	rctViewport;
	RECT	rctViewportFill;
	UINT	uiViewportWidth;
	UINT	uiViewportHeight;
	UINT	uiNodeWidth;
	UINT	uiNodeHeight;
	BACKBUFFER * pBackBuffer;
	
	// Statistics
	UINT64	uiSearchTime;	// Microseconds
	UINT64	uiDrawTime;		// Microseconds
	UINT	uiPathG;		// Path G score
	UINT	uiHighG;		// Highest G score
	
};

/* Procedures */

MAP *	__stdcall MapCreate				(UINT uiWidth, UINT uiHeight);
void	__stdcall MapDestroy			(MAP * const pMap);
MAP *	__stdcall MapOpen				(LPSTR strFileName);
BOOL	__stdcall MapSave				(MAP * const pMap, LPSTR strFileName);
void	__stdcall MapDestroy			(MAP * const pMap);
NODE *	__stdcall MapGetAt				(MAP * const pMap, UINT X, UINT Y);
NODE *	__stdcall MapGetAtCursor		(MAP * const pMap);
void	__stdcall MapReset				(MAP * const pMap);
void	__stdcall MapGenerateRandom		(MAP * const pMap);
void	__stdcall MapGenerateMaze		(MAP * const pMap);
MAP *	__stdcall MapClone				(MAP * const pMap);
BOOL	__stdcall MapComputePath		(MAP * const pMap, PATH * pOut, DWORD dwParams);
BOOL	__stdcall MapComputeRefinedPath	(MAP * const pMap, PATH * pOut, DWORD dwParams);
BOOL	__stdcall MapSetViewport		(MAP * const pMap, LPRECT pViewport);
void	__stdcall MapRender				(MAP * const pMap);
void	__stdcall MapRenderNode			(MAP * const pMap, NODE * pNode);
void	__stdcall MapRenderStartNode	(MAP * const pMap, NODE * pNode);
void	__stdcall MapRenderEndNode		(MAP * const pMap, NODE * pNode);

void	__stdcall PathDestroy			(PATH * pPath);
void	__stdcall PathRender			(PATH * pPath, MAP * pMap);

BACKBUFFER *	__stdcall BackBufferCreate	(HWND hWnd, LPRECT pViewport);
void			__stdcall BackBufferDestroy	(BACKBUFFER * pBuffer);
void			__stdcall BackBufferPresent	(BACKBUFFER * pBuffer);

#endif /* _PATHFINDER_INC */