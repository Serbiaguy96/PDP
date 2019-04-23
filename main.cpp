#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <omp.h>
#include <mpi.h>

using namespace std;

struct Node {
	vector<vector<int>> board;
	int  value = 0;
};

struct position {
	int x;
	int y;
};


struct QueuedNode {
	Node node;
	int next_id = 1;
};


struct Tile {
	int value;
	int length;
	int horizontal;
};

class Algorithm
{
private:
	int max_price;
	vector<vector<int>> board;
	vector<QueuedNode> q;
	int cols;
	int rows;
	int i1;
	int i2;
	int c1;
	int c2;
	int cn;
	int depth = 8 * 5;

public:
	Algorithm();
	Algorithm(int cols, int rows, int i1, int i2, int c1, int c2, int cn);
	void run(vector<vector<int>> b, Tile * tiles, int id);
	int countFreeTiles(vector<vector<int>> board);
	int evalPoi(int number);
	void printBoard(vector<vector<int>> board);
	void BBAlgorithm(Node n,int id, Tile * tiles);
	position getFreePosition(vector<vector<int>> board);
	Node createNewNode(Node n, int length, int value, int horizontal, position p, int id);
	bool freePath(vector<vector<int>> board, bool horizontal, int length, position p);
	int getMaxPrice(void);
	vector<vector<int>> getBoard(void);
	queue<QueuedNode> fillQueue(vector<vector<int>> b, Tile * tiles, int id);
	void printQueue(void);
	int getRows(void);
	int getCols(void);
};

struct Box {
	vector<vector<int>> board;
	int value;
	Algorithm a;
	int id;
	Tile * tiles;
};

int Algorithm::getCols(void)
{
	return cols;
}

int Algorithm::getRows(void)
{
	return rows;
}

Algorithm::Algorithm()
{

}

Algorithm::Algorithm(int cols, int rows, int i1, int i2, int c1, int c2, int cn)
{
	this->max_price = -100000000;
	this->cols = cols;
	this->rows = rows;
	this->i1 = i1;
	this->i1 = i1;
	this->i2 = i2;
	this->c1 = c1;
	this->c2 = c2;
	this->cn = cn;
}

vector<vector<int>> Algorithm::getBoard()
{
	return board;
}

int Algorithm::getMaxPrice()
{
	return max_price;
}

void Algorithm::run(vector<vector<int>> b, Tile * tiles, int id)
{
	Node n;
	queue<QueuedNode> q_node;
	n.board = b;
	q_node = fillQueue(b, tiles, 1);

	while (!q_node.empty())
	{
		q.push_back(q_node.front());
		q_node.pop();
	}
	unsigned int i = 0;
	//printQueue();
	#pragma omp parallel for private(i)
	for (i = 0; i < q.size(); i++)
	{
		BBAlgorithm(q[i].node, q[i].next_id, tiles);
	}

	//cout << "Final look of board: " << endl;

	//printBoard(board);

	//cout << "With value : " << max_price << endl;
}

void Algorithm::BBAlgorithm(Node n, int id, Tile * tiles)
{
	position pos = getFreePosition(n.board);

	if (pos.x == -1 && pos.y == -1)
	{
		if (n.value > max_price)
		{
			#pragma omp critical
			{
				if (n.value > max_price)
				{
					max_price = n.value;
					board = n.board;

					//printBoard(board);
				}
			}
		}
		return;
	}


	for (int i = 0; i < 5; i++)
	{
		if (tiles[i].horizontal && pos.y + tiles[i].length <= cols && freePath(n.board, 1, tiles[i].length, pos) && max_price < evalPoi(countFreeTiles(n.board) - tiles[i].length) + n.value + tiles[i].value)
		{
				BBAlgorithm(createNewNode(n, tiles[i].length, tiles[i].value, 1, pos, id), id + 1, tiles);
		} else if (!tiles[i].horizontal && pos.x + tiles[i].length <= rows && freePath(n.board, 0, tiles[i].length, pos) && max_price < evalPoi(countFreeTiles(n.board) - tiles[i].length) + n.value + tiles[i].value)
		{
				BBAlgorithm(createNewNode(n, tiles[i].length, tiles[i].value, 0, pos, id), id + 1, tiles);
		} else if (i == 4 && n.board[pos.x][pos.y] == 0  && max_price < evalPoi(countFreeTiles(n.board) - 1) + n.value + tiles[i].value)
		{
				BBAlgorithm(createNewNode(n, tiles[i].length, tiles[i].value, -1, pos, id), id + 1, tiles);
		}
	}
	return;
}

