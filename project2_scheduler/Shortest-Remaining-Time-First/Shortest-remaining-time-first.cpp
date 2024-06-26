// Shortest-remaining-time-first.cpp: 定義主控台應用程式的進入點。
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

//used to store process data
struct aprocess {	
	int id = -1;
	int priority;
	int btime = 0;
	int atime = 0;
	int recordresponse = 1;
};

//used to store simulating result
struct schedulingresult {
	int wtime[maxsize];
	int trtime[maxsize];
	int finished[maxsize];
	int responsed[maxsize];
	double throughput = 0;
	int CS = 0;
	vector<string> csrecord;
};
typedef struct aprocess AP;
typedef struct schedulingresult SR;

//update the new coming process to the queue
void arrival(AP* storage, int ssize, int tu, vector<int> &Q);

//Simulating od Shortest-remaining-time-first
SR SRTF(AP* storage, int ssize, vector<AP> &Q);

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
	vector<AP> Q;
	SR result = SRTF(storage, stindex, Q);

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

//update the new coming process to the queue
void arrival(AP* storage, int ssize, int tu, vector<AP> &Q) {

	for (int i = 0; i < ssize; i++) {
		if (storage[i].atime == tu) {
			Q.push_back(storage[i]);
		}
	}
}


//Simualting of Shortest-remaing-time-first Scheduling
SR SRTF(AP* storage, int ssize, vector<AP> &Q) {
	//queue<int> Q;
	SR result;


	AP CPU;
	int tu = 0;
	int finishedtask = 0;
	int contextclock = 0;
	while (1) {

		cout << endl << "time is " << tu << endl;

		//update the new coming processes to the queue
		arrival(storage, ssize, tu, Q);

		//if the current is finished, check if the queue is empty and run a new shortest process
		if (CPU.btime == 0) {
			if (CPU.id >= 0) {
				result.trtime[CPU.id - 1] = tu - CPU.atime;
				result.wtime[CPU.id - 1] = tu - CPU.atime - storage[CPU.id-1].btime;
				result.finished[CPU.id - 1] = tu;
				finishedtask++;
				cout << endl << "finished task" << CPU.id;
				CPU.id = -1;
			}

			//if there' s some task in the queue, we pick up the shortest one and run it
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
				Q.erase(Q.begin() + index);
				if (CPU.recordresponse) {
					CPU.recordresponse = 0;
					result.responsed[CPU.id - 1] = tu - CPU.atime;
				}
				//result.CS++;
				//contextclock = contextcost;
			}
		}
		//if the queue isn't empty, we find a task with shorter burst time than the current one and replace it
		if (!Q.empty()) {
			//find the shortest
			int shortestbt = 10000;
			int index = -1;
			for (int i = 0; i < Q.size(); i++) {
				if (Q[i].btime < shortestbt) {
					shortestbt = Q[i].btime;
					index = i;
				}
			}

			//if the current one is longer, then replace it
			if (index != -1&&CPU.btime>Q[index].btime) {
				Q.push_back(CPU);
				string tempa = to_string(CPU.id);
				string tempc = to_string(CPU.btime);
				CPU = Q[index];
				string tempb = to_string(Q[index].id);
				cout << endl << "We get task" << CPU.id << "to run" << endl;
				Q.erase(Q.begin() + index);
				result.CS++; 
				contextclock = contextcost;
				result.csrecord.push_back(tempa + " switched by " + tempb + " at time " + to_string(tu) + " with " + tempc + " burst time");
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

		//update the time, burst time of current process, contextclock
		tu++;
		if (CPU.btime - 1 >= 0 && !contextclock)
			CPU.btime--;
		if (contextclock - 1 >= 0)
			contextclock--;
	}
	
	//print out the result for debugging
	cout << "Context time = " << result.CS;
	for (int i = 0; i < result.csrecord.size(); i++) {
		cout << result.csrecord[i] << endl;
	}
	return result;

}