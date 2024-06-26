// SJN.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;
const int maxsize = 10000;
const int contextcost = 0;
struct aprocess {	//used to store process data
	int id = -1;
	int priority;
	int btime = 0;
	int atime = 0;
	int recordresponse = 1;
};

struct schedulingresult {	//used to store the result of simulating
	int wtime[maxsize];
	int trtime[maxsize];
	int finished[maxsize];
	int responsed[maxsize];
	double throughput = 0;
	int CS = 0;

};
typedef struct aprocess AP;
typedef struct schedulingresult SR;

//used to update the new coming process to the queue
void arrival(AP* storage, int ssize, int tu, vector<int> &Q);

//used for SJN Simulation 
SR SJN(AP* storage, int ssize, vector<AP> &Q);

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

	//Start Simulating
	vector<AP> FQ;
	SR result = SJN(storage, stindex, FQ);


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
	system("pause");
	return 0;

}

void arrival(AP* storage, int ssize, int tu, vector<AP> &Q) {

	for (int i = 0; i < ssize; i++) {
		if (storage[i].atime == tu) {
			Q.push_back(storage[i]);
		}
	}
}

SR SJN(AP* storage, int ssize, vector<AP> &Q) {
	//queue<int> Q;
	SR result;


	AP CPU;
	int tu = 0;
	int finishedtask = 0;
	//int contextcost = 0;
	//int contextclock = 0;
	while (1) {

		cout << endl << "time is " << tu << endl;
		
		//update the new coming process to the queue
		arrival(storage, ssize, tu, Q);

		//if the current process is finished, check the queue
		if (CPU.btime == 0) {
			if (CPU.id >= 0) {
				result.trtime[CPU.id - 1] = tu - CPU.atime;
				result.wtime[CPU.id - 1] = tu - CPU.atime - storage[CPU.id-1].btime;
				result.finished[CPU.id - 1] = tu;
				finishedtask++;
				cout << endl << "finished task" << CPU.id;
				CPU.id = -1;
			}

			//if the queue isn't empty,  we find the process with highest priority(smallest number) and run it
			if (!Q.empty()) {
				int shortestbt = 10000;
				int index = -1;
				for (int i = 0; i < Q.size(); i++) {
					if (Q[i].btime < shortestbt) {
						shortestbt = Q[i].btime;
						index = i;
					}
				}

				CPU = Q[index];
				cout << endl << "We get task" << CPU.id << "to run" << endl;
				Q.erase(Q.begin() + index);	//erase the process we get from the queue
				if (CPU.recordresponse) {
					CPU.recordresponse = 0;
					result.responsed[CPU.id - 1] = tu - CPU.atime;
				}
				//result.CS++;
				//contextclock = contextcost;
			}
		}

		if (finishedtask == ssize) {
			result.throughput = double(ssize) / tu;
			break;
		}
		//update time
		tu++;
		if (CPU.btime - 1 >= 0)
			CPU.btime--;
	}
	
	return result;

}