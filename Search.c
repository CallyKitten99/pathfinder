

#include "Main.h"


extern HWND g_hWnd;


typedef struct {
	NODE ** pStart;
	NODE ** pEnd;
	NODE ** pFront;
	NODE ** pBack;
} NodeQueue;


BOOL __stdcall _CP_DIKSTRA		(MAP * pMap, PATH * pOut); // Computes only
BOOL __stdcall _CP_ASTAR		(MAP * pMap, PATH * pOut); // Computes only
BOOL __stdcall _CPISD_DIKSTRA	(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with step-delay rendering
BOOL __stdcall _CPISD_ASTAR		(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with step-delay rendering
BOOL __stdcall _CPITO_DIKSTRA	(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with time-out rendering
BOOL __stdcall _CPITO_ASTAR		(MAP * pMap, PATH * pOut, UINT uiTime); // Computes and illustrates with time-out rendering


void __stdcall MapGenerateMaze(MAP * const pMap)
{
	UINT64 fr, ts, te;
	NODE * pNodes;		// Pointer to map nodes
	UINT w, h;			// Width and height
	NODE * pCurrent;	// Node being closed
	UINT ew, eh;		// Evaulation width and height
	UINT i;				// Iteration counter
	NodeQueue open;		// Open queue
	NODE * pRight;		// Nodes being carved
	NODE * pTop;		// Nodes being carved
	NODE * pLeft;		// Nodes being carved
	NODE * pBottom;		// Nodes being carved
	BYTE carve, cl, rb;	// Carve direction, carve loop, remove from back
	
	// Checks
	if (!pMap) {
		MessageBoxA(g_hWnd, "Cannot generate maze on an empty map.\nCreate a new map via File->New.", "Path Finder", MB_ICONEXCLAMATION);
		return;
	}
	if (!pMap -> pStart) {
		MessageBoxA(g_hWnd, "Use the 'S' key guided by the cursor to select Start node.", "Path Finder", MB_ICONEXCLAMATION);
		return;
	}
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	
	// Additional computations
	ew = w - 2;
	eh = h - 2;
	
	// Wall-up map
	for (i = 0; i < pMap -> uiNumNodes; i++) pNodes[i].s = 3;
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= pMap -> ppOpen;
	open.pBack	= pMap -> ppOpen;
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Carve start node
	pCurrent -> s = 2;
	*open.pBack = pCurrent;
	open.pBack ++;
	rb = FALSE;
	
	// Begin loop
	for (;;) {		
		/**
		 ** Open nodes for carving
		 **/
		
		// Open right node
		pRight = 0;
		if (pCurrent -> x < ew) { // If not on map edge
			pRight = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 2 ]; // Retrieve corresponding node
			if (pRight -> s != 3) pRight = 0; // Discard if already open
		}
		
		// Open left node
		pLeft = 0;
		if (pCurrent -> x > 1) {
			pLeft = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 2 ];
			if (pLeft -> s != 3) pLeft = 0;
		}
		
		// Open top node
		pTop = 0;
		if (pCurrent -> y > 1) {
			pTop = &pNodes [ ((pCurrent -> y - 2) * w) + pCurrent -> x ];
			if (pTop -> s != 3) pTop = 0;
		}
		
		// Open bottom node
		pBottom = 0;
		if (pCurrent -> y < eh) {
			pBottom = &pNodes [ ((pCurrent -> y + 2) * w) + pCurrent -> x ];
			if (pBottom -> s != 3) pBottom = 0;
		}
		
		// Select random direction
		carve = (BYTE)rand()%4;
		
		// Carve in that direction
		for (cl = 4; cl; cl --) {
			switch (carve)
			{
			case 0:
				if (pRight) {
					*open.pBack = pRight; // Add to back of queue
					open.pBack ++; // Reposition the back of the queue
					if (open.pBack >= open.pEnd) open.pBack = open.pStart; // Rewind if necessary
					pRight -> s = 0; // Carve
					pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ].s = 0; // Retrieve and carve through connecting node
					pCurrent = pRight; // Advance to next node to carve
					rb = TRUE; // Assert that this node is at the queue back, not front
					goto L_CheckCarve;
				} else carve ++;
				break;
			
			case 1:
				if (pTop) {
					*open.pBack = pTop;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pTop -> s = 0;
					pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ].s = 0;
					pCurrent = pTop;
					rb = TRUE;
					goto L_CheckCarve;
				} else carve ++;
				break;
			
			case 2:
				if (pLeft) {
					*open.pBack = pLeft;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pLeft -> s = 0;
					pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ].s = 0;
					pCurrent = pLeft;
					rb = TRUE;
					goto L_CheckCarve;
				} else carve ++;
				break;
			
			case 3:
				if (pBottom) {
					*open.pBack = pBottom;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pBottom -> s = 0;
					pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ].s = 0;
					pCurrent = pBottom;
					rb = TRUE;
					goto L_CheckCarve;
				} else carve ++;
				break;
			
			default:
				carve = 0;
				cl ++;
				break;
			}
		}
		
