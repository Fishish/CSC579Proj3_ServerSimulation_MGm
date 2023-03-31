#include <iostream>
#include <math.h>
#include <list>
#include <chrono>

using std::cout;
using std::endl;
using std::cin;
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876 /* unlikely value*/
#define transient_period 1000 // observations to ignore

// Object to represent each single client for analysis.
class Client{
    public:
        long id; // the id of client
        float dTime; // the depart time of client
        float aTime; // the arrival time of client
        float serviceTime; // the service time of the client
        float waitingTime; // the waiting time of the client.
        int queueNum;
        float originalServiceTime;
        float totalWait;
        float totalService;
        float cpuService = 0.0;
        float cpuWait = 0.0;
        float io1Service = 0.0;
        float io1Wait = 0.0;
        float io2Service = 0.0;
        float io2Wait = 0.0;
        float io3Service = 0.0;
        float io3Wait = 0.0;
        int CPUtimes = 0;
        int IO1times = 0;
        int IO2times = 0;
        int IO3times = 0;

        Client(){

        }
        Client(long id, float aTime, float dTime){ // construct the client object
            this->id = id;
            this->dTime = dTime;
            this->aTime = aTime;
            // this->cpuService = 0.0;
            // this->cpuWait = 0.0;
            // this->io1Service = 0.0;
            // this->io1Wait = 0.0;
            // this->io2Service = 0.0;
            // this->io2Wait = 0.0;
            // this->io3Service = 0.0;
            // this->io3Wait = 0.0;
            // this->CPUtimes = 0;
            // this->IO1times = 0;
            // this->IO2times = 0;
            // this->IO3times = 0;
        }
        Client(long id, float aTime){
            this->id = id;
            this->aTime = aTime;
            // this->cpuService = 0.0;
            // this->cpuWait = 0.0;
            // this->io1Service = 0.0;
            // this->io1Wait = 0.0;
            // this->io2Service = 0.0;
            // this->io2Wait = 0.0;
            // this->io3Service = 0.0;
            // this->io3Wait = 0.0;
            // this->CPUtimes = 0;
            // this->IO1times = 0;
            // this->IO2times = 0;
            // this->IO3times = 0;
        }

        void setServiceTime(float sTime){
            this->serviceTime = sTime;
        }

        void setOriginalServiceTime(float osTime){
            this->originalServiceTime = osTime;
        }

        void setQueueNumber(int qb){
            this->queueNum = qb;
        }
        void setWaitTime(float wTime){
            this->waitingTime = wTime;
        }
        void empty(){
            this->id = -1;
        }
        void setDtime(float dtime){
            this->dTime = dtime;
        }
        void clear(){
            this->dTime = 0.0;
            this->aTime = 0.0;
            this->waitingTime = 0.0;
            this->serviceTime = 0.0;
        }
        void setSWTime(float serviceTime, float waitingTime){ // set the service and waiting time
            this->serviceTime = serviceTime;
            this->waitingTime = waitingTime;
        }

    // comparator to make smaller ids to the front of the list.
    bool operator <(const Client & clientObj) const
    {
        return serviceTime < clientObj.serviceTime;
    }
};

// class for the event object
class Event{
    public:
        long id; // the id of event
        float time; // the time of the even
        char type; // the type of event A for arrival and D for depart
        float serviceTime;
        Client eventClient;
        Event(long id, float time, char type){ // construct the event object
            this->id = id;
            this->time = time;
            this->type = type;
            this->serviceTime = -1.0;
        }
        Event(){
        }

        void setServiceTime(float st){
            this->serviceTime = st;
        }

    // comparator for sorting, small time goes to front of the list.
    bool operator <(const Event & eventObj) const
    {
        return time < eventObj.time;
    }
};

//Given method from class.
float ran0(long *idum){
    long k;
    float ans;

    *idum ^= MASK;
    k = (*idum)/IQ;
    *idum = IA*(*idum-k*IQ)-IR*k;
    if (*idum < 0) *idum+=IM;
    ans = AM*(*idum);

    *idum ^= MASK;
    return ans;
}
// Given method from class
float expdev(long *idum, double lamb0){
    float ran0(long *idum);
    float dummy;

    do
        dummy = ran0(idum);
    while (dummy == 0.0);
    // cout << " the dummy " << -1/(lamb/u) << endl;
    return -log(dummy) * (1/lamb0);
    
}



float getVariance(std::list<float> meanList, float sampleMean, float runs){
    float s = 0;
    for ( int i = 0; i < meanList.size(); i++){
        s += pow((meanList.front() - sampleMean), 2);
        meanList.pop_front();
    }
    s = ( 1/(runs-1) ) * s;
    return s;
}

// 0 for -
// 1 for +
float get_Confidence_Interval(float sampleMean, float sampleVariance, float runs, int sign){
    float kScore = 1.96;
    float ans;
    if (sign == 0){  // 1
        ans =  sampleMean - ( (sqrt(sampleVariance) / sqrt(runs)) * kScore );
    }else{
        ans = sampleMean + ( (sqrt(sampleVariance) / sqrt(runs)) * kScore );
    };   
    return ans;
}

int push_to_queue_by_rp(Client c, int rp, std::list<Client> *q1, std::list<Client> *q2, std::list<Client> *q3, std::list<Client> *q4){
    if (rp < 25){
        c.setQueueNumber(1);
        q1->push_back(c);
        return 1;
    }else if(rp < 50){
        c.setQueueNumber(2);
        q2->push_back(c);
        return 2;
    }else if(rp < 75){
        c.setQueueNumber(3);
        q3->push_back(c);
        return 3;
    }else{
        c.setQueueNumber(4);
        q4->push_back(c);
        return 4;
    }
    return 0;
}

