/* Matrikelnummer: 581323 ; Levin Palm <palmlevi@informatik.hu-berlin.de> */
#include <stdio.h> /* fprintf etc. */
#include <stdlib.h> /* malloc/calloc etc. */
#include <stdint.h> /* uint32_t etc. */
#include <string.h> /* memcpy, memset */
#include <limits.h> /* UINT32_MAX etc. */
#include <stdbool.h> /* bool type */
#include <inttypes.h> /* PRIu32 etc. */
#include <errno.h>  /* error handling */

/* How much memory should be allocated in the beginning */
#define MEMORY_START_SIZE 32 /* for data */
#define SUB_NODE_START_SIZE 2 /* for neighbours in a node */
#define INFINITY32 UINT32_MAX /* infinity for dijkstra, because all numbers are smaller */
#define INFINITY64 UINT64_MAX /* than 4*10^9 we can use the values above that for whatever */

#define RESULT_OK 0x0			/* error codes for readData() */
#define RESULT_MALLOC_ERR 0x1
#define RESULT_INPUT_ERR 0x2
#define RESULT_OUT_OF_RANGE 0x4
#define RESULT_INPUT_EMPTY 0x8

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
	uint32_t *mapping; /* hastable for mapping ids to indices */
} graph_t;

bool buildGraph(savehouses_t*, edges_t*, graph_t*);

bool dijkstra(graph_t*, uint32_t); /* perform dijkstra on graph starting with index */

uint32_t findNode(graph_t*, uint32_t); /* find node with id in graph and give index */
bool insertChildNode(node_t*, uint32_t, uint64_t); /* inserts a childnode into the parent node */
bool insertNode(graph_t*, node_t); /* inserts a node into the node collection */

#define LEFT(INDEX) ((INDEX + INDEX) + 1) /* calculate left child */
#define RIGHT(INDEX) ((INDEX + INDEX) + 2) /* calculate right child */
#define PARENT(INDEX) ((INDEX - 1) >> 1) /* calculate parent index */

typedef struct heap_t /* this represents the heap */
{
	uint32_t count; /* how many elements are in the heap */
	uint32_t limit; /* how much capacity the heap currently has */
	uint32_t *data; /* the data */
	uint32_t *positions; /* saves which element is where in the heap (boost) */
} heap_t;

bool insertNodeToHeap(graph_t*, heap_t*, uint32_t); /* this inserts the given value into the heap */
uint32_t removeMinNodeFromHeap(graph_t*, heap_t*); /* this gets the "first" (the smallest) element from the heap */
void siftDownHeap(graph_t*, heap_t*, uint32_t); /* this is more for internal use, but basically */
void siftUpHeap(graph_t*, heap_t*, uint32_t);  /* makes sure the heap is a heap after changing values */

int compare_edges(const void*, const void*); /* these three are just wrappers for compare() */
int compare_saveHouses(const void*, const void*);
int compare_nodes(const void*, const void*);

void freeGraph(graph_t*); /* helper methods for freeing complex structures */
void freeEdges(edges_t*);
void freeSaveHouses(savehouses_t*);

