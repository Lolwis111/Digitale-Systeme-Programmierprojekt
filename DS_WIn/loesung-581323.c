/* Matrikelnummer: 581323 ; Levin Palm <palmlevi@informatik.hu-berlin.de> */
// TODO: not a problem but the heap never shrinks, might waste memory
// TODO: heap.update() is linear, speedup by using hashtable
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>

/* How much memory should be allocated in the beginning */
#define MEMORY_START_SIZE 32 /* for data */
#define SUB_NODE_START_SIZE 2 /* for neighbours in a node */
#define INFINITY32 UINT32_MAX /* infinity for dijkstra, because all numbers are smaller */
#define INFINITY64 UINT64_MAX /* than 4*10^9 we can use the values above that for whatever */

/* RAW data out of the file */
typedef struct edge_t
{
	uint32_t startID; /* edge from */
	uint32_t endID; /* to */
	uint64_t distance; /* with this weight/distance */
} edge_t;

typedef struct edges_t
{
	uint32_t count; /* size of actually used memory */
	uint32_t limit; /* size of allocated memory */
	edge_t *data; /* pointer to data itself */
} edges_t;

bool insertEdge(edges_t*, edge_t*); /* tries to insert an edge */

uint32_t findFirstEdge(edges_t*, uint32_t); /* finds the first index of the nodes with the given id */

typedef struct savehouses_t
{
	uint32_t count; /* how many savehouses this has */
	uint32_t limit; /* how much space we have */
	uint32_t *data; /* the savehouses */
} savehouses_t;

bool insertSaveHouse(savehouses_t*, uint32_t); /* tries to insert a savehouse id */
bool checkSaveHouse(savehouses_t*, uint32_t); /* checks if the given id is a savehouse */

typedef struct neighbour_t /* represents one neighbour of a node */
{
	uint32_t index;    /* the index in the node list of the neighbour node */
	uint64_t distance; /* the distance between the two nodes */
} neighbour_t;

typedef struct node_t /* structured data to work with */
{
	uint32_t id;				/* id of the node */
	uint64_t distance;			/* distance from the headNode (algorithm) */
	bool isSaveHouse;			/* is this node a savehouse? */
	bool visited;				/* was this node seen before? */
	neighbour_t *neighbours;	/* list of node's neighbours */
	uint32_t neighboursCount;   /* how many neighbours there are */
	uint32_t neighboursLimit;   /* how much memory we have */
} node_t;

typedef struct graph_t /* this represents the graph in a graph-like structure */
{
	uint32_t count; /* how many nodes there are */
	uint32_t limit; /* for how much nodes we have space */
	node_t *vertices; /* the nodes itself */
} graph_t;

bool buildGraph(savehouses_t*, edges_t*, graph_t*);

bool dijkstra(graph_t*, uint32_t); /* perform dijkstra on graph starting with index */

uint32_t findNode(graph_t*, uint32_t); /* find node with id in graph and give index */
bool insertChildNode(node_t*, uint32_t, uint64_t); /* inserts a childnode into the parent node */
bool insertNode(graph_t*, node_t); /* inserts a node into the node collection */

#define LEFT(INDEX) ((INDEX+INDEX) + 1) /* calculate left child */
#define RIGHT(INDEX) ((INDEX+INDEX) + 2) /* calculate right child */
#define PARENT(INDEX) ((INDEX - 1) / 2) /* calculate parent index */

typedef struct heap_t /* this represents the heap */
{
	uint32_t count; /* how many elements are in the heap */
	uint32_t limit; /* how much capacity the heap currently has */
	uint32_t *data; /* the data */
} heap_t;

bool insertNodeToHeap(graph_t*, heap_t*, uint32_t); /* this inserts the given value into the heap */
uint32_t removeMinNodeFromHeap(graph_t*, heap_t*); /* this gets the "first" (the smallest) element from the heap */
void siftDownHeap(graph_t*, heap_t*, uint32_t); /* this is more for internal use, but basically */
void siftUpHeap(graph_t*, heap_t*, uint32_t);  /* makes sure the heap is a heap after changing values */