/*
    Main function to run the program
    it receives 3 inputs and then display the resutls.
*/ 
int main(){
    auto start = std::chrono::high_resolution_clock::now();
    Client clientInServer;  // the current client in the server, representing the server.
    // int key = 0; // variable to 
    long idum = 4492; // The seed of random generator.
    // long k = 1000; // The k value, how many clients before terminates
    // float lambda = 0.85; // THe lambda value.
    // int mDiscipline = 0; // 1 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P. 0 for debug
    long k = 0; // The k value, how many clients before terminates
    float lambda = -1.0; // THe lambda value.
    int mDiscipline = 0; // 1 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P. 0 for debug
    int customerL = -1;
    // std::list<float> serviceMeanList; // A list to represent the queue.
    std::list<float> waitingMeanList; // A list to represent the queue.
    std::list<float> systemMeanList; // A list to represent the queue.
    float outPutMasterClock = 0.0;
    float totalAverageServiceTime = 0; // average time of service
    float totalAverageWaitingTime = 0; // average time of waitting
    float totalAverageWaitingTime1 = 0; // average time of waitting
    float totalAverageWaitingTime2 = 0; // average time of waitting
    float totalAverageWaitingTime3 = 0; // average time of waitting
    float totalAverageWaitingTime4 = 0; // average time of waitting

    float CPUtotalAverageWaitingTime = 0; // average time of waitting
    float IO1totalAverageWaitingTime = 0; // average time of waitting
    float IO2totalAverageWaitingTime = 0; // average time of waitting
    float IO3totalAverageWaitingTime = 0; // average time of waitting

    float totalSystemTime = 0;
    float sampleSystemMean;
    float sampleWaitMean;
    float sampleWaitMean1;
    float sampleWaitMean2;
    float sampleWaitMean3;
    float sampleWaitMean4;
    
    float sampleWaitMeanCPU;
    float sampleWaitMeanIO1;
    float sampleWaitMeanIO2;
    float sampleWaitMeanIO3;

    float sampleSystemVariance;
    float sampleSWaitVariance;
    

    int kRuns = 30;

    //////////////////////////// Get input //////////////////////////////
    // Get the inputs from user
    cout << "Please input the parameter lambda(0 < lambda < 1.0)." << endl;  // Lambda
    cin >> lambda;
    while(cin.fail() || lambda >= 1.0 || lambda <= 0.0) {
        std::cout << "0 < lambda < 1.0" << std::endl;
        std::cin.clear();
        std::cin.ignore(256,'\n');
        std::cin >> lambda;
    }    

    cout << "Please input the number K(Positive Intger Greater than 2)." << endl;  // K
    cin >> k;
    while(cin.fail() || k <= 2) {
        std::cout << "K must be a positive integer Greater than 2" << std::endl;
        std::cin.clear();
        std::cin.ignore(256,'\n');
        std::cin >> k;
    }    

    cout << "The integer L such that: 0 – M/M/1 system, 1 – CPU with I/O disks. " << endl;  // L
    cin >> customerL;
    while(cin.fail() || customerL < 0 || customerL > 1) {
        std::cout << "0 <= l <= 1" << std::endl;
        std::cin.clear();
        std::cin.ignore(256,'\n');
        std::cin >> customerL;
    }  

    if (customerL == 0) {
        cout << "Integer M in the range 1 < M < 5, denoting the service discipline, as follows: 1 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P. " << endl;  // L
        cin >> mDiscipline;
        while(cin.fail() || mDiscipline < 1 || mDiscipline > 5) {
            std::cout << "1 <= m <= 5" << std::endl;
            std::cin.clear();
            std::cin.ignore(256,'\n');
            std::cin >> mDiscipline;
        }  
    }
    //////////////////////////// Get input //////////////////////////////



  



    if (mDiscipline == 1){  // FCFS
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Event> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        clientQueue.push_back(eventList.front()); // add first event to queue
                        eventList.pop_back(); // pop the first event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, expdev(&idum, 1.0)+masterClock, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            clientQueue.push_back(temp); // add to the queue

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // create a client object with the id, atime, dtime.
                            Client tempClient(temp.id, clientQueue.front().time, temp.time);
                            dataList.push_back(tempClient);
                            clientQueue.pop_front(); // remove it from queue
                            if(count == k + transient_period){ // stop if k cutomer is served
                                break;
                            }
                            if (clientQueue.size() != 0){ // when queue is not empty
                                eventList.push_back(Event(clientQueue.front().id, expdev(&idum, 1.0)+masterClock, 'D'));
                                eventList.sort(); // sort the list
                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.push_back(Event(eventList.front().id, expdev(&idum, 1.0)+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                            }
                            // add next dep event to the event list.


                            count++;


                        }

                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients

                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                    }
                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                outPutMasterClock += masterClock;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;

            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    }else if ( mDiscipline == 2){ // LCFS-NP
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Event> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        clientQueue.push_back(eventList.front()); // add first event to queue
                        Client firstTemp(eventList.front().id, eventList.front().time);
                        clientInServer = firstTemp;     // push to server
                        eventList.pop_back(); // pop the first event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, expdev(&idum, 1.0)+masterClock, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // create the client based on the event
                            Client tempC(temp.id, temp.time);
                            // check is the server empty
                            if (clientInServer.id == -1){   // if is empty, update server
                                clientInServer = tempC;
                            }else{  // if not empty, push to the queue
                                clientQueue.push_back(temp); // add to the back of queue since last first
                            }
                            // put client into the list with id and atime;
                            aK++;
                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.

                            // create a client object with the id, atime, dtime. from server
                            clientInServer.setDtime(masterClock);
                            dataList.push_back(clientInServer);
                            // clear the server
                            clientInServer.empty(); // set the id to -1

                            if(count == k + transient_period){ // stop if k cutomer is served
                                break;
                            }

                            // check is queue empty
                            if (clientQueue.size() != 0){ // when queue is not empty
                                // push the next client into server, since last first, push the back
                                Client tempC(clientQueue.back().id, clientQueue.back().time);  // set the id and arrival time
                                clientInServer = tempC; // push to the server
                                eventList.push_back(Event(clientQueue.back().id, expdev(&idum, 1.0)+masterClock, 'D'));
                                clientQueue.pop_back();  // remove the one from queue in server.
                                eventList.sort(); // sort the list
                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.push_back(Event(eventList.front().id, expdev(&idum, 1.0)+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                            }
                            // add next dep event to the event list.


                            count++;


                        }

                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);

                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients

                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                    }
                    

                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                outPutMasterClock += masterClock;
                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;
            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    


    }else if (mDiscipline == 3){  // SJF-NP
    // roll the arrive event and add to the queue
    // roll the service time and sort the queue based on that
    // based on the servive time and queue generate the next depart event.
    // depart and dequeue.


        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Client> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        clientInServer = tempC; // add first client to server
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            // Create the new client based on the arrival
                            Client tempC(temp.id, temp.time); // ID and arrival time
                            // check the server time in case of empty queue
                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.

                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                            }
                            // finished client created

                            // check is the server empty
                            if (clientInServer.id == -1){   // if is empty, update server
                                clientInServer = tempC;
                            }else{  // if not empty, push to the queue
                                clientQueue.push_back(tempC); // add to the back of queue since last first
                            }

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.

                            // create a client object with the id, atime, dtime. from server
                            clientInServer.setDtime(masterClock);
                            dataList.push_back(clientInServer);
                            // clear the server
                            clientInServer.empty(); // set the id to -1
                            // sort the queue for shortest service time
                            clientQueue.sort();
 
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }

                            // check is queue empty
                            if (clientQueue.size() != 0){ // when queue is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue.front();   // push to server
                                // remove it from queue
                                clientQueue.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;


                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.front().setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                                // cout << eventList.front().serviceTime+eventList.front().time << endl;

                            }
                            // add next dep event to the event list.


                            count++;


                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients
                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;
                        // cout << it->waitingTime << endl;

                    }
                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                    // cout << it->waitingTime << endl;
                    

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                outPutMasterClock += masterClock;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;

            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    }else if (mDiscipline == 4){    // 4 – Prio-NP


        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

                std::list<Client> clientQueue1; // A list to represent the queue.
                std::list<Client> clientQueue2; // A list to represent the queue.
                std::list<Client> clientQueue3; // A list to represent the queue.
                std::list<Client> clientQueue4; // A list to represent the queue.
                float averageServiceTime1 = 0; // average time of service
                float averageWaitingTime1 = 0; // average time of waitting
                float averageSystemTime1 = 0;
                float averageServiceTime2 = 0; // average time of service
                float averageWaitingTime2 = 0; // average time of waitting
                float averageSystemTime2 = 0;
                float averageServiceTime3 = 0; // average time of service
                float averageWaitingTime3 = 0; // average time of waitting
                float averageSystemTime3 = 0;
                float averageServiceTime4 = 0; // average time of service
                float averageWaitingTime4 = 0; // average time of waitting
                float averageSystemTime4 = 0;
                int count1 = 0;
                int count2 = 0;
                int count3 = 0;
                int count4 = 0;
                int rp = 0;

                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        // set the queuenumber
                        tempC.setQueueNumber(1);  // since it is first give it doesnt matter.
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        clientInServer = tempC; // add first client to server
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;
                            // I get a new client, roll a p for which queue to go
                            // if all queues are empty, i push it to the server
                            // if not, push the priority one and push the client to the queue it belongs.
                            // so I should push it to the belonging queue first, and then gothorugh all queues to pick one.

                            aK++;
                            // Create the new client based on the arrival
                            Client tempC(temp.id, temp.time); // ID and arrival time              
                            // check the server time in case of empty queue
                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.

                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                            }
                            // roll the p for which queue to go
                            rp = int(expdev(&idum, 1.0) * 100000) % 100;
                            // set the queuenumber
                            if (rp < 25){
                                tempC.setQueueNumber(1);
                            }else if(rp < 50){
                                tempC.setQueueNumber(2);
                            }else if(rp < 75){
                                tempC.setQueueNumber(3);
                            }else{
                                tempC.setQueueNumber(4);
                            }

                            // finished client created



                            // check is the server empty
                            if (clientInServer.id == -1){   // if server is empty, update server
                                // if server is empty, I need to check all queues is it empty, and pick the priority one
                                if (clientQueue1.size() == 0 && clientQueue2.size() == 0 && clientQueue3.size() == 0 && clientQueue4.size() == 0){
                                    // if all queues are empty
                                    // push to the server
                                    clientInServer = tempC;
                                }
                            }else{  // if not empty, push to the queue
                                if (tempC.queueNum == 1){
                                    clientQueue1.push_back(tempC);
                                }else if(tempC.queueNum == 2){
                                    clientQueue2.push_back(tempC);
                                }else if(tempC.queueNum == 3){
                                    clientQueue3.push_back(tempC);
                                }else{
                                    clientQueue4.push_back(tempC);
                                }
                            }

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.

                            // create a client object with the id, atime, dtime. from server
                            // set the dpart time of client in server
                            clientInServer.setDtime(masterClock);
                            dataList.push_back(clientInServer);
                            // clear the server
                            clientInServer.empty(); // set the id to -1
   
 
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }

                            // check is queue empty
                            if (clientQueue1.size() != 0){ // when queue1 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue1.front();   // push to server
                                // remove it from queue
                                clientQueue1.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue2.size() != 0){ // when queue2 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue2.front();   // push to server
                                // remove it from queue
                                clientQueue2.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue3.size() != 0){ // when queue3 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue3.front();   // push to server
                                // remove it from queue
                                clientQueue3.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue4.size() != 0){ // when queue4 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue4.front();   // push to server
                                // remove it from queue
                                clientQueue4.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else{  // when all queues are empty, push the depart event based on the current arrival
                                eventList.front().setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                                // cout << eventList.front().serviceTime+eventList.front().time << endl;
                            }
                            // add next dep event to the event list.


                            count++;


                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients
                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;
                        // cout << it->waitingTime << endl;

                    }

                    if(it->queueNum == 1){
                        averageServiceTime1 += it->serviceTime;  // add the single service time to get the total time
                        averageWaitingTime1 += it->waitingTime;  // add the single waiting time to get the total time
                        count1 += 1;
                    }else if(it->queueNum ==2){
                        averageServiceTime2 += it->serviceTime;  // add the single service time to get the total time
                        averageWaitingTime2 += it->waitingTime;  // add the single waiting time to get the total time  
                        count2 += 1;                      
                    }else if(it->queueNum ==3){
                        averageServiceTime3 += it->serviceTime;  // add the single service time to get the total time
                        averageWaitingTime3 += it->waitingTime;  // add the single waiting time to get the total time     
                        count3 += 1;                   
                    }else{
                        averageServiceTime4 += it->serviceTime;  // add the single service time to get the total time
                        averageWaitingTime4 += it->waitingTime;  // add the single waiting time to get the total time   
                        count4 += 1;                     
                    }
                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                    // cout << it->waitingTime << endl;
                    

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                averageServiceTime1 = averageServiceTime1/count1;
                averageServiceTime2 = averageServiceTime2/count2;
                averageServiceTime3 = averageServiceTime3/count3;
                averageServiceTime4 = averageServiceTime4/count4;

                averageWaitingTime1 = averageWaitingTime1/count1;
                averageWaitingTime2 = averageWaitingTime2/count2;
                averageWaitingTime3 = averageWaitingTime3/count3;
                averageWaitingTime4 = averageWaitingTime4/count4;

                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                totalAverageWaitingTime1 += averageWaitingTime1;
                totalAverageWaitingTime2 += averageWaitingTime2;
                totalAverageWaitingTime3 += averageWaitingTime3;
                totalAverageWaitingTime4 += averageWaitingTime4;

                outPutMasterClock += masterClock;
                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleWaitMean1 = totalAverageWaitingTime1 / float(kRuns);
            sampleWaitMean2 = totalAverageWaitingTime2 / float(kRuns);
            sampleWaitMean3 = totalAverageWaitingTime3 / float(kRuns);
            sampleWaitMean4 = totalAverageWaitingTime4 / float(kRuns);

            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;

            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
            // cout << "sample wait1 mean " << sampleWaitMean1 << endl;
            // cout << "sample wait2 mean " << sampleWaitMean2 << endl;
            // cout << "sample wait3 mean " << sampleWaitMean3 << endl;
            // cout << "sample wait4 mean " << sampleWaitMean4 << endl;

    }else if (mDiscipline == 5){    // 5 – Prio-P


        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

                std::list<Client> clientQueue1; // A list to represent the queue.
                std::list<Client> clientQueue2; // A list to represent the queue.
                std::list<Client> clientQueue3; // A list to represent the queue.
                std::list<Client> clientQueue4; // A list to represent the queue.
                float averageServiceTime1 = 0; // average time of service
                float averageWaitingTime1 = 0; // average time of waitting
                float averageSystemTime1 = 0;
                float averageServiceTime2 = 0; // average time of service
                float averageWaitingTime2 = 0; // average time of waitting
                float averageSystemTime2 = 0;
                float averageServiceTime3 = 0; // average time of service
                float averageWaitingTime3 = 0; // average time of waitting
                float averageSystemTime3 = 0;
                float averageServiceTime4 = 0; // average time of service
                float averageWaitingTime4 = 0; // average time of waitting
                float averageSystemTime4 = 0;
                int count1 = 0;
                int count2 = 0;
                int count3 = 0;
                int count4 = 0;
                int rp = 0;

                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        // set the queuenumber
                        tempC.setQueueNumber(1);  // since it is first give it doesnt matter.
                        clientInServer = tempC; // add first client to server
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;
                            // I get a new client, roll a p for which queue to go
                            // if all queues are empty, i push it to the server
                            // if not, push the priority one and push the client to the queue it belongs.
                            // so I should push it to the belonging queue first, and then gothorugh all queues to pick one.


                            aK++;
                            // Create the new client based on the arrival
                            Client tempC(temp.id, temp.time); // ID and arrival time              
                            // check the server time in case of empty queue 
                            // to preset the depart event.
                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.
                                tempC.setOriginalServiceTime(tempC.serviceTime);

                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                                tempC.setOriginalServiceTime(tempC.serviceTime);
                            }
                            // roll the p for which queue to go
                            rp = int(expdev(&idum, 1.0) * 100000) % 100;
                            // set the queuenumber
                            if (rp < 25){
                                tempC.setQueueNumber(1);
                            }else if(rp < 50){
                                tempC.setQueueNumber(2);
                            }else if(rp < 75){
                                tempC.setQueueNumber(3);
                            }else{
                                tempC.setQueueNumber(4);
                            }
                            // finished client created


                            
                            // current server p is 2, new arrival client has p = 1
                            // so I should update the service time, and put the current server client back to the front of 
                            // the specific queue, and push the p1 client to the server 
                            // also need to update the depart, update the depart event time by (clock + p1 service time)
                            
                            // first check is the server empty, if empty, do as usual
                            // if occupied, check the p number of server client and new arrival client
                            // if new is lower priority(bigger number), push to queue



                            // check is the server empty
                            if (clientInServer.id == -1){   // if server is empty, update server
                            // the only situation that the server is empty is when
                            // after last depart, all queues are empty, and new arrival client is the only client right now
                                // push to the server
                                clientInServer = tempC;
                                clientInServer.setDtime(masterClock + clientInServer.serviceTime);
                            }else{  // if not empty, push to the queue
                            // 2 case, 1 new c has larger p, low priority, just push to queue
                            //         2 new c has smaller p, high priority, replace

                            // if the server is not empty, i need to compare the server client with new arrival client
                                if (tempC.queueNum < clientInServer.queueNum){
                                    // new arrival has higher priority/smaller number
                                    // update the service time of Server client
                                    // since every server client already has the depart and arrival time
                                    // service time = original.dtime - clock
                                    clientInServer.setServiceTime(clientInServer.dTime - masterClock);
                                    // put the old service client back to where it belongs
                                    // starts from 2 because 1 is the smallest
                                    if(clientInServer.queueNum == 2){
                                        // push it back to front since fsf
                                        clientQueue2.push_front(clientInServer);
                                    }else if(clientInServer.queueNum == 3){
                                        clientQueue3.push_front(clientInServer);
                                    }else if(clientInServer.queueNum == 4){
                                        clientQueue4.push_front(clientInServer);
                                    }

                                    // Update the client in server as the new arrival client.
                                    clientInServer = tempC;
                                    // update depart time
                                    clientInServer.setDtime(masterClock + clientInServer.serviceTime);
                                    // Update the depart time of current depart event.
                                    // since only dpart left, so event.front
                                    // new depart time = current clock + new arrival service time.
                                    eventList.front().time = clientInServer.dTime;
                                }else{  // if new arrival has lower priority/ larger p
                                        // no changes to the server client
                                        // push new client to the queue

                                    if (tempC.queueNum == 1){
                                        clientQueue1.push_back(tempC);
                                    }else if(tempC.queueNum == 2){
                                        clientQueue2.push_back(tempC);
                                    }else if(tempC.queueNum == 3){
                                        clientQueue3.push_back(tempC);
                                    }else{
                                        clientQueue4.push_back(tempC);
                                    }
                                }  // p checking


                            }

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.

                            // create a client object with the id, atime, dtime. from server
                            // set the dpart time of client in server
                            clientInServer.setDtime(masterClock);
                            dataList.push_back(clientInServer);
                            // clear the server
                            clientInServer.empty(); // set the id to -1
   
 
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }

                            // check is queue empty
                            if (clientQueue1.size() != 0){ // when queue1 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue1.front();   // push to server
                                // remove it from queue
                                clientQueue1.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue2.size() != 0){ // when queue2 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue2.front();   // push to server
                                // remove it from queue
                                clientQueue2.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue3.size() != 0){ // when queue3 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue3.front();   // push to server
                                // remove it from queue
                                clientQueue3.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else if(clientQueue4.size() != 0){ // when queue4 is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = clientQueue4.front();   // push to server
                                // remove it from queue
                                clientQueue4.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;

                            }else{  // when all queues are empty, push the depart event based on the current arrival
                                eventList.front().setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                                // cout << eventList.front().serviceTime+eventList.front().time << endl;
                            }
                            // add next dep event to the event list.


                            count++;


                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    // std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // // normal clients
                    // if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time
                    //     // cout << itPrevious->dTime << " and " << it ->aTime << endl;

                    //     it->setSWTime((it->dTime - it->aTime), 0);
                    // }else{ // previous one Departs after current arrive
                    //     // wait time = previous d time - current arrive time.
                    //     it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                    //     // cout << itPrevious->dTime << " and " << it ->aTime << endl;
                    //     // cout << it->waitingTime << endl;

                    // }
                    it->setWaitTime(it->dTime - it->aTime - it->originalServiceTime);
                    // I cannot just minus service time to get the waiting time since some client will have fewer service time in the end
                    // becasue of the preempt(keep getting updated)
                    // I can use a new varialbe to store the original service time.

                    if(it->queueNum == 1){
                        averageServiceTime1 += it->originalServiceTime;  // add the single service time to get the total time
                        averageWaitingTime1 += it->waitingTime;  // add the single waiting time to get the total time
                        count1 += 1;
                    }else if(it->queueNum ==2){
                        averageServiceTime2 += it->originalServiceTime;  // add the single service time to get the total time
                        averageWaitingTime2 += it->waitingTime;  // add the single waiting time to get the total time  
                        count2 += 1;                      
                    }else if(it->queueNum ==3){
                        averageServiceTime3 += it->originalServiceTime;  // add the single service time to get the total time
                        averageWaitingTime3 += it->waitingTime;  // add the single waiting time to get the total time     
                        count3 += 1;                   
                    }else{
                        averageServiceTime4 += it->originalServiceTime;  // add the single service time to get the total time
                        averageWaitingTime4 += it->waitingTime;  // add the single waiting time to get the total time   
                        count4 += 1;                     
                    }
                    averageServiceTime += it->originalServiceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                    // cout << it->waitingTime << endl;
                    

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                averageServiceTime1 = averageServiceTime1/count1;
                averageServiceTime2 = averageServiceTime2/count2;
                averageServiceTime3 = averageServiceTime3/count3;
                averageServiceTime4 = averageServiceTime4/count4;

                averageWaitingTime1 = averageWaitingTime1/count1;
                averageWaitingTime2 = averageWaitingTime2/count2;
                averageWaitingTime3 = averageWaitingTime3/count3;
                averageWaitingTime4 = averageWaitingTime4/count4;

                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                totalAverageWaitingTime1 += averageWaitingTime1;
                totalAverageWaitingTime2 += averageWaitingTime2;
                totalAverageWaitingTime3 += averageWaitingTime3;
                totalAverageWaitingTime4 += averageWaitingTime4;

                outPutMasterClock += masterClock;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleWaitMean1 = totalAverageWaitingTime1 / float(kRuns);
            sampleWaitMean2 = totalAverageWaitingTime2 / float(kRuns);
            sampleWaitMean3 = totalAverageWaitingTime3 / float(kRuns);
            sampleWaitMean4 = totalAverageWaitingTime4 / float(kRuns);

            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;

            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
            // cout << "sample wait1 mean " << sampleWaitMean1 << endl;
            // cout << "sample wait2 mean " << sampleWaitMean2 << endl;
            // cout << "sample wait3 mean " << sampleWaitMean3 << endl;
            // cout << "sample wait4 mean " << sampleWaitMean4 << endl;
    
    }else if (mDiscipline == 0){    // IO
        // when departs, roll a r for which io queue to enter or leave.
        // if in the io queue, roll a service time
        // when io finished, push back to cpu
        // repeat until k customers leaves cpu
        // 4 types of depart D-cpu D1-IO1 D2-IO2 D3-IO3
        // 4 types of arrivale.. maybe I dont need the arrival since I just roll a r in cpu depart and push it to the queue and create 
        // a depart, and then when the dpart event happens just do it again(roll)


        // to get the time for each queue
        // I can create a cpu service time, cpu waiting, io1.... for a client
        // every time when it is changeing queue in depart, I can just add up this servicetime += random number
        // waiting time += clock - s - a
        // all time initialized to 0


        // the time for cpu queue should be similar to the mm1 queues
        // 
        // float cpuV = 0;
        // float io1V = 0;
        // float io2V = 0;
        // float io3V = 0;

        float cpuAvertVisited = 0;
        float IO1AverageVisited = 0;
        float IO2AverageVisited = 0;
        float IO3AverageVisited = 0;
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

                std::list<Client> CPUclientQueue; // A list to represent the CPU queue.
                std::list<Client> IO1clientQueue; // A list to represent the IO1 queue.
                std::list<Client> IO2clientQueue; // A list to represent the IO2 queue.
                std::list<Client> IO3clientQueue; // A list to represent the IO3 queue.
                float CPUaverageServiceTime = 0; // average time of service
                float CPUaverageWaitingTime = 0; // average time of waitting
                float IO2averageServiceTime = 0; // average time of service
                float IO2averageWaitingTime = 0; // average time of waitting
                float IO3averageServiceTime = 0; // average time of service
                float IO3averageWaitingTime = 0; // average time of waitting
                float IO1averageServiceTime = 0; // average time of service
                float IO1averageWaitingTime = 0; // average time of waitting
                int CPUcount = 0;
                int IO1count = 0;
                int IO2count = 0;
                int IO3count = 0;
                int rp = 0;
                

                // start the loop until k customer is served.
                do{
                    // if (count > transient_period){
                    //     cout << "a" << endl;
                    // }
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        // set the queuenumber
                        tempC.setQueueNumber(1);  // since it is first give it doesnt matter.
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        // tempC.CPUtimes += 1;
                        clientInServer = tempC; // add first client to server
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        // if(clientInServer.waitingTime > 20.0){
                        //     // cout << clientInServer.aTime << " after change "<< endl;
                        //     // cout << masterClock << endl;
                        //     // cout << eventList.size() << endl;
                        //     cout << CPUclientQueue.size() << endl;
                        //     // cout << "end-----------------------" << endl;

                        // }
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;

                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;
                            // I get a new client, roll a p for which queue to go
                            // if all queues are empty, i push it to the server
                            // if not, push the priority one and push the client to the queue it belongs.
                            // so I should push it to the belonging queue first, and then gothorugh all queues to pick one.

                            aK++;
                            // Create the new client based on the arrival
                            Client tempC(temp.id, temp.time); // ID and arrival time              
                            // check the server time in case of empty queue
                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.

                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                            }
                            // tempC.CPUtimes += 1;  // update the cpu times
                            // finished client created
                            // cout << "CPU service time " << tempC.serviceTime << endl;
                            //when last Depart the queue is empty, created a depart based on the
                            // current arrival, but after the depart, a new arrival from the IO
                            // So in the interval, I need to check wehter the queue is empty upon arrival event
                            //

                            // check is the server empty
                            if (clientInServer.id == -1){   // if server is empty, update server
                                // if server is empty, I need to check all queues is it empty, and pick the priority one
                                    // if all queues are empty
                                    // push to the server
                                    clientInServer = tempC;
                                
                            }else{  // if not empty, push to the queue
                                CPUclientQueue.push_back(tempC);
                            }

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        // }else if(temp.type == 'I'){
                        //     // io recyle arrives.
                        //     // passed client only has the arrival time.
                        //     Client tempC = temp.eventClient;
                        //     tempC.CPUtimes += 1;
                        //     tempC.setServiceTime(expdev(&idum, 1.0)); // give service time
                            
                        //     if (clientInServer.id == -1){   // if server is empty, update server
                        //         // when the server is empty, there is a D event based on current A event
                        //         // so I need to update the D event 
                        //         tempC.setDtime(tempC.serviceTime + masterClock);
                        //         for (int l = 0; l < eventList.size(); l++){
                        //             std::list<Event>::iterator itE = std::next(eventList.begin(), l);


                        //             if(itE->type == 'D'){
                        //                 itE->time = tempC.dTime;
                        //             }
                        //         }
                        //         clientInServer = tempC;
                                
                        //     }else{  // if not empty, push to the queue
                        //         CPUclientQueue.push_back(tempC);
                        //     }
                        
                        }else if(temp.type == '1'){ 
                        // IO1 depart
                        // put it back to the cpu queue(clientqueu1)
                        Client tempC = IO1clientQueue.front();
                        // remove the client
                        IO1clientQueue.pop_front();
                        // update the io data for this time
                        tempC.setDtime(masterClock);
                        tempC.setWaitTime(tempC.dTime - tempC.aTime - tempC.serviceTime);
                     
                        tempC.io1Service += tempC.serviceTime;
                        tempC.io1Wait += tempC.waitingTime;


                        // // create a new Io arrival event for cpu queu
                        // tempC.clear();
                        // tempC.aTime = masterClock;
                        // eventList.push_back(Event(tempC.id,tempC.serviceTime, 'I'));
                        // eventList.sort(); // sort the list

                        // put it back to the cpu queue
                        // reset the arrival time
                        tempC.aTime = masterClock;
                        tempC.setServiceTime(expdev(&idum, 1.0));
                        if(clientInServer.id < 0){
                            clientInServer = tempC;
                            clientInServer.setWaitTime(0.0);
                            clientInServer.dTime = clientInServer.aTime + clientInServer.serviceTime;
                            std::list<Event>::iterator iit;
                            for (iit = eventList.begin(); iit != eventList.end(); ++iit){
                                if (iit->type == 'D'){
                                    break;
                                }
                            }
                            iit->time = clientInServer.dTime;
                            eventList.sort(); // sort the list
                        }else{
                            CPUclientQueue.push_back(tempC);
                        }

                        
                        
                        // check is the queu empty
                        if (IO1clientQueue.size() != 0){
                            // if not empty, create next the event
                            eventList.push_back(Event(IO1clientQueue.front().id, masterClock + IO1clientQueue.front().serviceTime, '1'));
                            eventList.sort(); // sort the list
                        }
                        // if empty, no new event is created.

                        }else if(temp.type == '2'){
                        // IO2 depart
                        // put it back to the cpu queue(clientqueu1)
                        Client tempC = IO2clientQueue.front();
                        // remove the client
                        IO2clientQueue.pop_front();
                        // update the io data for this time
                        tempC.setDtime(masterClock);
                        tempC.setWaitTime(tempC.dTime - tempC.aTime - tempC.serviceTime);
                        tempC.io2Service += tempC.serviceTime;
                        tempC.io2Wait += tempC.waitingTime;


                        // // create a new Io arrival event for cpu queu
                        // tempC.clear();
                        // tempC.aTime = masterClock;
                        // eventList.push_back(Event(tempC.id,tempC.serviceTime, 'I'));
                        // eventList.sort(); // sort the list
                        // update cpu times
                        // tempC.CPUtimes += 1;
                        // put it back to the cpu queue
                        // reset the arrival time
                        tempC.aTime = masterClock;
                        tempC.setServiceTime(expdev(&idum, 1.0));
                        if(clientInServer.id < 0){
                            clientInServer = tempC;
                            clientInServer.dTime = clientInServer.aTime + clientInServer.serviceTime;
                            clientInServer.setWaitTime(0.0);
                            std::list<Event>::iterator iit;
                            for (iit = eventList.begin(); iit != eventList.end(); ++iit){
                                if (iit->type == 'D'){
                                    break;
                                }
                            }
                            iit->time = clientInServer.dTime;
                            eventList.sort(); // sort the list
                        }else{
                            CPUclientQueue.push_back(tempC);
                        }

                        
                        
                        // check is the queu empty
                        if (IO2clientQueue.size() != 0){
                            // if not empty, create next the event
                            eventList.push_back(Event(IO2clientQueue.front().id, masterClock + IO2clientQueue.front().serviceTime, '2'));
                            eventList.sort(); // sort the list
                        }
                        // if empty, no new event is created.

                        }else if(temp.type == '3'){
                        // IO3 depart
                        // put it back to the cpu queue(clientqueu1)
                        Client tempC = IO3clientQueue.front();
                        // remove the client
                        IO3clientQueue.pop_front();
                        // update the io data for this time
                        tempC.setDtime(masterClock);
                        tempC.setWaitTime(tempC.dTime - tempC.aTime - tempC.serviceTime);
                        tempC.io3Service += tempC.serviceTime;
                        tempC.io3Wait += tempC.waitingTime;
                        // update cpu times

                        // // create a new Io arrival event for cpu queu
                        // tempC.clear();
                        // tempC.aTime = masterClock;
                        // eventList.push_back(Event(tempC.id,tempC.serviceTime, 'I'));
                        // eventList.sort(); // sort the list

                        // tempC.CPUtimes += 1;
                        // put it back to the cpu queue
                        // reset the arrival time
                        tempC.aTime = masterClock;
                        tempC.setServiceTime(expdev(&idum, 1.0));
                        if(clientInServer.id < 0){
                            clientInServer = tempC;
                            clientInServer.setWaitTime(0.0);

                            clientInServer.dTime = clientInServer.aTime + clientInServer.serviceTime;
                            std::list<Event>::iterator iit;
                            for (iit = eventList.begin(); iit != eventList.end(); ++iit){
                                if (iit->type == 'D'){
                                    break;
                                }
                            }
                            iit->time = clientInServer.dTime;
                            eventList.sort(); // sort the list
                        }else{
                            CPUclientQueue.push_back(tempC);
                        }

                        
                        
                        // check is the queu empty
                        if (IO3clientQueue.size() != 0){
                            // if not empty, create next the event
                            eventList.push_back(Event(IO3clientQueue.front().id, masterClock + IO3clientQueue.front().serviceTime, '3'));
                            eventList.sort(); // sort the list
                        }
                        // if empty, no new event is created.

                        
                        }else{ // CPU depart
                        clientInServer.CPUtimes ++;
                        // update the cpu data of this visit
                        clientInServer.setDtime(masterClock);
                        clientInServer.setWaitTime(masterClock - clientInServer.aTime - clientInServer.serviceTime);
                        
                        if (clientInServer.waitingTime < 0){
                            clientInServer.waitingTime = 0.0;
                        }
                        clientInServer.cpuWait += clientInServer.waitingTime;
                        clientInServer.cpuService += clientInServer.serviceTime;
                                // if(clientInServer.waitingTime > 100.0){
                                //     // cout << clientInServer.aTime << " after change "<< endl;
                                //     // cout << masterClock << endl;
                                //     // cout << eventList.size() << endl;
                                //     cout << CPUclientQueue.size() << endl;
                                //     // cout << "end-----------------------" << endl;

                                // }
                        // roll a number decide is this going to be real depart or go to IO
                            rp = int(expdev(&idum, lambda) * 100000) % 100;
                            // rp = int(expdev(&idum, 1.0) * 10000000) % 100;

                            // cout << int(expdev(&idum, 1.0) * 100) % 100 << endl;
                            if (rp < 70){   // r0 actually leaves the server
                                clientInServer.totalService = clientInServer.cpuService + clientInServer.io1Service + clientInServer.io2Service + clientInServer.io3Service;
                                clientInServer.totalWait = clientInServer.cpuWait + clientInServer.io1Wait + clientInServer.io2Wait + clientInServer.io3Wait;
                                // push to the datalist
                                dataList.push_back(clientInServer);
                                // cout << clientInServer.cpuWait << endl;
                                // cout << count << endl;
                                // if (clientInServer.cpuWait > 50){
                                //     cout << CPUclientQueue.size() << endl;
                                //     cout << CPUclientQueue.front().aTime << " arrival "<< endl;
                                //     cout << clientInServer.dTime << " current d "<< endl;
                                //     cout << masterClock << " master "<< endl;
                                //     cout << CPUclientQueue.back().aTime << " tail arrival "<< endl;


                                // }
                                if(count == k + transient_period){ // stop if k cutomer is served
                                    break;
                                }
                                count++;

                            }else if(rp < 80){  // r1 go to I/O 1(clientQueue2)
                                // roll the extra service time and create the new IO depart event
                                // roll the extra service time

                                // update the io times
                                clientInServer.IO1times += 1;

                                // if(clientInServer.waitingTime > 100.0){
                                //     cout << "start-----------------------" << endl;
                                //     cout << clientInServer.aTime << endl;
                                //     cout << masterClock << endl;
                                // }
                                // update the arrival time for IO
                                clientInServer.aTime = masterClock;

                                // if(clientInServer.waitingTime > 100.0){
                                //     // cout << clientInServer.aTime << " after change "<< endl;
                                //     // cout << masterClock << endl;
                                //     // cout << eventList.size() << endl;
                                //     cout << CPUclientQueue.size() << endl;
                                //     // cout << "end-----------------------" << endl;

                                // }

                                // roll the service time for IO
                                clientInServer.setServiceTime(5.0 * expdev(&idum, 1.0));
                                // cout << "IO time service " << clientInServer.serviceTime << endl;
                                if (IO1clientQueue.size() == 0){
                                    // when io queue is empty, push it to the queue and create the depart event
                                    // create the depart time for IO
                                    clientInServer.setDtime(masterClock + clientInServer.serviceTime);
                                    // create the depart event
                                    eventList.push_back(Event(clientInServer.id, clientInServer.dTime, '1'));
                                    eventList.sort(); // sort the list


                                }
                                // push to queue
                                IO1clientQueue.push_back(clientInServer);

                            }else if(rp < 90){  // r2

                                // update the io times
                                clientInServer.IO2times += 1;

                                // update the arrival time for IO
                                clientInServer.aTime = masterClock;

                                // roll the service time for IO
                                clientInServer.setServiceTime(5.0 * expdev(&idum, 1.0));

                                if (IO2clientQueue.size() == 0){
                                    // when io queue is empty, push it to the queue and create the depart event
                                    // create the depart time for IO
                                    clientInServer.setDtime(masterClock + clientInServer.serviceTime);
                                    // create the depart event
                                    eventList.push_back(Event(clientInServer.id, clientInServer.dTime, '2'));
                                    eventList.sort(); // sort the list
                                }
                                // push to queue
                                IO2clientQueue.push_back(clientInServer);
                                
                            }else{  //r3

                                // update the io times
                                clientInServer.IO3times += 1;

                                // update the arrival time for IO
                                clientInServer.aTime = masterClock;

                                // roll the service time for IO
                                clientInServer.setServiceTime(5.0 * expdev(&idum, 1.0));

                                if (IO3clientQueue.size() == 0){
                                    // when io queue is empty, push it to the queue and create the depart event
                                    // create the depart time for IO
                                    clientInServer.setDtime(masterClock + clientInServer.serviceTime);
                                    // create the depart event
                                    eventList.push_back(Event(clientInServer.id, clientInServer.dTime, '3'));
                                    eventList.sort(); // sort the list


                                }
                                // push to queue
                                IO3clientQueue.push_back(clientInServer);
                            }

                            // clear the server
                            clientInServer.empty(); // set the id to -1
   
 


                            // check is queue empty
                            if (CPUclientQueue.size() != 0){ // when cpu is not empty
                                // push the next client into server, since shortest first, just push the first from queue
                                clientInServer = CPUclientQueue.front();   // push to server
                                // remove it from queue
                                CPUclientQueue.pop_front();
                                // based on the service time and current clock, get the depart time
                                clientInServer.setDtime(clientInServer.serviceTime + masterClock);
                                // generate next depart event from the client in server
                                eventList.push_back(Event(clientInServer.id, clientInServer.dTime, 'D'));
                                eventList.sort(); // sort the list

                            }else{  // when cpu queues is empty, push the depart event based on the current arrival
                                std::list<Event>::iterator iit;
                                for (iit = eventList.begin(); iit != eventList.end(); ++iit){
                                    if (iit->type == 'A'){
                                        break;
                                    }
                                }
                                iit->setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(iit->id, iit->serviceTime+iit->time, 'D'));
                                eventList.sort(); // sort the list 
                                // cout << eventList.front().serviceTime+eventList.front().time << endl;
                            }
                            // add next dep event to the event list.




                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
                    // normal clients
                    averageServiceTime += it->totalService;  // add the single service time to get the total time
                    averageWaitingTime += it->totalWait;  // add the single waiting time to get the total time
                    CPUcount += it->CPUtimes;
                    
                    CPUaverageWaitingTime += it->cpuWait;
                    CPUaverageServiceTime += it->cpuService;

                    IO1count += it->IO1times;
                    IO1averageServiceTime += it->io1Service;
                    IO1averageWaitingTime += it->io1Wait;

                    IO2count += it->IO2times;
                    IO2averageServiceTime += it -> io2Service;
                    IO2averageWaitingTime += it->io2Wait;
                    

                    IO3count += it->IO3times;
                    IO3averageServiceTime += it -> io3Service;
                    IO3averageWaitingTime += it->io3Wait;
                    // if (it->CPUtimes != 0){
                    //     cpuV++;
                    // }
                    // if (it->IO1times != 0){
                    //     io1V++;
                    // }
                    // if (it->IO2times != 0){
                    //     io2V++;
                    // }

                    // if (it->IO3times != 0){
                    //     io3V++;
                    // }

                }
                cpuAvertVisited += float(CPUcount)/float(k);
                IO1AverageVisited += float(IO1count)/float(k);
                IO2AverageVisited += float(IO2count)/float(k);
                IO3AverageVisited += float(IO3count)/float(k);
                int total_count = CPUcount + IO1count + IO2count + IO3count;
                averageSystemTime =  ( averageServiceTime + averageWaitingTime) / total_count;
                averageServiceTime = (CPUaverageServiceTime + IO1averageServiceTime + IO2averageServiceTime + IO3averageServiceTime) / total_count;  // total service time divided by k
                averageWaitingTime = (CPUaverageWaitingTime + IO1averageWaitingTime + IO2averageWaitingTime + IO3averageWaitingTime) / total_count;  // total waiting time divided by k
                CPUaverageWaitingTime = CPUaverageWaitingTime/CPUcount;
                IO1averageWaitingTime = IO1averageWaitingTime/IO1count;
                IO2averageWaitingTime = IO2averageWaitingTime/IO2count;
                IO3averageWaitingTime = IO3averageWaitingTime/IO3count;

                // averageServiceTime1 = averageServiceTime1/count1;
                // averageServiceTime2 = averageServiceTime2/count2;
                // averageServiceTime3 = averageServiceTime3/count3;
                // averageServiceTime4 = averageServiceTime4/count4;

                // averageWaitingTime1 = averageWaitingTime1/count1;
                // averageWaitingTime2 = averageWaitingTime2/count2;
                // averageWaitingTime3 = averageWaitingTime3/count3;
                // averageWaitingTime4 = averageWaitingTime4/count4;

                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;

                CPUtotalAverageWaitingTime += CPUaverageWaitingTime;
                IO1totalAverageWaitingTime += IO1averageWaitingTime;
                IO2totalAverageWaitingTime += IO2averageWaitingTime;
                IO3totalAverageWaitingTime += IO3averageWaitingTime;

                
                // totalAverageWaitingTime1 += averageWaitingTime1;
                // totalAverageWaitingTime2 += averageWaitingTime2;
                // totalAverageWaitingTime3 += averageWaitingTime3;
                // totalAverageWaitingTime4 += averageWaitingTime4;

                outPutMasterClock += masterClock;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }
            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleWaitMeanCPU = CPUtotalAverageWaitingTime / float(kRuns);
            sampleWaitMeanIO1 = IO1totalAverageWaitingTime / float(kRuns);
            sampleWaitMeanIO2 = IO2totalAverageWaitingTime / float(kRuns);
            sampleWaitMeanIO3 = IO3totalAverageWaitingTime / float(kRuns);

            cpuAvertVisited = cpuAvertVisited/kRuns;
            IO1AverageVisited = IO1AverageVisited/kRuns;
            IO2AverageVisited = IO2AverageVisited/kRuns;
            IO3AverageVisited = IO3AverageVisited/kRuns;
            // sampleWaitMean1 = totalAverageWaitingTime1 / float(kRuns);
            // sampleWaitMean2 = totalAverageWaitingTime2 / float(kRuns);
            // sampleWaitMean3 = totalAverageWaitingTime3 / float(kRuns);
            // sampleWaitMean4 = totalAverageWaitingTime4 / float(kRuns);

            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "The value of the master clock at the end of the simulation: " << outPutMasterClock << endl;
            // cout << "sample system mean " << sampleSystemMean << endl;
            // cout << "sample system variance " << sampleSystemVariance << endl;
            // cout << "sample wait mean " << sampleWaitMean << endl;
            // cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
            // cout << "CPU sample wait mean " << sampleWaitMeanCPU << endl;
            // cout << "IO1 sample wait mean " << sampleWaitMeanIO1 << endl;
            // cout << "IO2 sample wait mean " << sampleWaitMeanIO2 << endl;
            // cout << "IO3 sample wait mean " << sampleWaitMeanIO3 << endl;
            // cout << "CPU sample visited mean " << cpuAvertVisited << endl;
            // cout << "IO1 sample visited mean " << IO1AverageVisited << endl;
            // cout << "IO2 sample visited mean " << IO2AverageVisited << endl;
            // cout << "IO3 sample visited mean " << IO3AverageVisited << endl;


    }else if(mDiscipline == 10){
        int mmm = 3;
        if ( mmm == 111){
            cout << mmm << endl;
        }
    }

  
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        elapsed).count();
// To get the value of duration use the count()
// member function on the duration object
    // cout << "Time taken by function: "<< microseconds << endl;
}


// • the value of the input parameter λ
// • the value of the input parameter K
// • the value of the master clock at the end of the simulation
// • the average service time (based on the K served customers only) -> time joined the queue - time left the queue
//      previous d time = current d time
// • the average waiting time (based on the K served customers only) -> arrival time - time join the queue
// • the arrival time, service time, time of departure of customers L, L + 1, L + 10, and L + 11, as well as the number of customers in the system immediately after the departure of each of these customers.



// for confidence interval x mean + (s/sqrt(n) * z score)

// prempt a new arrival q2, check is next dpart event has lower priority(no need to check it self since fcfs), push to the queue, next loop
// next arrival is q1, check priority, and found next is lower, so push it to the q1, and update the last arrival q2's service time(q2 service time - q2's arrival - q1's arrivale)
// push q2 back to q2, update dpart event time(clock + q1 service)

// for depart, just go though all queues, and pick one by order for generating  dpart 


// create a empty client clientInServer, representing the client in server
// for arrival, check is the server empty, if yes, update it, if not, push to the queue
// for depart, if server is not empty, depart the server, and generate next depart based on the first queue, and push it into the server.
// if the server is empty, which means the queue is empty too, generate the next depart event based on the arrival