L_CheckCarve:
		// Check whether carve has eventuated
		if (!cl) {
			// Remove node from list
			if (rb) {
				open.pBack --;
				if (open.pBack < open.pStart) open.pBack = (NODE **)((int)open.pEnd - 1);
			} else {
				open.pFront ++;
				if (open.pFront >= open.pEnd) open.pFront = open.pStart;
			}
			
			// Break out of loop when no more open-nodes remain
			if (open.pFront == open.pBack) {
				// End timer
				QueryPerformanceCounter((PLARGE_INTEGER)&te);
				
				// Calculate statistics
				pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );

				return;
			}
			
			// Retrieve front of open list
			pCurrent = *open.pFront;
			
			// Assert that node is at queue front
			rb = FALSE;
		}
	}
}


BOOL __stdcall _CP_DIKSTRA(MAP * pMap, PATH * pOut)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i;					// Iteration counter
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	UINT64 fr, ts, te;
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Clear map
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		pNodes[i].h = 0;
		// Disconnect parents
		pNodes[i].pParent = 0;
	}
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= pMap -> ppOpen;
	open.pBack	= pMap -> ppOpen;
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Add start node to open list
	*open.pBack = pCurrent;
	open.pBack ++;
	
	// Begin loop
	for (;;) {		
		// Retrieve front of open list
		pCurrent = *open.pFront;
		open.pFront ++;
		if (open.pFront >= open.pEnd) open.pFront = open.pStart;	
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pBack = pNext; // Add to back of queue
				open.pBack ++; // Reposition the back of the queue
				if (open.pBack >= open.pEnd) open.pBack = open.pStart; // Rewind if necessary
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Top is open
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
				}
			}
		}
		
		// Break out of loop when no more open-nodes remain
		if (open.pFront == open.pBack) {			
			// End timer
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			
			// Calculate statistics
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			
			// Report failure
			//MessageBoxA(g_hWnd, "No unobstructed path was found between the Start and Destination nodes.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		}
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}
BOOL __stdcall _CP_ASTAR(MAP * pMap, PATH * pOut)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i, j;				// Iteration counters
	int dstx, dsty, dx, dy;	// Destination X & Y, difference in X & Y
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	UINT64 fr, ts, te;
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	dstx		= pDest -> x;
	dsty		= pDest -> y;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pBack	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Clear map and queue
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		// Calculate heuristic distance
		dx = dstx - pNodes[i].x;
		dy = dsty - pNodes[i].y;
		if (dx < 0) dx = -dx;
		if (dy < 0) dy = -dy;
		pNodes[i].h = (dx + dy) * 10;
		// Disconnect parents
		pNodes[i].pParent = 0;
	} for (i = 0; i < pMap -> uiNumNodes; i++) {
		open.pStart[i] = 0;
	}
	
	// Add start node to open list
	*open.pStart = pCurrent;
	
	// Begin loop
	for (;;) {		
		// Find node
		for (i = (UINT)open.pStart; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pCurrent = *(NODE **)i;
			if (pCurrent) goto L_HasNode;
		}
		
		// No more nodes, end timer and return
		QueryPerformanceCounter((PLARGE_INTEGER)&te);
		pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
		pMap -> uiPathG = 0;
		pMap -> uiHighG = 0;
		return FALSE;	
		
L_HasNode:
		// Remove node from list
		open.pFront = (NODE **)i;
		*open.pFront = 0;
		
		// Find node with lowest f score
		for (; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pNext = *(NODE **)i;
			if (pNext && pNext -> f < pCurrent -> f) {
				*(NODE **)i = pCurrent; // Swap out
				pCurrent = pNext;
			}
		}
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pFront = pNext; // Add to queue
				do { // Reposition 'front' to next empty slot
					open.pFront ++;
					if (open.pFront >= open.pEnd) { // Stay within list
						open.pFront = open.pStart; // Find empty slot from list start
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
					} if (!(*open.pFront)) break; // Break if slot is empty
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
				pNext -> s = 1; // Set to open state
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> f = pNext -> g + pNext -> h; // Assign F score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10; // Assign new G score
					pNext -> f = pNext -> g + pNext -> h; // Assign F score
					pNext -> s = 1; // Set to open state
					pNext -> pParent = pCurrent; // Connect
					dwEvFlags |= 1; // 1 = Right is open
					*open.pFront = pNext; // Add to queue
					do { // Reposition 'front' to next empty slot
						open.pFront ++;
						if (open.pFront >= open.pEnd) { // Stay within list
							open.pFront = open.pStart; // Find empty slot from list start
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
						} if (!(*open.pFront)) break; // Break if slot is empty
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
				}
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Up is open
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 2;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
				}
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 4;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
				}
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 8;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
				}
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
					}
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
					}
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
					}
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
					}
				}
			}
		}
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}
BOOL __stdcall _CPISD_DIKSTRA(MAP * pMap, PATH * pOut, UINT uiTime)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i;					// Iteration counter
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	UINT64 fr, ts, te;		// Performance counters
	MSG msg;				// Junk store for peeking
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Clear map
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		pNodes[i].h = 0;
		// Disconnect parents
		pNodes[i].pParent = 0;
	}
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= pMap -> ppOpen;
	open.pBack	= pMap -> ppOpen;
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Add start node to open list
	*open.pBack = pCurrent;
	open.pBack ++;
	
	// Render the map
	InvalidateRect(g_hWnd, &pMap -> rctViewportFill, FALSE);
	MapRender(pMap);
	
	// Begin loop
	for (;;) {		
		// Cancel if signalled
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			MessageBox(g_hWnd, "The operation was cancelled by the user.", "Information", MB_ICONASTERISK);
			return FALSE;
		}
		
		// Sleep for specified time
		Sleep(uiTime);
		
		// Retrieve front of open list
		pCurrent = *open.pFront;
		open.pFront ++;
		if (open.pFront >= open.pEnd) open.pFront = open.pStart;	
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pBack = pNext; // Add to back of queue
				open.pBack ++; // Reposition the back of the queue
				if (open.pBack >= open.pEnd) open.pBack = open.pStart; // Rewind if necessary
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Top is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Break out of loop when no more open-nodes remain
		if (open.pFront == open.pBack) {			
			// End timer
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			
			// Calculate statistics
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			
			// Report failure
			//MessageBoxA(g_hWnd, "No unobstructed path was found between the Start and Destination nodes.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		}
		
		// Blit and peek
		MapRenderNode(pMap, pCurrent);
		BackBufferPresent(pMap -> pBackBuffer);
		PeekMessage(&msg, 0, 0, 0, 0);
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}
BOOL __stdcall _CPISD_ASTAR(MAP * pMap, PATH * pOut, UINT uiTime)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i, j;				// Iteration counters
	int dstx, dsty, dx, dy;	// Destination X & Y, difference in X & Y
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	UINT64 fr, ts, te;		// Performance counters
	MSG msg;				// Junk store for peeking
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	dstx		= pDest -> x;
	dsty		= pDest -> y;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pBack	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Clear map and queue
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		// Calculate heuristic distance
		dx = dstx - pNodes[i].x;
		dy = dsty - pNodes[i].y;
		if (dx < 0) dx = -dx;
		if (dy < 0) dy = -dy;
		pNodes[i].h = (dx + dy) * 10;
		// Disconnect parents
		pNodes[i].pParent = 0;
	} for (i = 0; i < pMap -> uiNumNodes; i++) {
		open.pStart[i] = 0;
	}
	
	// Add start node to open list
	*open.pStart = pCurrent;
	
	// Render the map
	InvalidateRect(g_hWnd, &pMap -> rctViewportFill, FALSE);
	MapRender(pMap);
	
	// Begin loop
	for (;;) {		
		// Cancel if signalled
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			MessageBox(g_hWnd, "The operation was cancelled by the user.", "Information", MB_ICONASTERISK);
			return FALSE;
		}
		
		// Sleep for time period set
		Sleep(uiTime);
		
		// Find node
		for (i = (UINT)open.pStart; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pCurrent = *(NODE **)i;
			if (pCurrent) goto L_HasNode;
		}
		
		// No more nodes, end timer and return
		QueryPerformanceCounter((PLARGE_INTEGER)&te);
		pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
		pMap -> uiPathG = 0;
		pMap -> uiHighG = 0;
		return FALSE;	
		