int readData(savehouses_t*, edges_t*); /* reads in the data from stdin */

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

	int result = readData(&saveHouses, &edges); /* try to read in the data, if anything is not correct != 0 gets returned */
	if (result != RESULT_OK)
	{
		switch (result)
		{
		case RESULT_INPUT_EMPTY:
			fputs(inputEmptyException, stderr);
			break;
		case RESULT_MALLOC_ERR:
			fputs(mallocZeroException, stderr);
			break;
		case RESULT_OUT_OF_RANGE:
			fputs(numbersOutOfRange, stderr);
			break;
		case RESULT_INPUT_ERR:
		default:
			fputs(invalidFormatException, stderr);
			break;
		}

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
	graph1.mapping = (uint32_t*)malloc(graph1.limit * sizeof(uint32_t));

	if (NULL == graph1.vertices) /* check if allocation worked */
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);

		return 1;
	}

	if (!buildGraph(&saveHouses, &edges, &graph1)) /* this will build a graph like structure from all the edges we have */
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

		return 1;
	}

	/* run dijkstra beginning from the start node (calc distance to every other node) */
	if (!dijkstra(&graph1, startIndex))
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph1);

		return 1;
	}

	freeSaveHouses(&saveHouses); /* delete all the old saveHouses */
	saveHouses.data = (uint32_t*)calloc(MEMORY_START_SIZE, sizeof(uint32_t)); /* and create "new" ones */
	saveHouses.limit = MEMORY_START_SIZE;
	saveHouses.count = 0;
	
	for (size_t i = 0; i < graph1.count; i++) 
	{   /* because on the second run we only have to check the savehouses that could be reached in the first run */
		if (graph1.vertices[i].isSaveHouse && graph1.vertices[i].distance <= globalDistance)
		{
			insertSaveHouse(&saveHouses, graph1.vertices[i].id);
		}
	}

	freeGraph(&graph1); /* release the old graph */

	qsort(saveHouses.data, saveHouses.count, sizeof(uint32_t), compare_saveHouses);

	for (size_t i = 0; i < edges.count; i++) /* reverse all the edges (swap start and end id) */
	{
		uint32_t t = edges.data[i].startID;
		edges.data[i].startID = edges.data[i].endID;
		edges.data[i].endID = t;
	}

	qsort(edges.data, edges.count, sizeof(edge_t), compare_edges); /* sort the edges to enable binsearch */

	uint32_t t = globalStartID; /* swap start and end */
	globalStartID = globalEndID;
	globalEndID = t;

	graph_t graph2; /* build the reversed graph (end -> start) */
	graph2.count = 0;
	graph2.limit = MEMORY_START_SIZE * 2;
	graph2.vertices = (node_t*)calloc(graph2.limit, sizeof(node_t));
	graph2.mapping = (uint32_t*)malloc(graph2.limit * sizeof(uint32_t));

	if (!buildGraph(&saveHouses, &edges, &graph2)) /* and find all savehouses which can be reached from the end */
	{
		fputs(mallocZeroException, stderr); /* if this fails we exit */

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 1;
	}

	startIndex = findNode(&graph2, globalStartID); /* find the node from where to start */

	if (INFINITY32 == startIndex) /* find the node with the start id, if it has no neighbours we exit */
	{
		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 0;
	}

	/* and then find every savehouse that is in distance from the end node */
	if (!dijkstra(&graph2, startIndex))
	{
		fputs(mallocZeroException, stderr);

		freeSaveHouses(&saveHouses);
		freeEdges(&edges);
		freeGraph(&graph2);

		return 1;
	}

	/* the results are all the saveHouses that are still valid after the second run */
	for (size_t i = 0; i < graph2.count; i++)
	{
		if (true == graph2.vertices[i].isSaveHouse && graph2.vertices[i].distance <= globalDistance)
		{
			fprintf(stdout, "%"PRIu32"\n", graph2.vertices[i].id);
			fflush(stdout);
		}
	}

	freeSaveHouses(&saveHouses);
	freeEdges(&edges);
	freeGraph(&graph2);

	return 0;
}

