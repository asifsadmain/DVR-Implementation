#include <iostream>
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

#define MAX 99999

class Info
{
  public:
    string destination;
    string nexthop;
    int cost;
};

vector<Info> routingTable;
vector<Info>::iterator it;
vector<string>::iterator it1;
vector<string> IPs;

void getAllIP(string topo)
{
    string data;
    ifstream topology;
    topology.open(topo);
    int count = 0;
    while (topology >> data)
    {
        //cout << data << " ";
        count++;
        if (count % 3 == 0)
            continue;
        else
        {
            it1 = find(IPs.begin(), IPs.end(), data);
            if (it1 == IPs.end())
                IPs.push_back(data);
        }
    }
    topology.close();
    /*for(int i=0; i<IPs.size(); i++)
    {
        cout << IPs.at(i) << endl;
    }*/
}

void printRouterTable()
{
    cout << "destination \tnextHop \tcost" << endl;
    cout << "-----------\t-----------\t-----------" << endl;
    for (int i = 0; i < routingTable.size(); i++)
    {
        cout << routingTable.at(i).destination << "\t" << routingTable.at(i).nexthop << "\t" << routingTable.at(i).cost << endl;
    }
}

void initialize(string routerIP, string topo)
{
    ifstream topology;
    topology.open(topo);
    string source, nexthop, cost, line;

    while (getline(topology, line))
    {
        stringstream topoIP(line);
        vector<string> tokens;
        while (getline(topoIP, line, ' '))
        {
            tokens.push_back(line);
        }

        if (tokens.at(0) == routerIP)
        {
            Info newNode;
            newNode.destination = tokens.at(1);
            newNode.nexthop = tokens.at(1);
            newNode.cost = stoi(tokens.at(2));
            routingTable.push_back(newNode);
        }
        else if (tokens.at(1) == routerIP)
        {
            Info newNode;
            newNode.destination = tokens.at(0);
            newNode.nexthop = tokens.at(0);
            newNode.cost = stoi(tokens.at(2));
            routingTable.push_back(newNode);
        }
    }
    topology.close();
    for (int i = 0; i < IPs.size(); i++)
    {
        int flag = 0;
        for (int j = 0; j < routingTable.size(); j++)
        {
            if ((IPs.at(i) == routerIP) || (IPs.at(i) == routingTable.at(j).destination))
            {
                flag = 1;
            }
        }
        if (flag == 0)
        {
            Info newNode;
            newNode.destination = IPs.at(i);
            newNode.nexthop = "\t-";
            newNode.cost = MAX;
            routingTable.push_back(newNode);
        }
    }
    printRouterTable();
}

void updateRouterCost(string routerIP, char *ip1, char *ip2, int cost)
{
    if (routerIP == ip1)
    {
        for (int i = 0; i < routingTable.size(); i++)
        {
            if (routingTable.at(i).destination == ip2)
            {
                routingTable.at(i).cost = cost;
            }
        }
    }
    else
    {
        for (int i = 0; i < routingTable.size(); i++)
        {
            if (routingTable.at(i).destination == ip1)
            {
                routingTable.at(i).cost = cost;
            }
        }
    }
    printRouterTable();
}

int main(int argc, char *argv[])
{
    string routerIP = argv[1];
    getAllIP(argv[2]);
    initialize(routerIP, argv[2]);

    int sockfd;
    int bind_flag;
    char buffer[1024];
    int bytes_received;
    socklen_t addrlen;
    //struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    if (argc != 3)
    {
        printf("%s <ip address>\n", argv[0]);
        exit(1);
    }

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(4747);
    client_address.sin_addr.s_addr = inet_addr(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind_flag = bind(sockfd, (struct sockaddr *)&client_address, sizeof(sockaddr_in));

    if (bind_flag)
    {
        cout << "\nCONNECTION ERROR!!!" << endl;
    }
    else
    {
        cout << "\nCONNECTION ESTABLISHED SUCCESSFULLY!" << endl;
    }

    while (true)
    {
        bytes_received = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&client_address, &addrlen);
        string data(buffer);
        string command = data.substr(0, 4);
        //cout << "size = " << data.size() << endl;
        if (command == "cost")
        {
            char ip1[INET_ADDRSTRLEN];
            char ip2[INET_ADDRSTRLEN];
            char str_cost[10];

            inet_ntop(AF_INET, buffer + 4, ip1, sizeof(ip1));
            inet_ntop(AF_INET, buffer + 8, ip2, sizeof(ip2));
            inet_ntop(AF_INET, buffer + 12, str_cost, sizeof(str_cost));

            int cost = stoi(str_cost);

            updateRouterCost(routerIP, ip1, ip2, cost);
        }
        else if (command == "show")
        {
            printRouterTable();
        }
        else if (command == "send")
        {
            char ip1[INET_ADDRSTRLEN];
            char ip2[INET_ADDRSTRLEN];
            char str_length[16];
            char msg[80];

            inet_ntop(AF_INET, buffer + 4, ip1, sizeof(ip1));
            inet_ntop(AF_INET, buffer + 8, ip2, sizeof(ip2));
            inet_ntop(AF_INET, buffer + 12, str_length, sizeof(str_length));
            int length = stoi(str_length);
            for(int i=0; i<length; i++){
                msg[i] = buffer[i+14];
            }
            msg[length] = '\0';

            cout << "IP1: " << ip1 << endl;
            cout << "IP2: " << ip2 << endl;
            cout << "Length: " << length << endl;
            cout << "Msg: " << msg << endl;
        }
        //printf("[%s:%d]: %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);
    }

    close(sockfd);
}