L_HasNode:
		// Remove node from list
		open.pFront = (NODE **)i;
		*open.pFront = 0;
		
		// Find node with lowest f score
		for (; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pNext = *(NODE **)i;
			if (pNext && pNext -> f < pCurrent -> f) {
				*(NODE **)i = pCurrent; // Swap out
				pCurrent = pNext;
			}
		}
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pFront = pNext; // Add to queue
				do { // Reposition 'front' to next empty slot
					open.pFront ++;
					if (open.pFront >= open.pEnd) { // Stay within list
						open.pFront = open.pStart; // Find empty slot from list start
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
					} if (!(*open.pFront)) break; // Break if slot is empty
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
				pNext -> s = 1; // Set to open state
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> f = pNext -> g + pNext -> h; // Assign F score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10; // Assign new G score
					pNext -> f = pNext -> g + pNext -> h; // Assign F score
					pNext -> s = 1; // Set to open state
					pNext -> pParent = pCurrent; // Connect
					dwEvFlags |= 1; // 1 = Right is open
					*open.pFront = pNext; // Add to queue
					do { // Reposition 'front' to next empty slot
						open.pFront ++;
						if (open.pFront >= open.pEnd) { // Stay within list
							open.pFront = open.pStart; // Find empty slot from list start
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
						} if (!(*open.pFront)) break; // Break if slot is empty
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Up is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 2;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 4;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 8;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
		}
		
		// Blit and peek
		MapRenderNode(pMap, pCurrent);
		BackBufferPresent(pMap -> pBackBuffer);
		PeekMessage(&msg, 0, 0, 0, 0);
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}
BOOL __stdcall _CPITO_DIKSTRA(MAP * pMap, PATH * pOut, UINT uiTime)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i;					// Iteration counter
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	MSG msg;				// Junk store for peeking
	UINT rtl, rtc;			// Render Tick Last & Current
	UINT64 fr, ts, te;		// Performance counters
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Clear map
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		pNodes[i].h = 0;
		// Disconnect parents
		pNodes[i].pParent = 0;
	}
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= pMap -> ppOpen;
	open.pBack	= pMap -> ppOpen;
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Add start node to open list
	*open.pBack = pCurrent;
	open.pBack ++;
	
	// Start timer
	rtl = GetTickCount();
	
	// Render the map
	InvalidateRect(g_hWnd, &pMap -> rctViewportFill, FALSE);
	MapRender(pMap);
	
	// Begin loop
	for (rtl = GetTickCount();;) {		
		// Cancel if signalled
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			MessageBox(g_hWnd, "The operation was cancelled by the user.", "Information", MB_ICONASTERISK);
			return FALSE;
		}
		
		// Retrieve front of open list
		pCurrent = *open.pFront;
		open.pFront ++;
		if (open.pFront >= open.pEnd) open.pFront = open.pStart;	
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pBack = pNext; // Add to back of queue
				open.pBack ++; // Reposition the back of the queue
				if (open.pBack >= open.pEnd) open.pBack = open.pStart; // Rewind if necessary
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Top is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pBack = pNext;
				open.pBack ++;
				if (open.pBack >= open.pEnd) open.pBack = open.pStart;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
				MapRenderNode(pMap, pNext);
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pBack = pNext;
					open.pBack ++;
					if (open.pBack >= open.pEnd) open.pBack = open.pStart;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Break out of loop when no more open-nodes remain
		if (open.pFront == open.pBack) {			
			// End timer
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			
			// Calculate statistics
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			
			// Report failure
			//MessageBoxA(g_hWnd, "No unobstructed path was found between the Start and Destination nodes.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		}
		
		// Render current node as closed node
		MapRenderNode(pMap, pCurrent);
		
		// Determine whether to blit
		rtc = GetTickCount();
		if (rtc - rtl > uiTime) {
			MSG msg;
			
			// Blit and peek
			rtl = rtc;
			BackBufferPresent(pMap -> pBackBuffer);
			PeekMessage(&msg, 0, 0, 0, 0);
		}
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}
BOOL __stdcall _CPITO_ASTAR(MAP * pMap, PATH * pOut, UINT uiTime)
{
	NODE * pNodes;			// Pointer to map nodes
	UINT w, h;				// Width and height
	NODE * pCurrent;		// Node being closed
	NODE * pDest;			// Destination node
	UINT ew, eh;			// Evaulation width and height
	UINT i, j;				// Iteration counters
	int dstx, dsty, dx, dy;	// Destination X & Y, difference in X & Y
	NodeQueue open;			// Open queue
	NODE * pNext;			// Node being opened
	DWORD dwEvFlags;		// Evaluations flags
	MSG msg;				// Junk store for peeking
	UINT rtl, rtc;			// Render Tick Last & Current
	UINT64 fr, ts, te;		// Performance counters
	
	// Get frequency
	QueryPerformanceFrequency((PLARGE_INTEGER)&fr);
	
	// Start timer
	QueryPerformanceCounter((PLARGE_INTEGER)&ts);
	
	// Retrieve values
	pNodes		= pMap -> pNodes;
	w			= pMap -> uiMapWidth;
	h			= pMap -> uiMapHeight;
	pCurrent	= pMap -> pStart;
	pDest		= pMap -> pEnd;
	dstx		= pDest -> x;
	dsty		= pDest -> y;
	
	// Additional computations
	ew = w - 1;
	eh = h - 1;
	
	// Set queue
	open.pStart	= pMap -> ppOpen;
	open.pFront	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pBack	= (NODE **)((int)pMap -> ppOpen + sizeof(NODE **));
	open.pEnd	= (NODE **)((int)pMap -> ppOpen + (pMap -> uiNumNodes * sizeof(NODE **)));
	
	// Clear map and queue
	for (i = 0; i < pMap -> uiNumNodes; i++) {
		// Reset node state if not impassable
		if (pNodes[i].s != 3) pNodes[i].s = 0;
		// Clear scores
		pNodes[i].f = 0;
		pNodes[i].g = 0;
		// Calculate heuristic distance
		dx = dstx - pNodes[i].x;
		dy = dsty - pNodes[i].y;
		if (dx < 0) dx = -dx;
		if (dy < 0) dy = -dy;
		pNodes[i].h = (dx + dy) * 10;
		// Disconnect parents
		pNodes[i].pParent = 0;
	} for (i = 0; i < pMap -> uiNumNodes; i++) {
		open.pStart[i] = 0;
	}
	
	// Add start node to open list
	*open.pStart = pCurrent;
	
	// Start timer
	rtl = GetTickCount();
	
	// Render the map
	InvalidateRect(g_hWnd, &pMap -> rctViewportFill, FALSE);
	MapRender(pMap);
	
	// Begin loop
	for (;;) {		
		// Cancel if signalled
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			MessageBox(g_hWnd, "The operation was cancelled by the user.", "Information", MB_ICONASTERISK);
			return FALSE;
		}
		
		// Find node
		for (i = (UINT)open.pStart; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pCurrent = *(NODE **)i;
			if (pCurrent) goto L_HasNode;
		}
		
		// No more nodes, end timer and return
		QueryPerformanceCounter((PLARGE_INTEGER)&te);
		pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
		pMap -> uiPathG = 0;
		pMap -> uiHighG = 0;
		return FALSE;	
		
L_HasNode:
		// Remove node from list
		open.pFront = (NODE **)i;
		*open.pFront = 0;
		
		// Find node with lowest f score
		for (; i < (UINT)open.pBack; i += sizeof(NODE **)) {
			pNext = *(NODE **)i;
			if (pNext && pNext -> f < pCurrent -> f) {
				*(NODE **)i = pCurrent; // Swap out
				pCurrent = pNext;
			}
		}
		
		// Set pCurrent to closed state
		pCurrent -> s = 2;
		
		// Reset evaluation flags
		dwEvFlags = 0;
		
		/**
		 ** Open nodes around current node
		 **/
		
		// Open right node
		if (pCurrent -> x < ew) { // If not on map edge
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x + 1 ]; // Retrieve corresponding node
			if (pNext == pDest) { // Break from loop if node is destination
				pNext -> g = pCurrent -> g + 10; // Assign G score
				break;
			} if (pNext -> s == 0) { // Open node only if passable
				*open.pFront = pNext; // Add to queue
				do { // Reposition 'front' to next empty slot
					open.pFront ++;
					if (open.pFront >= open.pEnd) { // Stay within list
						open.pFront = open.pStart; // Find empty slot from list start
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
					} if (!(*open.pFront)) break; // Break if slot is empty
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
				pNext -> s = 1; // Set to open state
				pNext -> g = pCurrent -> g + 10; // Assign G score
				pNext -> f = pNext -> g + pNext -> h; // Assign F score
				pNext -> pParent = pCurrent; // Connect
				dwEvFlags |= 1; // 1 = Right is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10; // Assign new G score
					pNext -> f = pNext -> g + pNext -> h; // Assign F score
					pNext -> s = 1; // Set to open state
					pNext -> pParent = pCurrent; // Connect
					dwEvFlags |= 1; // 1 = Right is open
					*open.pFront = pNext; // Add to queue
					do { // Reposition 'front' to next empty slot
						open.pFront ++;
						if (open.pFront >= open.pEnd) { // Stay within list
							open.pFront = open.pStart; // Find empty slot from list start
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **)); // Set back of list
						} if (!(*open.pFront)) break; // Break if slot is empty
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront; // Reposition the back of the queue if necessary
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open top node
		if (pCurrent -> y) {
			pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 2; // 2 = Up is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 2;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open left node
		if (pCurrent -> x) {
			pNext = &pNodes [ (pCurrent -> y * w) + pCurrent -> x - 1 ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 4; // 4 = Left is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 4;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open bottom node
		if (pCurrent -> y < eh) {
			pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x ];
			if (pNext == pDest) {
				pNext -> g = pCurrent -> g + 10;
				break;
			} if (pNext -> s == 0) {
				*open.pFront = pNext;
				do {
					open.pFront ++;
					if (open.pFront >= open.pEnd) {
						open.pFront = open.pStart;
						open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
					} if (!(*open.pFront)) break;
				} while (1);
				if (open.pFront > open.pBack) open.pBack = open.pFront;
				pNext -> s = 1;
				pNext -> g = pCurrent -> g + 10;
				pNext -> f = pCurrent -> g + 10 + pNext -> h;
				pNext -> pParent = pCurrent;
				dwEvFlags |= 8; // 8 = Bottom is open
				MapRenderNode(pMap, pNext);
			} else if (pNext -> s == 1 || pNext -> s == 2) {
				if (pNext -> g > pCurrent -> g + 10) {
					pNext -> g = pCurrent -> g + 10;
					pNext -> f = pNext -> g + pNext -> h;
					pNext -> s = 1;
					pNext -> pParent = pCurrent;
					dwEvFlags |= 8;
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					MapRenderNode(pMap, pNext);
				}
			}
		}
		
		// Open remaining top nodes
		if (dwEvFlags & 2) {
			// Open top-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
			
			// Open top-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y - 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
		}
		
		// Open remaining bottom nodes
		if (dwEvFlags & 8) {
			// Open bottom-right
			if (dwEvFlags & 1) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x + 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
			
			// Open bottom-left
			if (dwEvFlags & 4) {
				pNext = &pNodes [ ((pCurrent -> y + 1) * w) + pCurrent -> x - 1 ];
				if (pNext == pDest) {
					pNext -> g = pCurrent -> g + 14;
					break;
				} if (pNext -> s == 0) {
					*open.pFront = pNext;
					do {
						open.pFront ++;
						if (open.pFront >= open.pEnd) {
							open.pFront = open.pStart;
							open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
						} if (!(*open.pFront)) break;
					} while (1);
					if (open.pFront > open.pBack) open.pBack = open.pFront;
					pNext -> s = 1;
					pNext -> g = pCurrent -> g + 14;
					pNext -> f = pCurrent -> g + 14 + pNext -> h;
					pNext -> pParent = pCurrent;
					MapRenderNode(pMap, pNext);
				} else if (pNext -> s == 1 || pNext -> s == 2) {
					if (pNext -> g > pCurrent -> g + 14) {
						pNext -> g = pCurrent -> g + 14;
						pNext -> f = pNext -> g + pNext -> h;
						pNext -> s = 1;
						pNext -> pParent = pCurrent;
						*open.pFront = pNext;
						do {
							open.pFront ++;
							if (open.pFront >= open.pEnd) {
								open.pFront = open.pStart;
								open.pBack = (NODE **)((int)open.pEnd - sizeof(NODE **));
							} if (!(*open.pFront)) break;
						} while (1);
						if (open.pFront > open.pBack) open.pBack = open.pFront;
						MapRenderNode(pMap, pNext);
					}
				}
			}
		}
		
		// Render current node as closed node
		MapRenderNode(pMap, pCurrent);
		
		// Determine whether to blit
		rtc = GetTickCount();
		if (rtc - rtl > uiTime) {
			MSG msg;
			
			// Blit and peek
			rtl = rtc;
			BackBufferPresent(pMap -> pBackBuffer);
			PeekMessage(&msg, 0, 0, 0, 0);
		}
	}
	
	// Assign values to destination node
	pMap -> uiPathG = pNext -> g; // Record G Score
	pNext -> pParent = pCurrent; // Connect
	
	// Count nodes
	pCurrent = pNext;
	pNext = pCurrent -> pParent;
	for (i = 1; pNext; i++) pNext = pNext -> pParent;
	
	{ // Create path		
		pOut -> pNodes = (PATHNODE *)malloc(i * sizeof(PATHNODE));
		if (!pOut -> pNodes) {
			QueryPerformanceCounter((PLARGE_INTEGER)&te);
			pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
			MessageBoxA(g_hWnd, "Insufficient memory.", "Dijkstra's Search", MB_ICONHAND);
			return FALSE;
		} pOut -> uiNumNodes = i;
		
		pNext = pCurrent;
		for (i--; pNext; i--) {
			pOut -> pNodes[i].x = pNext -> x;
			pOut -> pNodes[i].y = pNext -> y;
			pNext = pNext -> pParent;
		}
	}
	
	// End timer
	QueryPerformanceCounter((PLARGE_INTEGER)&te);
	
	// Calculate statistics
	pMap -> uiSearchTime = (UINT64)( ((long double)(te - ts)) / ((long double)fr / (long double)1000000.0) );
	
	return TRUE;
}

