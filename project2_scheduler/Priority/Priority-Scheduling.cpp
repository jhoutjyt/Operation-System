// Priority-Scheduling.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;
const int maxsize = 10000;
const int contextcost = 0;	//time of doing Context Switch
//used for storing process data
struct aprocess {
	int id = -1;
	int priority;
	int btime = 0;
	int atime = 0;
	int recordresponse = 1;
};

//used to store the result 
struct schedulingresult {
	int wtime[maxsize];
	int trtime[maxsize];
	int finished[maxsize];
	int responsed[maxsize];
	int CS = 0;
	double throughput = 0;
	vector<string> csrecord;
};
typedef struct aprocess AP;
typedef struct schedulingresult SR;

void arrival(AP* storage, int ssize, int tu, vector<int> &Q);	//update the new coming process to queue
SR PS(AP* storage, int ssize, vector<AP> &Q);	//simulating the Preemptive Priority Scheduling

int main()
{
	//Read File
	ifstream inClientFile("data_1.txt", ios::in);
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

	//start Simulating
	vector<AP> Q;
	SR result = PS(storage, stindex, Q);

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

//update new coming process to queue
void arrival(AP* storage, int ssize, int tu, vector<AP> &Q) {

	for (int i = 0; i < ssize; i++) {
		if (storage[i].atime == tu) {
			Q.push_back(storage[i]);
		}
	}
}

//Simulating by Priority Scheduling
SR PS(AP* storage, int ssize, vector<AP> &Q) {
	//queue<int> Q;
	SR result;


	AP CPU;
	int tu = 0;
	int finishedtask = 0;
	//int contextcost = 2;
	int contextclock = 0;
	while (1) {

		cout << endl << "time is " << tu << endl;
		arrival(storage, ssize, tu, Q);	//update process to queue

		//if current process is finished
		if (CPU.btime == 0) {
			if (CPU.id >= 0) {
				result.trtime[CPU.id - 1] = tu - CPU.atime;
				result.wtime[CPU.id - 1] = tu - CPU.atime - storage[CPU.id -1 ].btime;
				result.finished[CPU.id - 1] = tu;
				finishedtask++;
				cout << endl << "finished task" << CPU.id;
				CPU.id = -1;
			}
			//if queue isn't empty, get a new process to run
			if (!Q.empty()) {
				int priority = 0;
				int index = -1;
				for (int i = 0; i < Q.size(); i++) {
					if (Q[i].priority > priority) {
						priority = Q[i].priority;
						index = i;
					}
				}

				CPU = Q[index];
				cout << endl << "We get task" << CPU.id << "to run" << endl;
				Q.erase(Q.begin() + index);
				//result.CS++;
				//contextclock = contextcost;
				contextclock = 0;
				if (CPU.recordresponse) {
					CPU.recordresponse = 0;
					result.responsed[CPU.id - 1] = tu - CPU.atime;
				}
			}
		}

		//if there's a process with higher priority in the queue, we do context switch
		if (!Q.empty()) {
			int priority = 10000;
			int index = -1;
			for (int i = 0; i < Q.size(); i++) {
				if (Q[i].priority < priority) {
					priority = Q[i].priority;
					index = i;
				}
			}

			if (index != -1 && CPU.priority > Q[index].priority) {
				Q.push_back(CPU);
				string tempa = to_string(CPU.id);
				string tempc = to_string(CPU.priority);
				CPU = Q[index];
				string tempb = to_string(Q[index].id);
				cout << endl << "We get task" << CPU.id << "to run" << endl;
				Q.erase(Q.begin() + index);
				result.CS++;
				contextclock = contextcost;
				result.csrecord.push_back(tempa + +" with " + tempc + " priority"+" switched by " + tempb + " at time " + to_string(tu) );
				if (CPU.recordresponse) {
					CPU.recordresponse = 0;
					result.responsed[CPU.id - 1] = tu - CPU.atime;
				}
			}
		}
		
		if (finishedtask == ssize) {
			result.throughput = double(ssize) / tu;
			break;
		}
		//update time, burst time, contextclock
		tu++;
		if (CPU.btime - 1 >= 0 && !contextclock) 
			CPU.btime--;
		if (contextclock - 1 >= 0)
			contextclock--;
	}
	
	//print the context switch result
	cout << "Context time = " << result.CS <<endl;
	for (int i = 0; i < result.csrecord.size(); i++) {
		cout << result.csrecord[i] << endl;
	}
	return result;

}