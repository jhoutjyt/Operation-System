// Scheduling2.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <list>
#include <string>
const int maxsize = 10000;
const  int contextcost = 0;
using namespace std;
list<int> FQ;

//A Queue build by myself, but not making any use of it
class Queuelist {
public:
	struct queuenode {
		int id;
		struct queuenode* next = NULL;
	};
	typedef struct queuenode QNode;
	QNode* first = NULL;

	void push(int value) {
		if (first == NULL) {
			QNode* newnode = new QNode;
			newnode->id = value;
			first = newnode;
		}
		else {
			QNode* newnode = new QNode;
			newnode->id = value;
			QNode* current = first;
			while (current->next != NULL)
				current = current->next;
			current->next = newnode;
		}
	}

	int pop() {

		if (first == NULL)
			return -1;
		else {
			int a = first->id;
			QNode* current = first;
			if (first->next == NULL) {
				delete(current);
				first = NULL;
			}
			else if (first->next != NULL) {
				first = first->next;
				delete(current);
			}
			return a;
		}
	}

	int empty() {
		return first == NULL;
	}

	int front() {
		return first->id;
	}

	void walk(QNode* r) {
		if(r!=NULL)
			cout << r->id << endl;
		if (r->next != NULL)
			walk(r->next);
	}
};  

//used to store process data
struct aprocess {	
	int id = -1;
	int priority;
	int btime = 0;
	int atime = 0;
	int recordresponse = 1;
};

//used to store the result of simulating
struct schedulingresult {
	int wtime[maxsize];
	int trtime[maxsize];
	int finished[maxsize];
	int responsed[maxsize] = {0};
	int CS = 0;
	double throughput = 0;

};
typedef struct aprocess AP;
typedef struct schedulingresult SR;

//update the new coming process(put it into the queue) 
void arrival(AP* storage, int ssize, int tu, list<int> &Q);

//Simulating of First In First Out
SR FIFO(AP* storage, int ssize, list<int> &Q);

int main()
{
	//Read File
	ifstream inClientFile("data_3.txt", ios::in);
	if (!inClientFile)
	{
		cerr << "File could not be opened" << endl;
		exit(0);
	}

	AP storage[maxsize];
	int stindex = 0;
	string temp;
	size_t mark;
	while (inClientFile >> temp) {

		if ((mark = temp.find_first_of("P")) != string::npos) {

			AP A;
			A.id = stoi(temp.substr(mark + 1, temp.size()));

			inClientFile >> temp;
			A.priority = stoi(temp);
			inClientFile >> temp;
			A.btime = stoi(temp);
			inClientFile >> temp;
			A.atime = stoi(temp);
			storage[stindex++] = A;
		}

	}
	inClientFile.close();

	//start simulating
	SR result = FIFO(storage, stindex, FQ);
	
	//export the result into a .csv file
	ofstream outClientFile("result.csv", ios::out);
	if (!outClientFile)
	{
		cerr << "File could not be opened" << endl;
		exit(0);
	}
	outClientFile << "Context Switch 次數: ," << result.CS << ",Context Switch Cost: ," << contextcost;
	double wsum = 0;
	double tsum = 0;
	double rsum = 0;
	int maxw = 0;
	int maxt = 0;
	int maxr = 0;
	int endtime = 0;
	for (int i = 0; i < stindex; i++) {
		wsum += result.wtime[i];
		tsum += result.trtime[i];
		rsum += result.responsed[i];
		if (result.wtime[i] > maxw)
			maxw = result.wtime[i];
		if (result.trtime[i] > maxt)
			maxt = result.trtime[i];
		if (result.responsed[i] > maxr)
			maxr = result.responsed[i];
		if (result.finished[i] > endtime)
			endtime = result.finished[i];
	}
	outClientFile << ", Average Waiting Time: ," << wsum / stindex;
	outClientFile << ", Average TurnAround Time: ," << tsum / stindex;
	outClientFile << ", Average Response Time: ," << rsum / stindex;
	outClientFile << ", Longest Wait: ," << maxw;
	outClientFile << ", Longest Turn: ," << maxt;
	outClientFile << ", Longest Response: ," << maxr;
	outClientFile << ", end time ," << endtime;
	outClientFile << ", Throughput: ," << result.throughput << endl;
	outClientFile << endl << "id,turnaround time,waiting time,response time,priority,burst time,arrive time,finished time" << endl;
	for (int i = 0; i < stindex; i++) {
		outClientFile << i + 1 << "," << result.trtime[i] << "," << result.wtime[i] << "," << result.responsed[i];
		outClientFile << "," << storage[i].priority << "," << storage[i].btime << "," << storage[i].atime << "," << result.finished[i] << endl;
	}
	//process finished
	system("Pause");
    return 0;
}

//update the new coming process(put it into queue)
void arrival(AP* storage, int ssize, int tu, list<int> &Q) {

	for (int i = 0; i < ssize; i++) {
		if (storage[i].atime == tu) {
			//cout << "id = " << storage[i].id << endl;
			int a = storage[i].id;
			Q.push_back(a);
		}
	}
}

SR FIFO(AP* storage, int ssize, list<int> &Q) {
	
	SR result;

	
	AP CPU;
	int tu = 0;
	int finishedtask = 0;
	while (1) {
		
		cout <<endl<< "time is" <<tu << endl;
		//update the new coming process
		arrival(storage, ssize, tu, Q);
		
		//if the current process is finished, we check the queue
		if (CPU.btime == 0) {
			if (CPU.id >= 0) {
				result.trtime[CPU.id - 1] = tu - CPU.atime;
				result.wtime[CPU.id - 1] = tu - CPU.atime - storage[CPU.id-1].btime;
				result.finished[CPU.id - 1] = tu;
				finishedtask++;
				cout <<endl<< "finished task" << CPU.id;
				CPU.id = -1;
			}
			
			//if the queue isn't empty, then we run a new process
			if (!Q.empty()) {
				CPU.id = Q.front();
				CPU.btime = storage[Q.front() - 1].btime;
				CPU.atime = storage[Q.front() - 1].atime;
				CPU.priority = storage[Q.front() - 1].priority;
				cout <<endl<< "We get task" << CPU.id << "to run"<<endl;
				Q.pop_front();
				//result.CS++;
				//contextclock = contextcost;
			
					result.responsed[CPU.id - 1] = tu - CPU.atime;
			}
		}

		if (finishedtask == ssize) {
			result.throughput = double(ssize) / tu;
			break;
		}

		//update the time
		tu++;
		if (CPU.btime - 1 >= 0) 
			CPU.btime--;

	}

	return result;
}
