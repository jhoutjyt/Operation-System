// Round-Robin.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;
const int maxsize = 10000;
const int contextcost = 0;	//Context Switch Spend Time
struct aprocess {	//Store the process data
	int id = -1;
	int priority;
	int btime = 0;
	int atime = 0;
	int recordresponse = 1;
};

struct schedulingresult {	//Store the Result
	int wtime[maxsize];
	int trtime[maxsize];
	int finished[maxsize];
	int CS = 0;
	double throughput = 0;
	int responsed[maxsize];
	vector<string> csrecord;	//Record what happened
};
typedef struct aprocess AP;
typedef struct schedulingresult SR;

void arrival(AP* storage, int ssize, int tu, vector<int> &Q);
SR RR(AP* storage, int ssize, vector<AP> &Q);

int main()
{
	//Read File
	ifstream inClientFile("data_2.txt", ios::in);
	if (!inClientFile)
	{
		cerr << "File could not be opened" << endl;
		exit(0);
	}

	//store the data into storage array for later use
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
	SR result = RR(storage, stindex, Q);

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
		outClientFile << i + 1 << "," << result.trtime[i] << "," << result.wtime[i]<< "," << result.responsed[i];
		outClientFile << "," << storage[i].priority << "," << storage[i].btime << "," << storage[i].atime << "," << result.finished[i] << endl;
	}

	//finished process
	system("pause");
	return 0;

}


//get the new coming process into Queue by checking time(tu)
void arrival(AP* storage, int ssize, int tu, vector<AP> &Q) {

	for (int i = 0; i < ssize; i++) {
		if (storage[i].atime == tu) {
			Q.push_back(storage[i]);
		}
	}
}

//Round Robin Simulation
SR RR(AP* storage, int ssize, vector<AP> &Q) {
	//queue<int> Q;
	SR result;

	AP CPU;	//A temporary process data structure for caculating
	int tu = 0;	//time variable
	int finishedtask = 0;	//record how many task is finished
	int timequantum = 15;	//time quantum
	int counter = -10000;	//counter to check if we preempt the current process
	//int contextcost = 0;
	int contextclock = 0;	//used to store the time flow of context switch time
	while (1) {

		cout << endl << "time is " << tu << endl;
		
		arrival(storage, ssize, tu, Q); // update new coming process

		//if timequantum has arrived and the queue isn't empty, switch it
		if (!Q.empty() && counter >= timequantum && CPU.btime != 0) {

			counter = 0 - contextcost;
			Q.push_back(CPU);
			string tempa = to_string(CPU.id);
			CPU = Q.front();
			string tempb = to_string(Q.front().id);
			cout << endl << "We get task" << CPU.id << "to run" << endl;
			Q.erase(Q.begin());
			result.CS++;
			result.csrecord.push_back(tempa + " switched by " + tempb + " at time " + to_string(tu));
			if (CPU.recordresponse) {
				CPU.recordresponse = 0;
				result.responsed[CPU.id - 1] = tu - CPU.atime;
			}
		}
		
		
		//check if the current process finished
		if (CPU.btime == 0) {
			if (CPU.id >= 0) {
				result.trtime[CPU.id - 1] = tu - CPU.atime;
				result.wtime[CPU.id - 1] = tu - CPU.atime - storage[CPU.id-1].btime;
				result.finished[CPU.id - 1] = tu;
				finishedtask++;
				cout << endl << "finished task" << CPU.id;
				CPU.id = -1;
			}
			//if queue isn't empty, replace a new process to CPU
			if (!Q.empty()) {
				//counter = 0 - contextcost;
				counter = 0;
				//result.CS++;
				CPU = Q.front();
				cout << endl << "We get task" << CPU.id << "to run" << endl;
				Q.erase(Q.begin());
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

		//update time, burst time, counter
		tu++;
		if (CPU.btime - 1 >= 0 && counter >= 0) {
			CPU.btime--;
		}
		counter++;
	}

	cout << "Context time = " << result.CS << endl;
	for (int i = 0; i < result.csrecord.size(); i++) {
		cout << result.csrecord[i] << endl;
	}
	return result;
}