int readData(savehouses_t *saveHouses, edges_t *edges)
{
	uint32_t last = 0; /* temporary value */
	bool firstLine = true; /* just to know if we read the first line (the first line is handled differently) */
	while (1)
	{
		char line[35]; /* buffer */
		char *result = fgets(line, 34, stdin); /* read up to 34 characers (3 numbers of which each is smaller than 4*10^9 + 3 spaces) */

		if (NULL == result && feof(stdin)) /* this means that the file ended before any savhouses were found, which is fine */
		{
			if (firstLine) return RESULT_INPUT_EMPTY;
			return RESULT_OK;
		}

		if (NULL == result || 0 != ferror(stdin)) return RESULT_INPUT_ERR; /* this checks if any error occured */

		if (line[0] < '0' || line[0] > '9') return RESULT_INPUT_ERR; /* leading white spaces are not allowed */

		char *start, *end, *distance; /* temporary strings for splitting the input */

		uint32_t startID = strtoul(line, &start, 10); /* parse the first number (start points to after the number) */

		if (start == line) return RESULT_INPUT_ERR; /* error */

		if (ERANGE == errno || startID > 3999999999) return RESULT_OUT_OF_RANGE;

		/* only accept \n, use dos2unix or something like that if input has Windows line endings (\r\n) */
		if ('\n' == *start || '\0' == *start) /* if the line ends after the first number we entered the savehouse section */
		{
			last = startID; /* save the savehouse for later */
			break; /* go to savehouse section */
		}

		if (' ' != start[0] || start[1] < '0' || start[1] > '9') return RESULT_INPUT_ERR;

		uint32_t endID = strtoul(start, &end, 10); /* parse the second number */

		if (start == end) return RESULT_INPUT_ERR;  /* error */

		if (ERANGE == errno || endID > 3999999999) return RESULT_OUT_OF_RANGE;

		if (end[0] != ' ' || end[1] < '0' || end[1] > '9') return RESULT_INPUT_ERR;

		uint32_t distanceIn = strtoul(end, &distance, 10); /* try to parse the last bit as the distance*/

		if (distance == end) return RESULT_INPUT_ERR;  /* error */

		if (ERANGE == errno || distanceIn > 3999999999) return RESULT_OUT_OF_RANGE;

		if (0 == strlen(distance) || '\n' == *distance) /* the triple can only be followed by a newline character (or nothing) */
		{
			if (firstLine) /* the very first line has a special purpose */
			{
				globalStartID = startID;
				globalEndID = endID;
				globalDistance = distanceIn;
				firstLine = false;
			}
			else /* every other triple is an edge of the graph */
			{
				if (distanceIn <= globalDistance) /* filter out edges which are too long anyways */
				{
					edge_t newEdge;
					newEdge.startID = startID;
					newEdge.endID = endID;
					newEdge.distance = distanceIn;

					if (false == insertEdge(edges, &newEdge)) return RESULT_MALLOC_ERR; /* try to insert the edge */
				}
			}
		}
		else return RESULT_INPUT_ERR;
	}

	if (!insertSaveHouse(saveHouses, last)) return RESULT_MALLOC_ERR; /* the very first savehouse gets parsed by the "edge-algorithm" so we just insert it here*/

	while (1)
	{
		/* end when the file is empty */
		if (feof(stdin)) return RESULT_OK;

		char line[15];
		char *result = fgets(line, 13, stdin); /* read a line (now we are only looking for one number, so buffer is smaller) */

		if (NULL == result && feof(stdin)) return RESULT_OK; /* if nothing returned and the file is empty we are finished */

		if (NULL == result || 0 != ferror(stdin)) return RESULT_INPUT_ERR; /* this checks if any error occured */

		if (line[0] < '0' || line[0] > '9') return RESULT_INPUT_ERR; /* do not accept leading white spaces */

		char *endPtr;
		uint32_t saveHouse = strtoul(line, &endPtr, 10); /* try to parse whatever into a number */

		if (endPtr == line) return RESULT_INPUT_ERR; /* error */

		if (ERANGE == errno || saveHouse > 3999999999) return RESULT_OUT_OF_RANGE;

		if (0 == strlen(endPtr) || '\n' == *endPtr) /* the number can only be followed by newline character (or nothing) */
		{
			if (!insertSaveHouse(saveHouses, saveHouse)) return RESULT_MALLOC_ERR; /* try to insert the savehouse */
		}
		else return RESULT_INPUT_ERR;
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

	uint32_t left = 0;
	uint32_t right = edges->count - 1;

	while (left <= right && right < edges->count)
	{
		uint32_t middle = left + ((right - left) / 2);
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
	if (checkSaveHouse(saveHouses, id)) return true;

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

bool checkSaveHouse(savehouses_t *saveHouses, uint32_t houseID)
{   /* do a binsearch in the saveHouses */
	return (NULL != bsearch(&houseID, saveHouses->data, saveHouses->count, sizeof(uint32_t), compare_saveHouses));
}
/*====SAVEHOUSE ROUTINES=======================================================*/


/*====COMPARATOR ROUTINES======================================================*/
int compare_edges(const void *e1, const void *e2)
{
	if (((edge_t*)e1)->startID == ((edge_t*)e2)->startID) return 0;
	else if (((edge_t*)e1)->startID < ((edge_t*)e2)->startID) return -1;
	else return 1;
}

int compare_saveHouses(const void *e1, const void *e2)
{
	if (*((uint32_t*)e1) == *((uint32_t*)e2)) return 0;
	else if (*((uint32_t*)e1) < *((uint32_t*)e2)) return -1;
	else return 1;
}

int compare_nodes(const void *e1, const void *e2)
{
	if (((node_t*)e1)->id == ((node_t*)e2)->id) return 0;
	else if (((node_t*)e1)->id < ((node_t*)e2)->id) return -1;
	else return 1;
}
/*====COMPARATOR ROUTINES======================================================*/


/*====GRAPH ROUTINES===========================================================*/
bool buildGraph(savehouses_t *saveHouses, edges_t *edges, graph_t *graph)
{
	if (NULL == graph->vertices /*|| NULL == graph->mapping*/) return false;

	uint32_t currentID = INFINITY32;
	for (size_t i = 0; i < edges->count; i++) /* convert all the edges to nodes */
	{
		if (currentID == edges->data[i].startID) continue; /* filter duplicates (the same startid may occour multiple times because one node can have many neighbours) */

		node_t nn; /* create a node */
		nn.id = edges->data[i].startID; /* save id */
		nn.isSaveHouse = checkSaveHouse(saveHouses, nn.id); /* check if it is a savehouse */
		nn.visited = false;
		nn.distance = INFINITY64; /* set distance from startID to INFINITY */

		nn.neighbours = (neighbour_t*)calloc(SUB_NODE_START_SIZE, sizeof(neighbour_t)); /* allocate start memory for the neighbours */
		nn.neighboursCount = 0;
		nn.neighboursLimit = SUB_NODE_START_SIZE;

		if (NULL == nn.neighbours) return false; /* check if that worked */

		if (!insertNode(graph, nn)) /* insert this node into the list */
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

	qsort(graph->vertices, graph->count, sizeof(node_t), compare_nodes); /* sort the nodes to enable binary search */

	if (INFINITY32 == findNode(graph, globalEndID)) /* check that we haven't missed the end node */
	{
		node_t nn;
		nn.distance = INFINITY64;
		nn.id = globalEndID;
		nn.isSaveHouse = checkSaveHouse(saveHouses, nn.id);
		nn.neighbours = NULL;
		nn.neighboursCount = 0;
		nn.neighboursLimit = 0;
		nn.visited = false;

		if (!insertNode(graph, nn)) return false;

		/* find position where this node belongs */
		for (size_t j = graph->count - 1; j > 0; j--)
		{
			if (graph->vertices[j].id < graph->vertices[j - 1].id)
			{
				node_t temp = graph->vertices[j];
				graph->vertices[j] = graph->vertices[j - 1];
				graph->vertices[j - 1] = temp;
			}
			else break;
		}
	}

	for (size_t i = 0; i < graph->count; i++) /* go through all nodes and set its neighbours */
	{
		/* find the first index where this id occurs (since the array is sorted all the elements with the
		same id are in a block, so by finding the first of that block we can iterate through all of them) */
		uint32_t index = findFirstEdge(edges, graph->vertices[i].id);

		if (INFINITY32 != index)
		{
			currentID = graph->vertices[i].id; /* this is the id we currently want to find */

											   /* just work through all the items in the block explained above
											   (keep going until we enter the next block) */
			while (index < edges->count && edges->data[index].startID == currentID)
			{
				uint32_t nodeIndex = findNode(graph, edges->data[index].endID); /* find the node with the endID */
			
				if (INFINITY32 != nodeIndex) /* nodeIndex == INFINITY32 means that the corresponding node hat no outgoing edges and is therefore useless */
					if (!insertChildNode(&(graph->vertices[i]), nodeIndex, edges->data[index].distance))
						return false; /* and try to insert the index into the parent node */

				index++;
			}
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
	if (parent->neighboursCount == parent->neighboursLimit) /* check if there is space left */
	{
		parent->neighboursLimit += SUB_NODE_START_SIZE; /* if no, increase memory  */

		neighbour_t *temp = (neighbour_t*)calloc(parent->neighboursLimit, sizeof(neighbour_t)); /* allocate more */

		if (NULL == temp) return false; /* check if it worked */

		memcpy(temp, parent->neighbours, sizeof(neighbour_t) * parent->neighboursCount); /* copy the content and free old data */
		free(parent->neighbours);
		parent->neighbours = temp;
	}

	neighbour_t nb; /* create new neighbour */
	nb.index = indexOfChild;
	nb.distance = distanceToChild;
	parent->neighbours[parent->neighboursCount] = nb; /* put it in the list */
	parent->neighboursCount++; /* count up */

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

		uint32_t *temp2 = (uint32_t*)calloc(graph->limit, sizeof(uint32_t)); /* get more memory */

		if (NULL == temp2) return false; /* check if this worked */

		memcpy(temp2, graph->mapping, sizeof(uint32_t) * graph->count); /* copy data to new memory */
		free(graph->mapping); /* free old data */
		graph->mapping = temp2; /* adjust */
	}

	graph->vertices[graph->count] = n; /* insert item */
	graph->mapping[graph->count] = n.id;
	graph->count++; /* increment counter */

	return true;
}

uint32_t findNode(graph_t *graph, uint32_t id)
{   /* TODO: make this efficient (dont ic it is yet); this function is executed >50% of runtime */
	if (0 == graph->count) return INFINITY32; /* if there are no entries its not there */

	if (graph->vertices[0].id == id) return 0; /* check borders because they are slow to reach with binsearch */
	if (graph->vertices[graph->count - 1].id == id) return graph->count - 1;

	uint32_t left = 0, right = graph->count, middle = 0; /* perform a binsearch to find the id */

	while (left <= right && right <= graph->count)
	{
		middle = left + ((right - left) >> 1);
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
	if (INFINITY32 != heap->positions[element]) /* check if the element is in the heap allready */
	{
		siftUpHeap(graph, heap, heap->positions[element]); /* if yes just update the position */

		return true;
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

	heap->positions[element] = heap->count;
	heap->data[heap->count] = element; /* set the last item to the new element */
	siftUpHeap(graph, heap, heap->count); /* sift-up the element */

	heap->count++; /* increment the count of elements */

	return true;
}

uint32_t removeMinNodeFromHeap(graph_t *graph, heap_t *heap)
{
	if (0 == heap->count) return INFINITY32;

	uint32_t result = heap->data[0]; /* remove first item (the smallest) */

	heap->count--;

	heap->data[0] = heap->data[heap->count]; /* set new root to the very last element (also decrement size) */
	heap->positions[heap->data[0]] = 0; /* mark position as free */
	siftDownHeap(graph, heap, 0); /* restore the heap properties starting from the new root  and update position */

	heap->positions[result] = INFINITY32; /* mark returned item as deleted */

	return result; /* return the value */
}

void siftDownHeap(graph_t *graph, heap_t *heap, uint32_t index)
{
	uint32_t localIndex = index;
	uint32_t minimum = index; /* assume the root is the biggest */
	while (true)
	{
		if (LEFT(localIndex) < heap->count) /* compare with left child */
		{
			if (graph->vertices[heap->data[LEFT(localIndex)]].distance < graph->vertices[heap->data[minimum]].distance)
				minimum = LEFT(localIndex);

			/* if the left children does not exists the right children can not exist */
			if (RIGHT(localIndex) < heap->count && graph->vertices[heap->data[RIGHT(localIndex)]].distance < graph->vertices[heap->data[minimum]].distance) /* compare with right child */
				minimum = RIGHT(localIndex);
		}

		if (minimum == localIndex) return;

		/* if the assumetion was wrong */
		uint32_t t = heap->data[localIndex]; /* just swap the parent with the smaller child */
		heap->data[localIndex] = heap->data[minimum];
		heap->data[minimum] = t;

		t = heap->positions[heap->data[localIndex]];
		heap->positions[heap->data[localIndex]] = heap->positions[heap->data[minimum]];
		heap->positions[heap->data[minimum]] = t;

		localIndex = minimum; /* go one layer down */
	}
}

void siftUpHeap(graph_t *graph, heap_t *heap, uint32_t index)
{
	if (0 == index) return; /* abort when we reach the root */

	if (graph->vertices[heap->data[index]].distance >= graph->vertices[heap->data[PARENT(index)]].distance) return; /* abort when we reached our final position */

	uint32_t t = heap->data[index]; /* swap the node with its parent */
	heap->data[index] = heap->data[PARENT(index)];
	heap->data[PARENT(index)] = t;

	t = heap->positions[heap->data[index]]; /* also swap the positions they are registered in */
	heap->positions[heap->data[index]] = heap->positions[heap->data[PARENT(index)]];
	heap->positions[heap->data[PARENT(index)]] = t;

	siftUpHeap(graph, heap, PARENT(index)); /* go one layer up */
}
/*====HEAP ROUTINES============================================================*/


/*====DIJKSTRA ROUTINE=========================================================*/
bool dijkstra(graph_t *graph, uint32_t startIndex)
{
	heap_t heap; /* create new heap for dijkstra */
	heap.count = 0;
	heap.limit = graph->count / 2; /* TODO: find good start size */
	heap.data = (uint32_t*)calloc(heap.limit, sizeof(uint32_t));

	heap.positions = (uint32_t*)malloc(graph->count * sizeof(uint32_t)); /* positions saves the index in the heap array of each possible item */

	if (NULL == heap.positions || NULL == heap.data) return false; /* check if the allocations worked */

	memset(heap.positions, INFINITY32, graph->count * sizeof(uint32_t)); /* initalize the positions-array to all be infinity (not in heap) */

	graph->vertices[startIndex].distance = 0; /* the startnode can reach its self in no time */

	if (!insertNodeToHeap(graph, &heap, startIndex)) /* insert the startnode into the heap */
	{
		if (NULL != heap.data) free(heap.data);  		/* free heap if neccessary */
		if (NULL != heap.positions) free(heap.positions);

		return false;
	}

	while (heap.count > 0) /* while there are unprocessed nodes we continue */
	{
		register uint32_t index = removeMinNodeFromHeap(graph, &heap); /* get the next node */

		if (index == INFINITY32) break; /* return on error */

		neighbour_t *neighbours = graph->vertices[index].neighbours; /* get the neighbours from that node */
		graph->vertices[index].visited = true; /* mark it as visited */

		uint32_t neighbourIndex;
		for (neighbourIndex = 0; neighbourIndex < graph->vertices[index].neighboursCount; neighbourIndex++) /* and update distance to all its neighbours */
		{
			uint32_t childIndex = neighbours[neighbourIndex].index;
			uint64_t newDistance = graph->vertices[index].distance + neighbours[neighbourIndex].distance; /* calculate new distance */

			if (!graph->vertices[childIndex].visited && newDistance <= graph->vertices[childIndex].distance) /* check if distance needs to be updated */
			{
				graph->vertices[childIndex].distance = newDistance; /* update if neccessary */

				if (!insertNodeToHeap(graph, &heap, childIndex)) /* put the unseen neighbours into the heap */
				{
					if (NULL != heap.data) free(heap.data); /* free heap if neccessary */
					if (NULL != heap.positions) free(heap.positions);

					return false;
				}
			}
		}
	}

	if (NULL != heap.data) free(heap.data); /* free heap if neccessary */
	if (NULL != heap.positions) free(heap.positions);

	return true;
}
/*====DIJKSTRA ROUTINE=========================================================*/


/*====FREE ROUTINES============================================================*/
void freeGraph(graph_t *graph)
{
	if (NULL == graph) return; /* check that pointer is valid */

	for (size_t i = 0; i < graph->count; i++) /* interate through all the nodes of the graph */
	{
		if (NULL != graph->vertices[i].neighbours) /* check for valid neighbour collections */
		{
			free(graph->vertices[i].neighbours); /* free neighbours of each node */
			graph->vertices[i].neighbours = NULL; /* indicate that is free */
			graph->vertices[i].neighboursCount = 0; /* update meta data */
			graph->vertices[i].neighboursLimit = 0;
		}
	}

	if (NULL != graph->vertices) free(graph->vertices); /* delete the nodes itself */
	if (NULL != graph->mapping) free(graph->mapping);
	graph->vertices = NULL; /* mark it as freed */
	graph->mapping = NULL; 
	graph->count = 0; /* meta data */
	graph->limit = 0;
}

void freeEdges(edges_t *edges)
{
	if (NULL != edges && NULL != edges->data) /* check that the pointer is valid */
	{
		free(edges->data); /* free the data */
		edges->data = NULL; /* indicate that this was freed */
		edges->count = 0; /* update meta data */
		edges->limit = 0;
	}
}

void freeSaveHouses(savehouses_t *saveHouses)
{
	if (NULL != saveHouses && NULL != saveHouses->data) /* check that the pointer is valid */
	{
		free(saveHouses->data); /* free the data */
		saveHouses->data = NULL; /* indicate that this was freed */
		saveHouses->count = 0; /* update meta data */
		saveHouses->limit = 0;
	}
}
/*====FREE ROUTINES============================================================*/