#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include "cGUI.h"
#include <inputbox.h>
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

raven::graph::sGraphData theGraph;
std::vector<int> theFlows;

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

    // theBudget = {20, 20, 20, 20, 20};
    // theBudget = {20, 6, 20, 20, 20};
    theBudget = {500, 0, 0, 0, 0};
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

bool enforceCrateLimit(
    raven::graph::sGraphData &gd,
    std::vector<int> &vEdgeFlow)
{
    for (auto &e : theEmployees)
    {
        if (e.myCrateLimit == INT_MAX)
            continue; // no crate limit

        // count number of crates assigned to the employee
        // add find the crate with the lowest payment
        int crateCount = 0;
        int lowestPayment = INT_MAX;
        int lowestCrate;
        for (int c : e.myCan)
        {
            int ei = gd.g.find(
                e.myName,
                crateName(c));
            if (ei > 0)
            {
                int f = vEdgeFlow[ei];
                if (f > 0)
                {
                    // this employee has been paid for this crate
                    crateCount++;
                    if (f < lowestPayment)
                    {
                        // this is the crate with the least payment
                        lowestPayment = f;
                        lowestCrate = c;
                    }
                }
            }
        }

        if (crateCount <= e.myCrateLimit)
            continue; // limit not exceeded

        // std::cout << e.myName << " exceeds crate limit\n";

        // remove crate with lowest payment from this employees capable list
        gd.g.remove(
            e.myName,
            crateName(lowestCrate));

        return false;
    }
    return true;
}
std::string display()
{
    std::stringstream ss;

    // input
    ss << "Pay Limits: ";
    for (auto &e : theEmployees)
    {
        ss << e.myName << " ";
        if (e.myPayLimit == INT_MAX)
            ss << "none";
        else
            ss << e.myPayLimit;
        ss << ", ";
    }
    ss << "\nbudget: ";
    for (int b : theBudget)
        ss << b << " ";
    ss << "\n=============\n";

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
            int ei = theGraph.g.find(
                e.myName,
                crateName(kc));
            if (ei > 0)
                totalPay += theFlows[ei];
        }
        ss << e.myName << " is paid " << totalPay << "\n";
        totalDistance += totalPay * e.myEfficiency;

        ss << "( ";
        for (int kc = 0; kc < theBudget.size(); kc++)
        {
            int ei = theGraph.g.find(
                e.myName,
                crateName(kc));
            if (ei > 0)
                if (theFlows[ei] > 0)
                    ss << " " << theFlows[ei]
                       << " for crate " << crateName(kc) << " ";
        }
        ss << " )\n";
    }
    ss << "Total Distance " << totalDistance << "\n";

    return ss.str();
}
void run()
{
    theGraph = makeGraph();

    bool crateLimitOK = false;
    while (!crateLimitOK)
    {
        theFlows = maxFlow(theGraph);

        crateLimitOK = enforceCrateLimit(theGraph, theFlows);
    }

}

cGUI::cGUI()
    : fm(wex::maker::make())
{
    generate2();
    run();

    menus();

    fm.move({50, 50, 1000, 500});
    fm.text("Crate Pushing");

    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            S.text(display(), {5, 5, 1000, 1000});
        });

    fm.show();
    fm.run();
}

std::vector<std::string> tokenize(const std::string &line)
{
    std::vector<std::string> ret;
    std::stringstream sst(line);
    std::string a;
    while (getline(sst, a, ' '))
        ret.push_back(a);
    return ret;
}

void cGUI::menus()
{
    wex::menubar mb(fm);

    wex::menu mf(fm);

    mf.append("Parameters",
              [&](const std::string &title)
              {
                  wex::inputbox ib(fm);
                  ib.labelWidth(100);
                  ib.gridWidth(200);

                  std::string sp;
                  for (int b : theBudget)
                      sp += std::to_string(b) + " ";
                  ib.add("Budget", sp);

                  sp = "";
                  for (auto &e : theEmployees)
                  {
                      if (e.myPayLimit == INT_MAX)
                          sp += "none ";
                      else
                          sp += std::to_string(e.myPayLimit) + " ";
                  }
                  ib.add("Pay Limits", sp);

                  ib.show();

                  theBudget.clear();
                  for (auto s : tokenize(ib.value("Budget")))
                      theBudget.push_back(atoi(s.c_str()));

                  auto tokens = tokenize(ib.value("Pay Limits"));
                  int i = 0;
                  for (auto &e : theEmployees)
                  {
                      if (tokens[i] == "none")
                          e.myPayLimit = INT_MAX;
                      else
                          e.myPayLimit = atoi(tokens[i].c_str());
                      i++;
                  }

                  run();

                  fm.update();
              });

    mb.append("Edit", mf);
}
main()
{

    cGUI GUI;

    return 0;
}
