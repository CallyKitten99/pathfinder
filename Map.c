

#include "Main.h"


extern HWND		g_hWnd;
extern BYTE		g_bAltData;


typedef struct {
	uint32_t uiWidth;
	uint32_t uiHeight;
	uint32_t uiStartIndex;
	uint32_t uiEndIndex;
} ASFILE_MAPDESC;

struct _BACKBUFFER {
	RECT rect;
	UINT uiWidth;
	UINT uiHeight;
	
	HWND hWnd;
	
	HDC hdcBuffer;
	HBITMAP hBitmap;
	HGDIOBJ hOld;
};


static BOOL __stdcall _MapCreateBB(MAP * pMap); // Create new back buffer


extern BOOL __stdcall _CP_DIKSTRA		(MAP * pMap, PATH * pOut); // Computes only
extern BOOL __stdcall _CP_ASTAR			(MAP * pMap, PATH * pOut); // Computes only
extern BOOL __stdcall _CPISD_DIKSTRA	(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with step-delay rendering
extern BOOL __stdcall _CPISD_ASTAR		(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with step-delay rendering
extern BOOL __stdcall _CPITO_DIKSTRA	(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with time-out rendering
extern BOOL __stdcall _CPITO_ASTAR		(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with time-out rendering


MAP * __stdcall MapCreate(UINT uiWidth, UINT uiHeight)
{
	MAP * p;
	UINT i, x, y;
	size_t szMap, szList;
	
	// Check parameters
	if (uiWidth < 2 || uiHeight < 2) return 0;
	
	// Create map
	p = (MAP *)malloc(sizeof(MAP));
	if (!p) return 0;
	
	// Initialise map
	p -> uiMapWidth		= uiWidth;
	p -> uiMapHeight	= uiHeight;
	p -> uiNumNodes		= uiWidth * uiHeight;
	szMap				= (size_t)(sizeof(NODE)		*	p -> uiNumNodes);
	szList				= (size_t)(sizeof(NODE *)	*	p -> uiNumNodes);
	
	// Create nodes
	p -> pNodes = (NODE *)malloc(szMap);
	if (!p -> pNodes) {
		free(p);
		return 0;
	}
	
	// Create open list
	p -> ppOpen = (NODE **)malloc(szList);
	if (!p -> ppOpen) {
		free(p -> pNodes);
		free(p);
		return 0;
	}
	
	// Initialise nodes
	for (i = 0, x = 0, y = 0; ; i++) {
		p -> pNodes[i].x = x;
		p -> pNodes[i].y = y;
		p -> pNodes[i].s = 0;
		p -> pNodes[i].f = 0;
		p -> pNodes[i].g = 0;
		p -> pNodes[i].h = 0;
		p -> pNodes[i].pParent = 0;
		
		if (++x >= uiWidth) {
			if (++y >= uiHeight) break;
			x = 0;
		}
	}
	
	// Initialise queue
	for (i = 0; i < p -> uiNumNodes; i++) p -> ppOpen[i] = 0;
	
	// Set start and end nodes
	x = (uiWidth/2);
	y = (uiHeight/2);
	p -> pStart = & p -> pNodes [ (uiWidth * y) + x ];
	x ++; y ++;
	if (x < (uiWidth-1) && y < (uiHeight-1)) {
		p -> pEnd = & p -> pNodes [ (uiWidth * y) + x ];
	} else p -> pEnd = p -> pStart;
	
	// Set default viewport size
	p -> rctViewport.left		= 0;
	p -> rctViewport.top		= 0;
	p -> rctViewport.right		= 400;
	p -> rctViewport.bottom		= 400;
	p -> rctViewportFill.left	= 0;
	p -> rctViewportFill.top	= 0;
	p -> rctViewportFill.right	= 400;
	p -> rctViewportFill.bottom	= 400;
	p -> uiViewportWidth		= 0;
	p -> uiViewportHeight		= 0;
	p -> uiNodeWidth			= 0;
	p -> uiNodeHeight			= 0;
	p -> pBackBuffer			= 0;
	
	// Initialise statistics
	p -> uiSearchTime	= 0;
	p -> uiDrawTime		= 0;
	p -> uiPathG		= 0;
	p -> uiHighG		= 0;
	
	// Return
	return p;
}
void __stdcall MapDestroy(MAP * pMap)
{
	// Checks
	if (!pMap) return;
	
	// Destroy back buffer
	BackBufferDestroy(pMap -> pBackBuffer);
	
	// Destroy lists
	if (pMap -> ppOpen) free(pMap -> ppOpen);
	
	// Destroy nodes
	if (pMap -> pNodes) free(pMap -> pNodes);
	
	// Destroy map
	free(pMap);
}
MAP * __stdcall MapOpen(LPSTR strFileName)
{
	FILE * file;
	WORD header, version;
	ASFILE_MAPDESC desc;
	UINT uiNumNodes;
	size_t szStream;
	uint8_t * pStream;
	uint8_t * pStreamC;
	size_t szMap;
	NODE * pNodes;
	NODE * pNodesC;
	NODE * pNodesE;
	MAP * pMap;
	UINT b, x, y;
	
	// Open file
	file = fopen(strFileName, "rb");
	if (!file) {
		MessageBoxA (0, "Failed to open file.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Read header
	if (fread((void*)&header, sizeof(WORD), 1, file) != 1 ) {
		fclose(file);
		MessageBoxA (0, "Failed to read from file.", 0, MB_ICONHAND);
		return 0;
	} if (header != 21313) {
		fclose(file);
		MessageBoxA (0, "Map file is invalid.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Read version
	if (fread((void*)&version, sizeof(WORD), 1, file) != 1 ) {
		fclose(file);
		MessageBoxA (0, "Failed to read from file.", 0, MB_ICONHAND);
		return 0;
	} if (!version) {
		fclose(file);
		MessageBoxA (0, "Invalid map file version.", 0, MB_ICONHAND);
		return 0;
	} if (version > 1) {
		fclose(file);
		MessageBoxA (0, "Map file version is not supported.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Read map description
	if (fread((void *)&desc, sizeof(ASFILE_MAPDESC), 1, file) != 1 ) {
		fclose(file);
		MessageBoxA (0, "Failed to read from file.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Checks
	uiNumNodes = desc.uiWidth * desc.uiHeight;
	if (desc.uiStartIndex >= uiNumNodes) desc.uiStartIndex = 0;
	if (desc.uiEndIndex   >= uiNumNodes) desc.uiEndIndex = 0;
	
	// Allocate stream buffer
	szStream = uiNumNodes / 8;
	if (uiNumNodes % 8) szStream ++;
	pStream = (uint8_t *)malloc(szStream);
	if (!pStream) {
		fclose(file);
		MessageBoxA (0, "The system could not allocate enough memory to complete the request.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Read in stream
	if (fread((void *)pStream, 1, szStream, file) != szStream ) {
		free(pStream);
		fclose(file);
		MessageBoxA (0, "Failed to read from file.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Close file
	fclose(file);
	
	// Allocate new buffer for inflated nodes
	szMap = uiNumNodes * sizeof(NODE);
	pNodes = (NODE *)malloc(szMap);
	if (!pNodes) {
		free(pStream);
		MessageBoxA (0, "The system could not allocate enough memory to complete the request.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Allocate map object
	pMap = (MAP *)malloc(sizeof(MAP));
	if (!pMap) {
		free(pNodes);
		free(pStream);
		MessageBoxA (0, "The system could not allocate enough memory to complete the request.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Allocate queue
	pMap -> ppOpen = (NODE **)malloc(uiNumNodes * sizeof(NODE *));
	if (!pMap -> ppOpen) {
		free(pMap);
		free(pNodes);
		free(pStream);
		MessageBoxA (0, "The system could not allocate enough memory to complete the request.", 0, MB_ICONHAND);
		return 0;
	}
	
	// Inflate
	pStreamC	= pStream;
	pNodesC		= pNodes;
	pNodesE		= (NODE *)((int)pNodes + (uiNumNodes * sizeof(NODE)));
	x = y		= 0;
	b			= 7;
	for (; pNodesC < pNodesE; pNodesC = (NODE *)((int)pNodesC + sizeof(NODE))) {
		pNodesC -> x = x;
		pNodesC -> y = y;
		pNodesC -> f = 0;
		pNodesC -> g = 0;
		pNodesC -> h = 0;
		pNodesC -> pParent = 0;
		pNodesC -> s = (*pStreamC & (1<<b)) ? 3 : 0;
		if (!b) {
			b = 7;
			pStreamC ++;
		} else b --;
		if (++x >= desc.uiWidth) {
			x = 0;
			y ++;
		}
	}
	
	// Free stream buffer
	free(pStream);
	
	// Initialise queue
	for (b = 0; b < uiNumNodes; b++) pMap -> ppOpen[b] = 0;
	
	// Fill map details
	pMap -> uiMapWidth		= desc.uiWidth;
	pMap -> uiMapHeight		= desc.uiHeight;
	pMap -> uiNumNodes		= uiNumNodes;
	pMap -> pNodes			= pNodes;
	pMap -> pStart			= &pNodes[desc.uiStartIndex];
	pMap -> pEnd			= &pNodes[desc.uiEndIndex];
	
	// Set default viewport size
	pMap -> rctViewport.left		= 0;
	pMap -> rctViewport.top			= 0;
	pMap -> rctViewport.right		= 400;
	pMap -> rctViewport.bottom		= 400;
	pMap -> rctViewportFill.left	= 0;
	pMap -> rctViewportFill.top		= 0;
	pMap -> rctViewportFill.right	= 400;
	pMap -> rctViewportFill.bottom	= 400;
	pMap -> uiViewportWidth			= 400;
	pMap -> uiViewportHeight		= 400;
	pMap -> uiNodeWidth				= 400 / desc.uiWidth;
	pMap -> uiNodeHeight			= 400 / desc.uiHeight;
	pMap -> pBackBuffer				= 0;
	
	// Initialise statistics
	pMap -> uiSearchTime	= 0;
	pMap -> uiDrawTime		= 0;
	pMap -> uiPathG			= 0;
	pMap -> uiHighG			= 0;
	
	// Render map
	MapRender(pMap);
	
	// Return
	return pMap;
}
BOOL __stdcall MapSave(MAP * pMap, LPSTR strFileName)
{
	UINT uiNumNodes;
	size_t szStream;
	uint8_t * pStream;
	uint8_t * pStreamC;
	NODE * pNodes;
	NODE * pNodesC;
	NODE * pNodesE;
	UINT b;
	FILE * file;
	WORD header = 21313, version = 1;
	ASFILE_MAPDESC desc;
	
	// Checks
	if (!pMap) return FALSE;
	
	// Allocate stream buffer
	uiNumNodes = pMap -> uiNumNodes;
	szStream = uiNumNodes / 8;
	if (uiNumNodes % 8) szStream ++;
	pStream = (uint8_t *)malloc(szStream);
	if (!pStream) {
		MessageBoxA (0, "The system could not allocate enough memory to complete the request.", 0, MB_ICONHAND);
		return FALSE;
	} pStreamC = pStream;
	
	// Zero-out stream buffer
	memset(pStream, 0, szStream);
	
	// Feed stream buffer
	pNodes = pNodesC = pMap -> pNodes;
	pNodesE = (NODE *)((int)pNodes + (uiNumNodes * sizeof(NODE)));
	b = 7;
	for (;; pNodesC = (NODE *)((int)pNodesC + sizeof(NODE))) {
		if (pNodesC >= pNodesE)
			break;
		if (pNodesC -> s == 3)
			*pStreamC |= (1<<b);
		if (!b) {
			b = 7;
			pStreamC ++;
		} else b --;
	}
	
	// Open file
	file = fopen(strFileName, "wb");
	if (!file) {
		free(pStream);
		MessageBoxA (0, "Failed to open file.", 0, MB_ICONHAND);
		return FALSE;
	}
	
	// Write header
	if (fwrite((void*)&header, sizeof(WORD), 1, file) != 1) {
		fclose(file);
		free(pStream);
		MessageBoxA (0, "Failed to write to file.", 0, MB_ICONHAND);
		return FALSE;
	}
	
	// Write version
	if (fwrite((void*)&version, sizeof(WORD), 1, file) != 1) {
		fclose(file);
		free(pStream);
		MessageBoxA (0, "Failed to write to file.", 0, MB_ICONHAND);
		return FALSE;
	}
	
	// Write map description
	desc.uiWidth		= pMap -> uiMapWidth;
	desc.uiHeight		= pMap -> uiMapHeight;
	desc.uiStartIndex	= 0;
	desc.uiEndIndex		= 0;
	for (b = 0; b < uiNumNodes; b++) {
		if (&pNodes[b] == pMap -> pStart) {
			desc.uiStartIndex = b;
			break;
		}
	} for (b = 0; b < uiNumNodes; b++) {
		if (&pNodes[b] == pMap -> pEnd) {
			desc.uiEndIndex = b;
			break;
		}
	}
	if (fwrite((void *)&desc, sizeof(ASFILE_MAPDESC), 1, file) != 1 ) {
		fclose(file);
		free(pStream);
		MessageBoxA (0, "Failed to write to file.", 0, MB_ICONHAND);
		return FALSE;
	}
	
	// Write deflated buffer
	if (fwrite((void *)pStream, 1, szStream, file) != szStream) {
		fclose(file);
		free(pStream);
		MessageBoxA (0, "Failed to write to file.", 0, MB_ICONHAND);
		return FALSE;
	}
	
	// Close file
	fclose(file);
	
	// Return
	return TRUE;
}
NODE * __stdcall MapGetAt(MAP * pMap, UINT X, UINT Y)
{
	if (!pMap || X >= pMap -> uiMapWidth || Y >= pMap -> uiMapHeight) return 0;
	return &pMap -> pNodes[(Y * pMap -> uiMapWidth)+X];
}
NODE * __stdcall MapGetAtCursor(MAP * pMap)
{
	POINT pCur;
	
	if (!pMap) return 0;
	
	GetCursorPos(&pCur);
	ScreenToClient(g_hWnd, &pCur);
	
	pCur.x = (long)((pCur.x * pMap -> uiMapWidth)  / pMap -> uiViewportWidth)  - (long)pMap -> rctViewportFill.left;
	pCur.y = (long)((pCur.y * pMap -> uiMapHeight) / pMap -> uiViewportHeight) - (long)pMap -> rctViewportFill.top;
	
	if (pCur.x >= pMap -> uiMapWidth || pCur.y >= pMap -> uiMapHeight) return 0;
	return &pMap -> pNodes[(pCur.y*pMap -> uiMapWidth)+pCur.x];
}
void __stdcall MapReset(MAP * pMap)
{
	NODE * pNodes;
	UINT i;
	
	if (!pMap) return;
	
	pNodes = pMap -> pNodes;
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		pNodes[i].s = 0;
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		pNodes[i].h = 0;
		pNodes[i].pParent = 0;
	}
}
void __stdcall MapGenerateRandom(MAP * pMap)
{
	NODE * pNodes;
	UINT i;
	
	if (!pMap) return;
	
	pNodes = pMap -> pNodes;
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		if (!(rand()%4)) pNodes[i].s = 3;
		else pNodes[i].s = 0;
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		pNodes[i].h = 0;
		pNodes[i].pParent = 0;
	}
}
MAP * __stdcall MapClone(MAP * pMap)
{
	MAP * p;
	UINT i, x, y;
	size_t szMap, szList;
	NODE * pn;
	
	// Checks
	if (!pMap) return 0;
	
	// Create map
	p = (MAP *)malloc(sizeof(MAP));
	if (!p) return 0;
	
	// Initialise map
	p -> uiMapWidth		= pMap -> uiMapWidth;
	p -> uiMapHeight	= pMap -> uiMapHeight;
	p -> uiNumNodes		= pMap -> uiNumNodes;
	szMap				= (size_t)(sizeof(NODE)		*	p -> uiNumNodes);
	szList				= (size_t)(sizeof(NODE *)	*	p -> uiNumNodes);
	
	// Create nodes
	p -> pNodes = (NODE *)malloc(szMap);
	if (!p -> pNodes) {
		free(p);
		return 0;
	}
	
	// Create open list
	p -> ppOpen = (NODE **)malloc(szList);
	if (!p -> ppOpen) {
		free(p -> pNodes);
		free(p);
		return 0;
	}
	
	// Copy nodes and lists
	memcpy(p -> pNodes, pMap -> pNodes, szMap);
	memcpy(p -> ppOpen, pMap -> ppOpen, szList);
	
	// Assign start and destination nodes
	p -> pStart	= &p -> pNodes[ (pMap -> uiMapWidth * pMap -> pStart -> y) + pMap -> pStart -> x ];
	p -> pEnd	= &p -> pNodes[ (pMap -> uiMapWidth * pMap -> pEnd   -> y) + pMap -> pEnd   -> x ];
	
	// Set default viewport size	
	p -> rctViewport.left		= 0;
	p -> rctViewport.top		= 0;
	p -> rctViewport.right		= 400;
	p -> rctViewport.bottom		= 400;
	p -> rctViewportFill.left	= 0;
	p -> rctViewportFill.top	= 0;
	p -> rctViewportFill.right	= 400;
	p -> rctViewportFill.bottom	= 400;
	p -> uiViewportWidth		= 400;
	p -> uiViewportHeight		= 400;
	p -> uiNodeWidth			= 400 / pMap -> uiMapWidth;
	p -> uiNodeHeight			= 400 / pMap -> uiMapHeight;
	p -> pBackBuffer			= 0;
	
	// Initialise statistics
	p -> uiSearchTime	= 0;
	p -> uiDrawTime		= 0;
	p -> uiPathG		= 0;
	p -> uiHighG		= 0;
	
	// Return
	return p;
}
BOOL __stdcall MapComputePath(MAP * pMap, PATH * pOut, DWORD dwParams)
{	
	// Check parameters
	if (!pMap) {
		MessageBoxA(g_hWnd, "Cannot perform search on an empty map.\nCreate a new map via File->New.", "Path Finder", MB_ICONEXCLAMATION);
		return FALSE;
	} if (!pOut) {
		MessageBoxA(g_hWnd, "Must provide path resulting from search (incorrect call to procedure).", "Path Finder", MB_ICONEXCLAMATION);
		return FALSE;
	} if (!pMap -> pStart) {
		MessageBoxA(g_hWnd, "Use the 'S' key guided by the cursor to select Start node.", "Path Finder", MB_ICONEXCLAMATION);
		return FALSE;
	} if (!pMap -> pEnd) {
		MessageBoxA(g_hWnd, "Use the SPACE key guided by the cursor to select Destination node.", "Path Finder", MB_ICONEXCLAMATION);
		return FALSE;
	} if (pMap -> pStart == pMap -> pEnd) return FALSE;
	
	// Destroy existing path
	PathDestroy(pOut);
	
	// Feed to correct function
	switch (METHOD_GET_ILLLUSTRATE(dwParams))
	{
	case METHOD_ILLUSTATE_NONE:
		switch (METHOD_GET_METHOD(dwParams))
		{
		case METHOD_ASTAR:
			return _CP_ASTAR(pMap, pOut);
		
		case METHOD_DIJKSTRA:
			return _CP_DIKSTRA(pMap, pOut);
		
		default: return FALSE;
		}
		
	case METHOD_ILLUSTATE_DELAY:
		switch (METHOD_GET_METHOD(dwParams))
		{
		case METHOD_ASTAR:
			return _CPISD_ASTAR(pMap, pOut, METHOD_GET_TIME_PARAM(dwParams));
		
		case METHOD_DIJKSTRA:
			return _CPISD_DIKSTRA(pMap, pOut, METHOD_GET_TIME_PARAM(dwParams));
		
		default: return FALSE;
		}
		
	case METHOD_ILLUSTATE_TIMEOUT:
		switch (METHOD_GET_METHOD(dwParams))
		{
		case METHOD_ASTAR:
			return _CPITO_ASTAR(pMap, pOut, METHOD_GET_TIME_PARAM(dwParams));
		
		case METHOD_DIJKSTRA:
			return _CPITO_DIKSTRA(pMap, pOut, METHOD_GET_TIME_PARAM(dwParams));
		
		default: return FALSE;
		}
		
	default: return FALSE;
	}
}
BOOL __stdcall MapComputeRefinedPath(MAP * pMap, PATH * pOut, DWORD dwParams)
{
	
}
BOOL __stdcall MapSetViewport(MAP * pMap, LPRECT pViewport)
{
	// Checks
	if (!pMap) return FALSE;
	
	// Destroy back buffer
	BackBufferDestroy(pMap -> pBackBuffer);
	pMap -> pBackBuffer = 0;
	
	// Set viewport
	if (pViewport) {
		pMap -> rctViewport.left		= pViewport -> left;
		pMap -> rctViewportFill.left	= pViewport -> left;
		pMap -> rctViewport.top			= pViewport -> top;
		pMap -> rctViewportFill.top		= pViewport -> top;
		pMap -> rctViewport.right		= pViewport -> right;
		pMap -> rctViewport.bottom		= pViewport -> bottom;		
		
		pMap -> uiNodeWidth				= (pViewport -> right - pViewport -> left) / pMap -> uiMapWidth;
		pMap -> uiNodeHeight			= (pViewport -> bottom - pViewport -> top) / pMap -> uiMapHeight;
		
		pMap -> uiViewportWidth			= pMap -> uiNodeWidth * pMap -> uiMapWidth;
		pMap -> uiViewportHeight		= pMap -> uiNodeHeight * pMap -> uiMapHeight;
		
		pMap -> rctViewportFill.right	= pViewport -> left + pMap -> uiViewportWidth;
		pMap -> rctViewportFill.bottom	= pViewport -> top + pMap -> uiViewportHeight;
	} else {
		pMap -> rctViewport.left		= 0;
		pMap -> rctViewport.top			= 0;
		pMap -> rctViewport.right		= 400;
		pMap -> rctViewport.bottom		= 400;
		pMap -> rctViewportFill.left	= 0;
		pMap -> rctViewportFill.top		= 0;
		pMap -> rctViewportFill.right	= 400;
		pMap -> rctViewportFill.bottom	= 400;
		pMap -> uiViewportWidth			= 400;
		pMap -> uiViewportHeight		= 400;
		pMap -> uiNodeWidth				= 400 / pMap -> uiMapWidth;
		pMap -> uiNodeHeight			= 400 / pMap -> uiMapHeight;
	}
}
void __stdcall MapRender(MAP * pMap)
{
	UINT64 fr, ts, te;
	NODE * pNodes;
	HDC dc;
	UINT i;
	COLORREF col;
	RECT rct;
	
	// Checks
	if (!pMap) return;
	if (!pMap -> pBackBuffer && !_MapCreateBB(pMap)) return;
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Invalidate region so that it may be redrawn
	InvalidateRect(g_hWnd, &pMap -> rctViewportFill, FALSE);
	
	// Retrievals
	pNodes	= pMap -> pNodes;
	dc		= pMap -> pBackBuffer -> hdcBuffer;	
	
	// Clear background
	SetDCBrushColor(dc, (COLORREF)0xC0C0C0);
	FillRect(dc, &pMap -> rctViewportFill, (HBRUSH)GetStockObject(DC_BRUSH));
	
	if (g_bAltData) { // Render using alternative colours
		// Loop through each node
		for (i = 0; i < pMap -> uiNumNodes; i++) {
			// Select appropriate colour
			switch (pNodes[i].s)
			{
			case 1:
				col = 0xFFFF00;
				break;
			case 2:
				col = 0xFF0000;
				break;
			case 3:
				col = 0x404040;
				break;
			default:
				continue;
			}
			
			rct.left	= pNodes[i].x * pMap -> uiNodeWidth;
			rct.top		= pNodes[i].y * pMap -> uiNodeHeight;
			rct.right	= (pNodes[i].x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
			rct.bottom	= (pNodes[i].y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
			SetDCBrushColor(dc, col);
			FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
		}
	} else { // Render using normal colours
		for (i = 0; i < pMap -> uiNumNodes; i++) {
			if (pNodes[i].s == 3) {
				rct.left	= pNodes[i].x * pMap -> uiNodeWidth;
				rct.top		= pNodes[i].y * pMap -> uiNodeHeight;
				rct.right	= (pNodes[i].x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
				rct.bottom	= (pNodes[i].y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
				SetDCBrushColor(dc, 0x404040);
				FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
			}
		}
	}
	
	// Draw start node
	if (pMap -> pStart) {
		rct.left	= pMap -> pStart -> x * pMap -> uiNodeWidth;
		rct.top		= pMap -> pStart -> y * pMap -> uiNodeHeight;
		rct.right	= (pMap -> pStart -> x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
		rct.bottom	= (pMap -> pStart -> y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
		SetDCBrushColor(dc, 0x00FFFF);
		FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
	}
	
	// Draw destination node
	if (pMap -> pEnd) {
		rct.left	= pMap -> pEnd -> x * pMap -> uiNodeWidth;
		rct.top		= pMap -> pEnd -> y * pMap -> uiNodeHeight;
		rct.right	= (pMap -> pEnd -> x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
		rct.bottom	= (pMap -> pEnd -> y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
		SetDCBrushColor(dc, 0x00FF00);
		FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate result
	pMap -> uiDrawTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
}
void __stdcall MapRenderNode(MAP * pMap, NODE * pNode)
{
	RECT rct;
	COLORREF col;
	HDC dc;
	
	// Checks
	if (!pMap || !pNode) return;
	if (!pMap -> pBackBuffer) {
		if (_MapCreateBB(pMap)) MapRender(pMap);
		return;
	}
	
	// Invalidate region so that it may be redrawn
	InvalidateRect(g_hWnd, &rct, FALSE);
	
	// Select appropriate colour
	if (g_bAltData) {
		switch (pNode -> s)
		{
		case 0:
			col = 0xC0C0C0;
			break;
		case 1:
			col = 0xFFFF00;
			break;
		case 2:
			col = 0xFF0000;
			//double dblLerp = dwHighestGScore > 0 ? double(pNodes[i].g)/double(dwHighestGScore) : 1.0;
			//col = Lerp(0xC00000, 0x0000C0, dblLerp);
			break;
		case 3:
			col = 0x404040;
			break;
		default: return;
		}
	} else {
		if (pNode -> s == 3) col = 0x404040;
		else col = 0xC0C0C0;
	}
	
	// Calculate rectangle
	rct.left	= pNode -> x * pMap -> uiNodeWidth;
	rct.top		= pNode -> y * pMap -> uiNodeHeight;
	rct.right	= (pNode -> x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
	rct.bottom	= (pNode -> y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
	
	// Retrieve DC
	dc = pMap -> pBackBuffer -> hdcBuffer;
	
	// Draw rectangle
	SetDCBrushColor(dc, col);
	FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
}
void __stdcall MapRenderStartNode(MAP * pMap, NODE * pNode)
{
	RECT rct;
	HDC dc;
	
	// Checks
	if (!pMap || !pNode) return;
	if (!pMap -> pBackBuffer) {
		if (_MapCreateBB(pMap)) MapRender(pMap);
		return;
	}
	
	// Invalidate region so that it may be redrawn
	InvalidateRect(g_hWnd, &rct, FALSE);
	
	// Calculate rectangle
	rct.left	= pNode -> x * pMap -> uiNodeWidth;
	rct.top		= pNode -> y * pMap -> uiNodeHeight;
	rct.right	= (pNode -> x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
	rct.bottom	= (pNode -> y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
	
	// Retrieve DC
	dc = pMap -> pBackBuffer -> hdcBuffer;
	
	// Draw rectangle
	SetDCBrushColor(dc, 0x00FFFF);
	FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
}
void __stdcall MapRenderEndNode(MAP * pMap, NODE * pNode)
{
	RECT rct;
	HDC dc;
	
	// Checks
	if (!pMap || !pNode) return;
	if (!pMap -> pBackBuffer) {
		if (_MapCreateBB(pMap)) MapRender(pMap);
		return;
	}
	
	// Invalidate region so that it may be redrawn
	InvalidateRect(g_hWnd, &rct, FALSE);
	
	// Calculate rectangle
	rct.left	= pNode -> x * pMap -> uiNodeWidth;
	rct.top		= pNode -> y * pMap -> uiNodeHeight;
	rct.right	= (pNode -> x * pMap -> uiNodeWidth) + pMap -> uiNodeWidth;
	rct.bottom	= (pNode -> y * pMap -> uiNodeHeight) + pMap -> uiNodeHeight;
	
	// Retrieve DC
	dc = pMap -> pBackBuffer -> hdcBuffer;
	
	// Draw rectangle
	SetDCBrushColor(dc, 0x00FF00);
	FillRect(dc, &rct, (HBRUSH)GetStockObject(DC_BRUSH));
}


void __stdcall PathDestroy(PATH * pPath)
{
	if (pPath -> pNodes) {
		free (pPath -> pNodes);
		pPath -> pNodes = 0;
	} pPath -> uiNumNodes = 0;
}
void __stdcall PathRender(PATH * pPath, MAP * pMap)
{
	HDC dc;
	UINT HW, HH;
	UINT i;
	HGDIOBJ old;
	HPEN pen;
	
	// Checks
	if (!pMap || !pMap -> pBackBuffer || !pPath || !pPath -> pNodes) return;
	
	// Precomputations
	dc = pMap -> pBackBuffer -> hdcBuffer;
	HW = pMap -> uiNodeWidth >> 1;
	HH = pMap -> uiNodeHeight >> 1;
	
	// Create pen
	pen = CreatePen(PS_SOLID, 4, 0x00FF00);
	old = SelectObject(dc, pen);
	
	// Render
	MoveToEx(dc,
		(int)( (pPath -> pNodes[0].x * pMap -> uiNodeWidth) + HW ),
		(int)( (pPath -> pNodes[0].y * pMap -> uiNodeHeight) + HH ),
		0);
	for (i = 0; i < pPath -> uiNumNodes; i++) {
		LineTo(dc,
			(int)( (pPath -> pNodes[i].x * pMap -> uiNodeWidth) + HW),
			(int)( (pPath -> pNodes[i].y * pMap -> uiNodeHeight) + HH) );
	}
	
	// Select original object and destroy pen
	SelectObject(dc, old);
	DeleteObject(pen);
}



BACKBUFFER * __stdcall BackBufferCreate(HWND hWnd, LPRECT pViewport)
{
	BACKBUFFER * p;
	HDC front, back;
	UINT uiWidth, uiHeight;
	HBITMAP bmp;
	HGDIOBJ old;
	
	// Checks
	if (!hWnd || !pViewport) return 0;
	
	// Allocate
	p = (BACKBUFFER *)malloc(sizeof(BACKBUFFER));
	if (!p) return 0;
	
	// Get front device context
	front = GetDC(hWnd);
	if (!front) {
		free(p);
		return 0;
	}
	
	// Create back device context
	back = CreateCompatibleDC(front);
	if (!back) {
		ReleaseDC(hWnd, front);
		free(p);
		return 0;
	}
	
	// Create bitmap
	uiWidth		= pViewport -> right  - pViewport -> left;
	uiHeight	= pViewport -> bottom - pViewport -> top;
	bmp			= CreateCompatibleBitmap(front, uiWidth, uiHeight);
	if (!bmp) {
		ReleaseDC(hWnd, front);
		DeleteDC(back);
		free(p);
		return 0;
	}
	
	// Select bitmap into back device context
	old = SelectObject(back, (HGDIOBJ)bmp);
	
	// Success, store information in object
	p -> rect.left		= pViewport -> left;
	p -> rect.top		= pViewport -> top;
	p -> rect.right		= pViewport -> right;
	p -> rect.bottom	= pViewport -> bottom;
	p -> uiWidth		= uiWidth;
	p -> uiHeight		= uiHeight;
	p -> hWnd			= hWnd;
	p -> hdcBuffer		= back;
	p -> hBitmap		= bmp;
	p -> hOld			= old;
	
	// Release and return
	ReleaseDC(hWnd, front);
	return p;
}
void __stdcall BackBufferDestroy(BACKBUFFER * pBuffer)
{
	// Checks
	if (!pBuffer) return;
	
	// Select old object
	SelectObject(pBuffer -> hdcBuffer, pBuffer -> hOld);
	
	// Destroy bitmap
	DeleteObject(pBuffer -> hBitmap);
	
	// Delete device context
	DeleteDC(pBuffer -> hdcBuffer);
	
	// Destroy buffer object
	free(pBuffer);
}
void __stdcall BackBufferPresent(BACKBUFFER * pBuffer)
{
	HDC front;
	
	// Checks
	if (!pBuffer) return;
	
	// Retrieve front device context
	front = GetDC(pBuffer -> hWnd);
	
	// Perform blit
	BitBlt(front, pBuffer -> rect.left, pBuffer -> rect.top,
		pBuffer -> uiWidth, pBuffer -> uiHeight,
		pBuffer -> hdcBuffer, 0, 0, SRCCOPY);
	
	// Release device context
	ReleaseDC(pBuffer -> hWnd, front);
}



static BOOL __stdcall _MapCreateBB(MAP * pMap)
{
	BACKBUFFER * bb;
	
	bb = BackBufferCreate(g_hWnd, &pMap -> rctViewportFill);
	if (!bb) return FALSE;
	pMap -> pBackBuffer = bb;
	return TRUE;
}