queue<QueuedNode> Algorithm::fillQueue(vector<vector<int>> b, Tile * tiles, int id_n)
{
	QueuedNode n;
	int id = id_n;
	int nodes_count = 1;
	int nodes_to_add = 0;
	n.node.board = b;
	n.next_id = id;
	queue<QueuedNode> q_tmp;

	q_tmp.push(n);

	QueuedNode node_to_process;
	while (nodes_count < depth)
	{
		nodes_to_add = 0;
		node_to_process = q_tmp.front();
		position pos = getFreePosition(node_to_process.node.board);
		for(int i = 0; i < 5; i++)
		{
			if (tiles[i].horizontal && pos.y + tiles[i].length <= cols && freePath(node_to_process.node.board, 1, tiles[i].length, pos))
			{
				n.node = createNewNode(node_to_process.node, tiles[i].length, tiles[i].value, 1, pos, node_to_process.next_id);
				n.next_id = node_to_process.next_id + 1;
				q_tmp.push(n);
				nodes_to_add += 1;
			} else if (!tiles[i].horizontal && pos.x + tiles[i].length <= rows && freePath(node_to_process.node.board, 0, tiles[i].length, pos))
			{
				n.node = createNewNode(node_to_process.node, tiles[i].length, tiles[i].value, 0, pos, node_to_process.next_id);
				n.next_id = node_to_process.next_id + 1;
				q_tmp.push(n);
				nodes_to_add += 1;
			} else if ( i == 4 && node_to_process.node.board[pos.x][pos.y] == 0)
			{
				n.node = createNewNode(node_to_process.node, tiles[i].length, tiles[i].value, -1, pos, node_to_process.next_id);
				n.next_id = node_to_process.next_id + 1;
				q_tmp.push(n);
				nodes_to_add += 1;
			}
		}
		nodes_count += nodes_to_add - 1;
		id += 1;
	}

	return q_tmp;
}

void Algorithm::printQueue(void)
{
	for (unsigned int i = 0; i < q.size(); i++)
	{
		printBoard(q[i].node.board);
	}
}

Node Algorithm::createNewNode(Node n, int length, int value, int horizontal, position p, int id)
{
	Node new_Node;
	new_Node.board = n.board;
	new_Node.value = n.value + value;

	if (horizontal == 0)
	{
		for (int i = p.x; i < p.x + length; i++)
		{
			new_Node.board[i][p.y] = id;
		}
	} else if (horizontal == 1) {
		for (int i = p.y; i < p.y + length; i++)
		{
			new_Node.board[p.x][i] = id;
		}
	} else {
		new_Node.board[p.x][p.y] = -2;
	}

	return new_Node;
}

int Algorithm::countFreeTiles(vector<vector<int>> board)
{
	int freeTiles = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++ )
		{
			if (board[i][j] == 0)
			{
				freeTiles++;
			}
		}
	}
	return freeTiles;
}

int Algorithm::evalPoi(int number)
{
	int i,x;
	int max=c2*(number/i2);
	int zb=number%i2;
	max+=c1*(zb/i1);

	zb=zb%i1;
	max+=zb*cn;

	for(i=0;i<(number/i2);i++)
	{
	x=c2*i;
	zb=number-i*i2;
	x+=c1*(zb/i1);
	zb=zb%i1;
	x+=zb*cn;

	if (x>max) max=x;
	}
	return max;
}

void Algorithm::printBoard(vector<vector<int>> board)
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			cout << board[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

position Algorithm::getFreePosition(vector<vector<int>> b)
{
	position ret;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (b[i][j] == 0)
			{
				ret.x = i;
				ret.y = j;
				return ret;
			}
		}
	}

	ret.x = -1;
	ret.y = -1;
	return ret;
}

bool Algorithm::freePath(vector<vector<int>> board, bool horizontal, int length, position p)
{
	if (horizontal)
	{
		for (int i = p.y; i < p.y + length; i++)
		{
			if (board[p.x][i] != 0)
			{
				return 0;
			}
		}
		return 1;
	} else {
		for (int i = p.x; i < p.x + length; i++)
		{
			if (board[i][p.y] != 0)
			{
				return 0;
			}
		}
		return 1;
	}
}

vector<vector<int>> setBoard(int rows, int cols)
{
	vector<vector<int>> b;
	for (int i = 0; i < rows; i++)
	{
		vector<int> tmp;
		for (int j = 0; j < cols; j++)
		{
			 tmp.push_back(0);
		}
		b.push_back(tmp);
	}
	return b;
}

int *  createArrFromClass(vector<vector<int>> b, int next_id, Tile * tiles, int rows, int cols, int value)
{
	int * arr = new int[1024];
	int index = 0;

	arr[index++] = rows;
	arr[index++] = cols;

	for (int i = 0; i < 5; i++)
	{
		arr[index++] = tiles[i].value;
		arr[index++] = tiles[i].length;
		arr[index++] = tiles[i].horizontal;
	}

	arr[index++] = next_id;
	arr[index++] = value;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			arr[index++] = b[i][j];
		}
	}

	return arr;

}

