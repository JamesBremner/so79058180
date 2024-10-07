#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include "GraphTheory.h"

struct sEmployee
{
    // configuration
    std::string myName;
    std::vector<int> myCan;
    int myPayLimit;
    int myCrateLimit;
    int myEfficiency;

    // status
    int myPay;
    int myCrate;

    // default constructor  - unemployably inefficient
    sEmployee()
        : myEfficiency(INT_MAX)
    {
    }

    // true if employee can push crate
    bool isCapable(int c)
    {
        return (
            std::find(
                myCan.begin(), myCan.end(), c) != myCan.end());
    }
};

struct sAssign
{
    sEmployee *myEmp;
    int myCrate;
    int myPaid;

    sAssign(sEmployee *e, int crate, int paid)
        : myEmp(e),
          myCrate(crate),
          myPaid(paid)
    {
        std::cout << e->myName << " crate " << crate
                  << " paid " << paid
                  << "\n";
    }
};

std::vector<int> theBudget;
std::vector<sEmployee> theEmployees;
std::vector<sAssign> theAssigns;

// generate example problem
void generate1()
{
    sEmployee e;
    e.myName = "Alice";
    e.myCan = {0, 1, 2};
    e.myPayLimit = 35;
    e.myCrateLimit = 2;
    e.myEfficiency = 3;
    e.myPay = 0;
    e.myCrate = 0;
    theEmployees.push_back(e);
    e.myName = "Bob";
    e.myCan = {0, 3};
    e.myPayLimit = INT_MAX;
    e.myCrateLimit = INT_MAX;
    e.myEfficiency = 2;
    theEmployees.push_back(e);

    theBudget = {20, 20, 20, 20, 20};
}
void generate2()
{
    sEmployee e;
    e.myName = "Alice";
    e.myCan = {0, 1, 2};
    e.myPayLimit = INT_MAX; // remove pay limit, so crate limit will come into play
    e.myCrateLimit = 2;
    e.myEfficiency = 3;
    e.myPay = 0;
    e.myCrate = 0;
    theEmployees.push_back(e);
    e.myName = "Bob";
    e.myCan = {0, 3};
    e.myPayLimit = INT_MAX;
    e.myCrateLimit = INT_MAX;
    e.myEfficiency = 2;
    theEmployees.push_back(e);

    theBudget = {20, 20, 20, 20, 20};
}

std::string crateName(int index)
{
    return std::string(1, (char)('A' + index));
}

raven::graph::sGraphData makeGraph()
{
    raven::graph::sGraphData gd;
    gd.g.directed();

    // loop over employees
    for (auto &e : theEmployees)
    {
        // connect source to each employe
        // limiting the capacity to the employees pay limit
        gd.g.add("src", e.myName);
        gd.edgeWeight.push_back(e.myPayLimit);

        // connect each employee to the crates they can push
        for (int c : e.myCan)
        {
            gd.g.add(e.myName, crateName(c));
            gd.edgeWeight.push_back(INT_MAX);
        }
    }

    // loop over crates
    for (int i = 0; i < theBudget.size(); i++)
    {
        // connect crates to the sink
        // limiting capacity to the budget for that crate
        gd.g.add(crateName(i), "snk");
        gd.edgeWeight.push_back(theBudget[i]);
    }

    // flow from source to sink
    gd.startName = "src";
    gd.endName = "snk";

    return gd;
}

std::vector<int> maxFlow(
    raven::graph::sGraphData &gd)
{
    std::vector<int> vEdgeFlow;

    //  Maximum flows using Edmonds–Karp implementation of Ford–Fulkerson
    raven::graph::flows(gd, vEdgeFlow);

    return vEdgeFlow;
}

bool checkCrateLimit(
    raven::graph::sGraphData &gd,
    std::vector<int> &vEdgeFlow)
{
    for (auto &e : theEmployees)
    {
        if (e.myCrateLimit == INT_MAX)
            continue;

        int crateCount = 0;
        for (int c : e.myCan)
        {
            int ei = gd.g.find(
                e.myName,
                crateName(c));
            if (ei > 0)
                if (vEdgeFlow[ei] > 0)
                    crateCount++;
        }
        if (crateCount > e.myCrateLimit)
        {
            std::cout << e.myName << " exceeds crate limit\n";
            return false;
        }
    }
    return true;
}
void display(
    raven::graph::sGraphData &gd,
    std::vector<int> &vEdgeFlow)
{

    // std::cout << "Flows\n";
    // for (int e = 0; e < gd.g.edgeCount(); e++)
    //     std::cout << gd.g.userName(gd.g.src(e))
    //               << " " << gd.g.userName(gd.g.dest(e))
    //               << " " << vEdgeFlow[e]
    //               << "\n";

    // calculate totals and display
    int totalDistance = 0;
    for (auto &e : theEmployees)
    {
        int totalPay = 0;
        for (int kc = 0; kc < theBudget.size(); kc++)
        {
            int ei = gd.g.find(
                e.myName,
                std::string(1, (char)('A' + kc)));
            if (ei > 0)
                totalPay += vEdgeFlow[ei];
        }
        std::cout << e.myName << " is paid " << totalPay << "\n";
        totalDistance += totalPay * e.myEfficiency;
    }
    std::cout << "Total Distance " << totalDistance << "\n";
}
main()
{
    generate2();

    auto gd = makeGraph();

    auto flows = maxFlow(gd);

    checkCrateLimit(gd, flows);

    display(gd, flows);

    return 0;
}
