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
#define AMP (10000000000.0/IM)
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

float ran0BPD(long *idum){
    long k;
    float ans;

    *idum ^= MASK;
    k = (*idum)/IQ;
    *idum = IA*(*idum-k*IQ)-IR*k;
    if (*idum < 0) *idum+=IM;
    ans = 332 + AMP*(*idum);

    *idum ^= MASK;
    // cout << ans << endl;
    return ans;
}

// Given method from class
float expdev(long *idum, double lamb0){
    float ran0(long *idum);
    float dummy;

    do
        dummy = ran0(idum);
    while (dummy == 0.0);


    return -log(dummy) * (1/lamb0);


    
}

// Given method from class
float expdevService(long *idum, double lamb0, int select){
    if(select == 0){
        float ran0(long *idum);
        float dummy;

        do
            dummy = ran0(idum);
        while (dummy == 0.0);


        return -log(dummy) * (1/lamb0);
    }else{
        float ran0BPD(long *idum);
        float dummy;

        do
            dummy = ran0BPD(idum);
        while (dummy == 0.0);
        // cout << dummy << endl;
        return pow(-593.26872/(0.9984722625-dummy),10./11.);

        // srand(*idum);
        // float dummy = (rand() % (9999999669) + 332);
        
        // cout << dummy << endl;
        // return pow(-593.26872/(0.9984722625-dummy),10./11.);
    }

    
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

bool compare_depart(const Client& first, const Client& second){
    return first.dTime < second.dTime;
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
    long k = 50000; // The k value, how many clients before terminates
    float lambda = 3. * 0.1 * 3000.; // THe lambda value.
    // int mDiscipline = 0; // 0 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P. 0 for debug
    int disciplineL = 0;    // 0 – FCFS, 1 – SJF-NP
    float mu = 3000.0;
    // float mu = 0.99999;
    int systemM = 0;     // 0 - M/M/3, 1 - M/G/3
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


    // 1. the parameter λ of the distribution of interarrival times
    // 2. the number K of customers serviced before the program terminates
    // 3. an integer L such that: 0 – FCFS, 1 – SJF.
    // 4. an integer M such that: 0 – M/M/3 system, 1 – M/G/3 system.

    // //////////////////////////// Get input //////////////////////////////
    // Get the inputs from user
    // cout << "Please input the parameter lambda." << endl;  // Lambda
    // cin >> lambda;
    // while(cin.fail() || lambda <= 0.0) {
    //     std::cout << "0 < lambda" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> lambda;
    // }    

    // cout << "Please input the number K(Positive Intger Greater than 2)." << endl;  // K
    // cin >> k;
    // while(cin.fail() || k <= 2) {
    //     std::cout << "K must be a positive integer Greater than 2" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> k;
    // }    

    // cout << "The integer L such that: 0 – FCFS, 1 – SJF. " << endl;  // L
    // cin >> disciplineL;
    // while(cin.fail() || disciplineL < 0 || disciplineL > 1) {
    //     std::cout << "0 <= l <= 1" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> disciplineL;
    // }  

    // cout << "Integer M such that: 0 – M/M/3 system, 1 – M/G/3 system. " << endl;  // L
    // cin >> systemM;
    // while(cin.fail() || systemM < 0 || systemM > 5) {
    //     std::cout << "0 <= m <= 1" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> systemM;
    // }  

    // //////////////////////////// Get input //////////////////////////////


    // cout << "The mu is " << mu << endl;
    // cout << lambda << endl;
    // cout << AMP << endl;
    // float lll = 10./11.;
    // cout << "the 10/11 " << lll << endl;
    // float firstpart =-593.26872/(0.9984722625-5000.);
    // cout << "the first part " << firstpart << endl;

    // cout << pow(593.26872/(0.9984722625-5000),0.909091) << endl;
    // cout << pow(firstpart,0.909091) << endl;

    if ( disciplineL == 0){ // FCFS-NP
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Event> clientQueue; // A list to represent the queue.
                std::list<Client> servers;
                // Client servers[3];
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.
                float tempArrival = -1;
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
                        firstTemp.setDtime(expdevService(&idum, mu, systemM)+masterClock);
                        servers.push_back(firstTemp);
                        // clientInServer1 = firstTemp;     // push to server
                        eventList.pop_back(); // pop the first event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, firstTemp.dTime, 'D'));
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

                            if (servers.size() < 4){   // if less than 3 servers are ocuppied
                                tempC.setDtime(expdevService(&idum, mu, systemM)+masterClock);
                                servers.push_back(tempC);
                            }else{  // if not empty, push to the queue
                                clientQueue.push_back(temp); // add to the back of queue since last first
                            }

                            // if (clientInServer1.id == -1){   // if server 1 is empty, update server
                            //     clientInServer1 = tempC;
                            // }else if(clientInServer2.id == -1){
                            //     clientInServer2 = tempC;

                            // }else if(clientInServer3.id == -1){
                            //     clientInServer3 = tempC;
                            // }else{  // if not empty, push to the queue
                            //     clientQueue.push_back(temp); // add to the back of queue since last first
                            // }
                            
                            // put client into the list with id and atime;
                            aK++;
                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.

                            servers.sort(compare_depart);  // sort the servers list first
                            dataList.push_back(servers.front());
                            servers.pop_front();


                            if(count == k + transient_period){ // stop if k cutomer is served
                                break;
                            }

                            // Check is the server full (remember I need to generate the D event)
                            // if it is full
                            // simply generate the next D based on the font
                            // if it is not full, fill it until servers.size() == 3 or queue.size() == 0
                            // and check is the server empty
                            // if the server is not empty, sort it and generate next D
                            // if the server is empty, generate based on the next arrival


                            if (servers.size() == 3){   // server is full
                            // generate next D based on the front
                                servers.sort(compare_depart);  // sort the servers list first
                                eventList.push_back(Event(clientQueue.front().id, servers.front().dTime, 'D'));
                            }else{   // server is not full
                            // fill the server until servers.size() == 3 or queue.size() == 0
                                while(servers.size() != 3 && clientQueue.size() != 0){
                                    Client tempC(clientQueue.front().id, clientQueue.front().time);  // set the id and arrival time
                                    tempC.setDtime(expdevService(&idum, mu, systemM)+masterClock);
                                    clientQueue.pop_front();
                                    servers.push_back(tempC);
                                }
                                // check is the server empty
                                if (servers.size() != 0){
                                // if the server is not empty, sort it and generate next D
                                    servers.sort();
                                    eventList.push_back(Event(servers.front().id, servers.front().dTime, 'D'));
                                    eventList.sort(); // sort the list                        
                                }else{
                                // if the server is empty, generate based on the next arrival
                                    eventList.push_back(Event(eventList.front().id, expdevService(&idum, mu, systemM)+eventList.front().time, 'D'));
                                    eventList.sort(); // sort the list 
                                }
                            }


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
                    
                    // cout << it->serviceTime << endl;

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
    
    


    }else if (disciplineL == 1){  // SJF-NP
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
                std::list<Client> servers;

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
                        tempC.setServiceTime(expdevService(&idum, mu, systemM)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        servers.push_back(tempC);
                        // clientInServer = tempC; // add first client to server
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
                                tempC.setServiceTime(expdevService(&idum, mu,systemM)); // set service time
                            }
                            // finished client created




                            if (servers.size() < 3){   // if less than 3 servers are ocuppied
                                tempC.setDtime(tempC.serviceTime+masterClock);
                                servers.push_back(tempC);
                            }else{  // if not empty, push to the queue
                                clientQueue.push_back(tempC); // add to the back of queue since last first
                            }


                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // first depart the server cliend and push to data list
                            // second empty the server
                            // third check the queue.
                            // cout << servers.front().serviceTime << endl;
                            servers.sort(compare_depart);  // sort the servers list first
                            dataList.push_back(servers.front());
                            servers.pop_front();
                            clientQueue.sort();
 
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }



                            if (servers.size() == 3){   // server is full
                            // generate next D based on the front
                                servers.sort(compare_depart);  // sort the servers list first
                                eventList.push_back(Event(servers.front().id, servers.front().dTime, 'D'));
                            }else{   // server is not full
                            // fill the server until servers.size() == 3 or queue.size() == 0
                                while(servers.size() != 3 && clientQueue.size() != 0){
                                    clientQueue.front().setDtime(masterClock + clientQueue.front().serviceTime);
                                    servers.push_back(clientQueue.front());
                                    clientQueue.pop_front();
                                }
                                // check is the server empty
                                if (servers.size() != 0){
                                // if the server is not empty, sort it and generate next D
                                    servers.sort();
                                    eventList.push_back(Event(servers.front().id, servers.front().dTime, 'D'));
                                    eventList.sort(); // sort the list                        
                                }else{
                                // if the server is empty, generate based on the next arrival
                                    eventList.front().setServiceTime(expdevService(&idum, mu,systemM)); // preset the service time 
                                    eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                    eventList.sort(); // sort the list 
                                    
                                }
                            }


             

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
                    // cout << it->serviceTime << endl;
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
    

            while 
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