Box createBoardFromArr(int * arr)
{
	Box b;
	int i1, i2, c1, c2, cn, cols, rows, id, value;
	Tile tiles[5];

	rows = arr[0];
	cols = arr[1];
	c2 = arr[2];
	i2 = arr[3];
	c1 = arr[8];
	i1 = arr[9];
	cn = arr[14];
	id = arr[17];
	value = arr[18];

	Algorithm a(cols, rows, i1, i2, c1, c2, cn);

	tiles[0].value = c2;
	tiles[0].length = i2;
	tiles[0].horizontal = 1;
	tiles[1].value = c2;
	tiles[1].length = i2;
	tiles[1].horizontal = 0;
	tiles[2].value = c1;
	tiles[2].length = i1;
	tiles[2].horizontal = 1;
	tiles[3].value = c1;
	tiles[3].length = i1;
	tiles[3].horizontal = 0;
	tiles[4].value = cn;
	tiles[4].length = 1;
	tiles[4].horizontal = -1;


	int index = 19;
	vector<vector<int>> board;
	for (int i = 0; i < rows; i++)
	{
		vector<int> tmp;
		for (int j = 0; j < cols; j++)
		{
			tmp.push_back(arr[index++]);
		}
		board.push_back(tmp);
	}

	b.board = board;
	b.a = a;
	b.id = id;
	b.value = value;
	b.tiles = tiles;

	return b;

}

int * createSolution(vector<vector<int>> board, int value, int rows, int cols)
{
	int * arr = new int[1024];

	int index = 0;

	arr[index++] = value;
	arr[index++] = rows;
	arr[index++] = cols;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			arr[index++] = board[i][j];
		}
	}

	return arr;
}

Node parseSolution(int * arr)
{
	Node n;
	int index = 0;
	int cols, rows;
	vector<vector<int>> b;

	n.value = arr[index++];
	rows = arr[index++];
	cols = arr[index++];

	for (int i = 0; i < rows; i++)
	{
		vector<int> tmp;
		for (int j = 0; j < cols; j++)
		{
			tmp.push_back(arr[index++]);
		}
		b.push_back(tmp);
	}

	n.board = b;

	return n;
}

void printFinalBoard(vector<vector<int>> board)
{
	for (vector<int> v : board)
	{
		for (unsigned i = 0; i < v.size(); i++)
		{
			cout << v[i] << " ";
		}
		cout << endl;
	}
	cout << endl;
}


const int tag_work = 0;
const int tag_done = 1;
const int tag_finished = 2;

int main(int argc, char* argv[])
{
	int proc_num, num_procs; // ˇc´ıslo procesu, poˇcet proces˚u
	int max_value = -10000;
	vector<vector<int>> finalBoard;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_num);

	MPI_Status status;
	if (proc_num == 0)
	{
		int rows, cols, i1, i2, c1, c2, cn, k, x, y;
		vector<vector<int>> board;
		Tile tiles[5];

		cin >> rows >> cols;
		board = setBoard(rows, cols);
		cin >> i1 >> i2 >> c1 >> c2 >> cn;
		cin >> k;

		Algorithm a(cols, rows, i1, i2, c1, c2, cn);

		int i = 0;

		tiles[0].value = c2;
		tiles[0].length = i2;
		tiles[0].horizontal = 1;
		tiles[1].value = c2;
		tiles[1].length = i2;
		tiles[1].horizontal = 0;
		tiles[2].value = c1;
		tiles[2].length = i1;
		tiles[2].horizontal = 1;
		tiles[3].value = c1;
		tiles[3].length = i1;
		tiles[3].horizontal = 0;
		tiles[4].value = cn;
		tiles[4].length = 1;
		tiles[4].horizontal = -1;



		while (i++ < k)
		{
			cin >> x >> y;
			board[y][x] = -1;
		}

		queue<QueuedNode> q = a.fillQueue(board, tiles, 1);

		for (int i = 1; i < num_procs; i++)
		{
			QueuedNode n = q.front();
			q.pop();
			MPI_Send(createArrFromClass(n.node.board, n.next_id, tiles, rows, cols, n.node.value), 1024, MPI_INT, i, tag_work, MPI_COMM_WORLD);

			int working_slaves = num_procs - 1;
			while (working_slaves > 0)
			{
				int * sol = NULL;
				MPI_Recv(sol, 1024, MPI_INT, MPI_ANY_SOURCE, tag_done, MPI_COMM_WORLD, &status);
				Node solution = parseSolution(sol);
				if (solution.value > max_value)
				{
					max_value = solution.value;
					finalBoard = solution.board;
				}
				if (!q.empty())
				{
					n = q.front();
					q.pop();
					MPI_Send(createArrFromClass(n.node.board, n.next_id, tiles, rows, cols, n.node.value), 1024, MPI_INT, status.MPI_SOURCE, tag_work, MPI_COMM_WORLD);
				} else {
					MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, tag_finished, MPI_COMM_WORLD);
					working_slaves--;
				}
			}
		}

		cout << "Best value: " << max_value << endl;
		printFinalBoard(finalBoard);

	}
	else
	{
		while (true)
		{
			int * arr = NULL;
			MPI_Recv(arr, 1024, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if (status.MPI_TAG == tag_finished)
			{
				break;
			}
			else if (status.MPI_TAG == tag_work)
			{
					Box b = createBoardFromArr(arr);

					b.a.run(b.board, b.tiles, b.id);
					MPI_Send(createSolution(b.a.getBoard(), b.a.getMaxPrice(), b.a.getRows(), b.a.getCols()), 1024, MPI_INT, 0, tag_done, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();

	return 0;
}