inline int compare(uint32_t, uint32_t); /* used for qsort() to determine in which way to sort */
int compare_edges(const void*, const void*); /* these three are just wrappers for compare() */
int compare_saveHouses(const void*, const void*);
int compare_nodes(const void*, const void*);

void freeGraph(graph_t*); /* helper methods for freeing complex structures */
void freeEdges(edges_t*);
void freeSaveHouses(savehouses_t*);

bool readData(savehouses_t*, edges_t*); /* reads in the data from stdin */

const char *mallocZeroException = "malloc ran out of memory while allocating!\n"; /* exception message for when malloc fails */
const char *invalidFormatException = "the given is data is not in a valid format!\n"; /* exception message for when the input format is invalid */
const char *inputEmptyException = "the input is empty!\n"; /* exception message for when the input is empty */
const char *numbersOutOfRange = "the numbers in the input are out of range!\n"; /* for negative numbers or ints bigger 4000000000 */

uint32_t globalStartID; /* this is the first triple in the file */
uint32_t globalEndID;  /* startID and endID are is the route to find */
uint64_t globalDistance; /* distance is the maximum distance per day */

/*====UTIL ROUTINES============================================================*/
int main(void)
{
	edges_t edges; /* this will hold the raw edges */
	savehouses_t saveHouses; /* this will hold all the ids which are savehouses */

	edges.data = (edge_t*)malloc(sizeof(edge_t) * MEMORY_START_SIZE); /* allocate the beginning memory for edges */
	edges.count = 0;
	edges.limit = MEMORY_START_SIZE;

	saveHouses.data = (uint32_t*)malloc(sizeof(uint32_t) * MEMORY_START_SIZE);
	saveHouses.count = 0;
	saveHouses.limit = MEMORY_START_SIZE;

	/* check if any of the allocations failed (usually means that host is out of memory) */
	if (NULL == edges.data || NULL == saveHouses.data)
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);

		return 1;
	}

	if (false == readData(&saveHouses, &edges)) /* try to read in the data, if anything is not correct false gets returned */
	{
		fputs(invalidFormatException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);

		return 1;
	}

	if (edges.count > 0)
	{
		edge_t *temp = (edge_t*)malloc(sizeof(edge_t) * edges.count); /* adjust the array size to actual size */

		if (NULL == temp)
		{
			fputs(mallocZeroException, stderr);

			freeSaveHouses(&saveHouses);
			freeEdges(&edges);

			return 1;
		}
        
        /* (adjust the space to the size actually used, can free big chunks of unused memory) */
		memcpy(temp, edges.data, sizeof(edge_t) * edges.count);
        free(edges.data);
		edges.data = temp;
		edges.limit = edges.count;

		/* sort the edges by the startID (quicksort from stdlib.h) this is some investment which will give us binary search */
		qsort(edges.data, edges.count, sizeof(edge_t), compare_edges);
	}
	else
	{
		free(edges.data);
		edges.data = NULL;
		edges.limit = 0;
	}

	if (saveHouses.count > 0)
	{
		/* adjust size of savehouses to actual size */
		uint32_t *temp2 = (uint32_t*)malloc(sizeof(edge_t) * saveHouses.count);

		if (NULL == temp2)
		{
			fputs(mallocZeroException, stderr);

			freeSaveHouses(&saveHouses);
			freeEdges(&edges);

			return 1;
		}

		memcpy(temp2, saveHouses.data, sizeof(uint32_t) * saveHouses.count);
		free(saveHouses.data);
		saveHouses.data = temp2;
		saveHouses.limit = saveHouses.count;

		qsort(saveHouses.data, saveHouses.count, sizeof(uint32_t), compare_saveHouses);
	}
	else
	{
		/* if we do not have any save houses the answer is obviously empty */
		free(saveHouses.data);
		saveHouses.data = NULL;
		saveHouses.count = 0;

		freeEdges(&edges); 
        
		return 0;
	}


	if (edges.count == 0) /* if there are no edges we have to check if start==end==savehouse */
	{
		if (globalStartID == globalEndID && checkSaveHouse(&saveHouses, globalStartID))
        {
			fprintf(stdout, "%"PRIu32, globalStartID);
        }

		freeSaveHouses(&saveHouses);

		return 0;
	}

	graph_t graph1; /* create the graph with direction start -> end */
	graph1.count = 0;
	graph1.limit = MEMORY_START_SIZE * 2;
	graph1.vertices = (node_t*)calloc(graph1.limit, sizeof(node_t));

	if (NULL == graph1.vertices) /* check if allocation worked */
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);

		return 1;
	}

	if (false == buildGraph(&saveHouses, &edges, &graph1)) /* this will build a graph like structure from all the edges we have */
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph1);

		return 1;
	}

	uint32_t startIndex = findNode(&graph1, globalStartID); /* find the node from which it all starts */

	if (INFINITY32 == startIndex)
	{   /* startNode has no neighbours -> nothing can be reached */
		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph1);

		return 0;
	}

	/* run dijkstra beginning from the start node (calc distance to every other node) */
	if (false == dijkstra(&graph1, startIndex))
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph1);

		return 1;
	}

	savehouses_t validSaveHouses; /* validSaveHouses saves all the id's that can be reached in the first run */
	validSaveHouses.count = 0;
	validSaveHouses.limit = MEMORY_START_SIZE;
	validSaveHouses.data = (uint32_t*)malloc(sizeof(uint32_t) * validSaveHouses.limit);

	if (NULL == validSaveHouses.data) /* check if allocation worked */
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph1);

		return 1;
	}

	register uint32_t i;
	for (i = 0; i < graph1.count; i++) /* every savehouse-nodes with a distance in the range get inserted into validSaveHouses */
	{
		if (graph1.vertices[i].isSaveHouse && graph1.vertices[i].distance <= globalDistance)
		{
			insertSaveHouse(&validSaveHouses, graph1.vertices[i].id);
		}
	}

	freeGraph(&graph1); /* release the old graph */

	/* sort validSaveHouses for searching later */
	qsort(validSaveHouses.data, validSaveHouses.count, sizeof(uint32_t), compare_saveHouses);

	register uint32_t t; /* reverse all the edges (swap start and end id) */
	for (i = 0; i < edges.count; i++)
	{
		t = edges.data[i].startID;
		edges.data[i].startID = edges.data[i].endID;
		edges.data[i].endID = t;
	}

	qsort(edges.data, edges.count, sizeof(edge_t), compare_edges); /* sort the edges to enable bin search */

	t = globalStartID; /* swap start and end */
	globalStartID = globalEndID;
	globalEndID = t;

	graph_t graph2; /* build the reversed graph (end -> start) */
	graph2.count = 0;
	graph2.limit = MEMORY_START_SIZE * 2;
	graph2.vertices = (node_t*)calloc(graph2.limit, sizeof(node_t));

	if (false == buildGraph(&saveHouses, &edges, &graph2)) /* and find all savehouses which can be reached from the end */
	{
		fputs(mallocZeroException, stderr); /* if this fails we exit */

		freeSaveHouses(&saveHouses);
		freeSaveHouses(&validSaveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 1;
	}

	
	startIndex = findNode(&graph2, globalStartID); /* find the node from where to start */

	if (INFINITY32 == startIndex) /* find the node with the start id, if it has no neighbours we exit */
	{
		freeSaveHouses(&saveHouses);
		freeSaveHouses(&validSaveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 0;
	}

	/* and then find every savehouse that is in distance from the end node */
	if (false == dijkstra(&graph2, startIndex))
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeSaveHouses(&validSaveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 1;
	}

	/* the result is the intersection between the results from direction start->end and results from direction end->start */
	for (i = 0; i < graph2.count; i++)
	{
		if (true == graph2.vertices[i].isSaveHouse && graph2.vertices[i].distance <= globalDistance
			&& true == checkSaveHouse(&validSaveHouses, graph2.vertices[i].id))
		{
			fprintf(stdout, "%"PRIu32"\n", graph2.vertices[i].id);
		}
	}

	freeSaveHouses(&saveHouses);
	freeSaveHouses(&validSaveHouses);
	freeEdges(&edges);
	freeGraph(&graph2);

	return 0;
}

bool readData(savehouses_t *saveHouses, edges_t *edges)
{
	uint32_t last = 0; /* temporary value */
	bool firstLine = true; /* just to know if we read the first line (the first line is handled differently) */
	while (1)
	{
		char line[35]; /* buffer */
		char *result = fgets(line, 34, stdin); /* read up to 34 characers (3 numbers of which each is smaller than 4*10^9 + 3 spaces) */

		if (NULL == result && feof(stdin)) /* this means that the file ended before any savhouses were found, which is fine */
		{
			if (firstLine) fputs(inputEmptyException, stderr); /* except for when the input is empty */
			return !firstLine; /* if firstline == true then its empty, else its okay */
		}

		if (NULL == result || 0 != ferror(stdin)) return false; /* this checks if any error occured */

		if (line[0] < '0' || line[0] > '9') return false; /* leading white spaces are not allowed */

		char *start, *end, *distance; /* temporary strings for splitting the input */

		uint32_t startID = strtoul(line, &start, 10); /* parse the first number (start points to after the number) */

		if (start == line) return false; /* error */

		if (ERANGE == errno || startID > 3999999999)
		{
			fputs(numbersOutOfRange, stderr); /* out of range */
			return false;
		}

		/* only accept \n, use dos2unix or something like that if input has Windows line endings (\r\n) */
		if ('\n' == *start || '\0' == *start) /* if the line ends after the first number we entered the savehouse section */
		{
			last = startID; /* save the savehouse for later */
			break; /* go to savehouse section */
		}

		if (start[0] != ' ' || start[1] < '0' || start[1] > '9') return false;

		uint32_t endID = strtoul(start, &end, 10); /* parse the second number */

		if (start == end) return false; /* error */

		if (ERANGE == errno || endID > 3999999999)
		{
			fputs(numbersOutOfRange, stderr); /* out of range */
			return false;
		}

		if (end[0] != ' ' || end[1] < '0' || end[1] > '9') return false;

		uint32_t distanceIn = strtoul(end, &distance, 10); /* try to parse the last bit as the distance*/

		if (distance == end) return false;  /* error */

		if (ERANGE == errno || distanceIn > 3999999999)
		{
			fputs(numbersOutOfRange, stderr); /* out of range */
			return false;
		}

		if (0 == strlen(distance) || '\n' == *distance) /* the triple can only be followed by a newline character (or nothing) */
		{
			if (true == firstLine) /* the very first line has a special purpose */
			{
				globalStartID = startID;
				globalEndID = endID;
				globalDistance = distanceIn;
				firstLine = false;
			}
			else /* every other triple is an edge of the graph */
			{
				edge_t newEdge;
				newEdge.startID = startID;
				newEdge.endID = endID;
				newEdge.distance = distanceIn;

				if (false == insertEdge(edges, &newEdge)) return false; /* try to insert the edge */
			}
		}
		else return false;
	}

	if (false == insertSaveHouse(saveHouses, last)) return false; /* the very first savehouse gets parsed by the "edge-algorithm"
																	so we just insert it here*/
	while (1)
	{
		/* end when the file is empty */
		if (feof(stdin)) return true;

		char line[15];
		char *result = fgets(line, 13, stdin); /* read a line (now we are only looking for one number, so buffer is smaller) */

		if (NULL == result && feof(stdin)) return true; /* if nothing returned and the file is empty we are finished */

		if (NULL == result || 0 != ferror(stdin)) return false; /* this checks if any error occured */

		if (line[0] < '0' || line[0] > '9') return false; /* do not accept leading white spaces */

		char *endPtr;
		uint32_t saveHouse = strtoul(line, &endPtr, 10); /* try to parse whatever into a number */

		if (endPtr == line) return false; /* error */

		if (ERANGE == errno || saveHouse > 3999999999)
		{
			fputs(numbersOutOfRange, stderr);
			return false; /* out of range? */
		}

		if (0 == strlen(endPtr) || '\n' == *endPtr) /* the number can only be followed by newline character (or nothing) */
		{
			if (false == insertSaveHouse(saveHouses, saveHouse)) return false; /* try to insert the savehouse */
		}
		else return false;
	}
}
/*====UTIL ROUTINES============================================================*/


/*====EDGE ROUTINES============================================================*/
bool insertEdge(edges_t *edges, edge_t *e)
{
	if (edges->count == edges->limit) /* check if we reached the memory limit */
	{
		edges->limit = edges->limit << 1; /* if yes increase limit accordingly */

		edge_t *temp = (edge_t*)calloc(edges->limit, sizeof(edge_t)); /* and get new memory in the new size */

		if (temp == NULL) return false; /* check if that worked */

		memcpy(temp, edges->data, sizeof(edge_t) * edges->count); /* copy the old data to the new array */
		free(edges->data); /* free the old data */
		edges->data = temp; /* set our pointer to that new array */
	}

	edges->data[edges->count] = *e; /* insert element at the end */
	edges->count++; /* increase index */

	return true;
}

uint32_t findFirstEdge(edges_t *edges, uint32_t startID)
{
	if (0 == edges->count) return INFINITY32;

	register uint32_t left = 0;
	register uint32_t right = edges->count - 1;

	while (left <= right && right < edges->count)
	{
		register uint32_t middle = left + ((right - left) / 2);
		if (edges->data[middle].startID > startID) right = middle - 1;
		else if (edges->data[middle].startID < startID) left = middle + 1;
		else
		{
			while (middle > 0 && edges->data[middle - 1].startID == startID) middle--;
			return middle;
		}
	}

	return INFINITY32;
}
/*====EDGE ROUTINES============================================================*/


/*====SAVEHOUSE ROUTINES=======================================================*/
bool insertSaveHouse(savehouses_t *saveHouses, uint32_t id)
{
	if (true == checkSaveHouse(saveHouses, id)) return true;

	if (saveHouses->count == saveHouses->limit)
	{
		saveHouses->limit = saveHouses->limit << 1;

		uint32_t *temp = (uint32_t*)malloc(sizeof(uint32_t) * saveHouses->limit);

		if (NULL == temp) return false;

		memcpy(temp, saveHouses->data, sizeof(uint32_t) * saveHouses->count);
		free(saveHouses->data);
		saveHouses->data = temp;
	}

	saveHouses->data[saveHouses->count] = id;
	saveHouses->count++;

	return true;
}

inline bool checkSaveHouse(savehouses_t *saveHouses, uint32_t houseID)
{   /* do a binsearch in the saveHouses */
	return (NULL != bsearch(&houseID, saveHouses->data, saveHouses->count, sizeof(uint32_t), compare_saveHouses));
}
/*====SAVEHOUSE ROUTINES=======================================================*/


/*====COMPARATOR ROUTINES======================================================*/
int compare_edges(const void *e1, const void *e2)
{
	return compare(((edge_t*)e1)->startID, ((edge_t*)e2)->startID);
}

int compare_saveHouses(const void *e1, const void *e2)
{
	return compare(*((uint32_t*)e1), *((uint32_t*)e2));
}

int compare_nodes(const void *e1, const void *e2)
{
	return compare(((node_t*)e1)->id, ((node_t*)e2)->id);
}

inline int compare(uint32_t a, uint32_t b)
{   /* IDs like 0 and 4000000000 would lead to incorect results, so this clipping is needed */
	if (a == b) return 0;
	else if (a < b) return -1;
	else return 1;
}
/*====COMPARATOR ROUTINES======================================================*/


/*====GRAPH ROUTINES===========================================================*/
bool buildGraph(savehouses_t *saveHouses, edges_t *edges, graph_t *graph)
{
	if (NULL == graph->vertices) return false;

	uint32_t currentID = INFINITY32, i = 0;
	for (i = 0; i < edges->count; i++) /* convert all the edges to nodes */
	{
		if (currentID != edges->data[i].startID) /* filter duplicates (the same startid may occour multiple times because one node can have many neighbours) */
		{
			node_t nn; /* create a node */
			nn.id = edges->data[i].startID; /* save id */
			nn.isSaveHouse = checkSaveHouse(saveHouses, nn.id); /* check if it is a savehouse */
			nn.visited = false;

			nn.distance = INFINITY64; /* set distance from startID to INFINITY */

			nn.neighbours = (neighbour_t*)calloc(SUB_NODE_START_SIZE, sizeof(neighbour_t)); /* allocate start memory for the neighbours */
			nn.neighboursCount = 0;
			nn.neighboursLimit = SUB_NODE_START_SIZE;

			if (NULL == nn.neighbours) return false; /* check if that worked */

			if (false == insertNode(graph, nn)) /* insert this node into the list */
			{
				if (NULL != nn.neighbours)
				{
					free(nn.neighbours);
					nn.neighbours = NULL;
				}

				return false;
			}

			currentID = edges->data[i].startID; /* update currentID (skip the next occurences of this id) */
												/* this works because the edges are sorted, the same ids are right behind each other in chunks */
		}
	}

	qsort(graph->vertices, graph->count, sizeof(node_t), compare_nodes); /* sort the nodes to enable binary search */

	for (i = 0; i < edges->count; i++)
	{
		if (INFINITY32 == findNode(graph, edges->data[i].endID))
		{
			/* the endID is an endpoint => it has no extra entry => we have to manually insert this here */
			node_t nn;
			nn.distance = INFINITY64;
			nn.id = edges->data[i].endID;
			nn.isSaveHouse = checkSaveHouse(saveHouses, nn.id);
			nn.neighbours = NULL;
			nn.neighboursCount = 0;
			nn.neighboursLimit = 0;
			nn.visited = false;

			if (false == insertNode(graph, nn)) return false;

			/* to still use binary search we have to sort this again
			this is slow, so hopefully most branches dont just end */
			qsort(graph->vertices, graph->count, sizeof(node_t), compare_nodes);
		}
	}

	for (i = 0; i < graph->count; i++) /* go through all nodes and set its neighbours */
	{
		/* find the first index where this id occurs (since the array is sorted all the elements with the
		same id are in a block, so by finding  the of that block we can iterate through all of them) */
		uint32_t index = findFirstEdge(edges, graph->vertices[i].id);

		currentID = graph->vertices[i].id; /* this is the id we currently want to find */

		/* just work through all the items in the block explained above
		(keep going until we enter the next block) */
		while (index < edges->count && edges->data[index].startID == currentID)
		{
			uint32_t nodeIndex = findNode(graph, edges->data[index].endID); /* find the node with the endID */

			if (false == insertChildNode(&(graph->vertices[i]), nodeIndex, edges->data[index].distance)) return false; /* else try to insert the index into the parent node */

			index++;
		}

		if (0 == graph->vertices[i].neighboursCount)
		{
			free(graph->vertices[i].neighbours);
			graph->vertices[i].neighbours = NULL;
			graph->vertices[i].neighboursCount = 0;
			graph->vertices[i].neighboursLimit = 0;
		}
	}

	return true;
}

bool insertChildNode(node_t *parent, uint32_t indexOfChild, uint64_t distanceToChild)
{
	if (parent->neighboursCount == parent->neighboursLimit)
	{
		parent->neighboursLimit += SUB_NODE_START_SIZE;

		neighbour_t *temp = (neighbour_t*)calloc(parent->neighboursLimit, sizeof(neighbour_t));

		if (NULL == temp) return false;

		memcpy(temp, parent->neighbours, sizeof(neighbour_t) * parent->neighboursCount);
		free(parent->neighbours);

		parent->neighbours = temp;
	}

	neighbour_t nb;
	nb.index = indexOfChild;
	nb.distance = distanceToChild;
	parent->neighbours[parent->neighboursCount] = nb;
	parent->neighboursCount++;

	return true;
}

bool insertNode(graph_t *graph, node_t n)
{
	if (graph->count == graph->limit) /* check if the memory limit is reached */
	{
		graph->limit = graph->limit << 1; /* if => increase size */

		node_t *temp = (node_t*)calloc(graph->limit, sizeof(node_t)); /* get more memory */

		if (NULL == temp) return false; /* check if this worked */

		memcpy(temp, graph->vertices, sizeof(node_t) * graph->count); /* copy data to new memory */
		free(graph->vertices); /* free old data */
		graph->vertices = temp; /* adjust */
	}

	graph->vertices[graph->count] = n; /* insert item */
	graph->count++; /* increment counter */

	return true;
}

uint32_t findNode(graph_t *graph, uint32_t id)
{
	if (0 == graph->count) return INFINITY32; /* if there are no entries its not there */

	register uint32_t left = 0;
	register uint32_t right = graph->count;

	while (left <= right && right <= graph->count) /* perform a binsearch to find the id */
	{
		register uint32_t middle = left + ((right - left) / 2);
		if (id < graph->vertices[middle].id) right = middle - 1;
		else if (id > graph->vertices[middle].id) left = middle + 1;
		else return middle;
	}

	return INFINITY32;
}
/*====GRAPH ROUTINES===========================================================*/


/*====HEAP ROUTINES============================================================*/
bool insertNodeToHeap(graph_t *graph, heap_t *heap, uint32_t element)
{
	if (heap->count > 0)
	{
		/* search through all the elements to find out if this index is 
			allready stored, and if yes: update its position (instead of inserting it multiple times) */
		register uint32_t i;
		for (i = 0; i < heap->count; i++)
		{
			if (heap->data[i] == element)
			{
				siftUpHeap(graph, heap, i);
				return true;
			}
		}
	}

	if (heap->count == heap->limit) /* if the memory limit is reached */
	{
		heap->limit = heap->limit << 1; /* increase memory limit */

		uint32_t *temp = (uint32_t*)calloc(heap->limit, sizeof(uint32_t)); /* get new memory in bigger size */

		if (NULL == temp) return false; /* check if that worked */

		memcpy(temp, heap->data, sizeof(uint32_t) * heap->count); /* copy the data to the new array */
		free(heap->data); /* delete the old array */
		heap->data = temp; /* adjust the pointers */
	}

	heap->data[heap->count] = element; /* set the last item to the new element */
	siftUpHeap(graph, heap, heap->count); /* sift-up the element */

	heap->count++; /* increment the count of elements */

	return true;
}

uint32_t removeMinNodeFromHeap(graph_t *graph, heap_t *heap)
{
	if (heap->count == 0) return INFINITY32;

	uint32_t result = heap->data[0]; /* remove first item (the smallest) */

	heap->count--;

	heap->data[0] = heap->data[heap->count]; /* set new root to the very last element (also decrement size) */

	siftDownHeap(graph, heap, 0); /* restore the heap properties starting from the new root */

	return result; /* return the value */
}

void siftDownHeap(graph_t *graph, heap_t *heap, uint32_t index)
{
	register uint32_t minimum = index; /* assume the root is the biggest */

	if (LEFT(index) < heap->count && graph->vertices[heap->data[LEFT(index)]].distance <= graph->vertices[heap->data[minimum]].distance) /* compare with left child */
		minimum = LEFT(index);

	if (RIGHT(index) < heap->count && graph->vertices[heap->data[RIGHT(index)]].distance <= graph->vertices[heap->data[minimum]].distance) /* compare with right child */
		minimum = RIGHT(index);

	if (minimum != index) /* if the assumetion was wrong */
	{
		register uint32_t t = heap->data[index]; /* just swap the parent with the smaller child */
		heap->data[index] = heap->data[minimum];
		heap->data[minimum] = t;

		siftDownHeap(graph, heap, minimum); /* and go one layer down */
	}
}

void siftUpHeap(graph_t *graph, heap_t *heap, uint32_t index)
{
	if (0 == index) return; /* abort when we reach the root */

	if (graph->vertices[heap->data[index]].distance > graph->vertices[heap->data[PARENT(index)]].distance) return; /* abort when we reached our final position */

	register uint32_t t = heap->data[index]; /* swap the node with its parent */
	heap->data[index] = heap->data[PARENT(index)];
	heap->data[PARENT(index)] = t;


	siftUpHeap(graph, heap, PARENT(index)); /* go one layer up */
}
/*====HEAP ROUTINES============================================================*/


/*====DIJKSTRA ROUTINE=========================================================*/
bool dijkstra(graph_t *graph, uint32_t startIndex)
{
	heap_t heap;
	heap.count = 0;
	heap.limit = SUB_NODE_START_SIZE;
	heap.data = (uint32_t*)calloc(heap.limit, sizeof(uint32_t));

	if (NULL == heap.data) return false;

	graph->vertices[startIndex].distance = 0;

	if (false == insertNodeToHeap(graph, &heap, startIndex))
	{
		if (NULL != heap.data)
		{
			free(heap.data);
			heap.data = NULL;
		}

		return false;
	}
	
	while (heap.count > 0)
	{
		register uint32_t index = removeMinNodeFromHeap(graph, &heap);

		if (index == INFINITY32) break;

		neighbour_t *neighbours = graph->vertices[index].neighbours;
		graph->vertices[index].visited = true;

		uint32_t neighbourIndex = 0;
		for (neighbourIndex = 0; neighbourIndex < graph->vertices[index].neighboursCount; neighbourIndex++)
		{
			uint32_t childIndex = neighbours[neighbourIndex].index;
			uint64_t newDistance = graph->vertices[index].distance + neighbours[neighbourIndex].distance;

			if (false == graph->vertices[childIndex].visited && newDistance <= graph->vertices[childIndex].distance)
			{
				graph->vertices[childIndex].distance = newDistance;

				if (false == insertNodeToHeap(graph, &heap, childIndex))
				{
					if (NULL != heap.data)
					{
						free(heap.data);
						heap.data = NULL;
					}

					return false;
				}
			}
		}
	}

	if (NULL != heap.data)
	{
		free(heap.data);
		heap.data = NULL;
	}

	return true;
}
/*====DIJKSTRA ROUTINE=========================================================*/


/*====FREE ROUTINES============================================================*/
void freeGraph(graph_t *graph)
{
	if (NULL == graph) return;

	register uint32_t i;
	for (i = 0; i < graph->count; i++)
	{
		if (NULL != graph->vertices[i].neighbours)
		{
			free(graph->vertices[i].neighbours);
			graph->vertices[i].neighbours = NULL;
			graph->vertices[i].neighboursCount = 0;
			graph->vertices[i].neighboursLimit = 0;
		}
	}

	free(graph->vertices);
	graph->vertices = NULL;
	graph->count = 0;
	graph->limit = 0;
}

void freeEdges(edges_t *edges)
{
	if (NULL != edges && NULL != edges->data)
	{
		free(edges->data);
		edges->data = NULL;
		edges->count = 0;
		edges->limit = 0;
	}
}

void freeSaveHouses(savehouses_t *saveHouses)
{
	if (NULL != saveHouses && NULL != saveHouses->data)
	{
		free(saveHouses->data);
		saveHouses->data = NULL;
		saveHouses->count = 0;
		saveHouses->limit = 0;
	}
}
/*====FREE ROUTINES============================================================*/