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
    std::vector<std::string> myCan;
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
    bool isCapable(const std::string &crateName)
    {
        return (
            std::find(
                myCan.begin(), myCan.end(), crateName) != myCan.end());
    }
};

struct sCrate
{
    std::string myName;
    int myBudget;
    int myPop; // number employess capable of pushing this crate
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

std::vector<sCrate> theCrates, theOriginalCrates;
std::vector<sEmployee> theEmployees;
std::vector<sAssign> theAssigns;

raven::graph::sGraphData theGraph;
std::vector<int> theFlows;

// generate example problem
void generate1()
{
    sEmployee e;
    e.myName = "Alice";
    e.myCan = {"A", "B", "C"};
    e.myPayLimit = 35;
    e.myCrateLimit = 2;
    e.myEfficiency = 3;
    e.myPay = 0;
    e.myCrate = 0;
    theEmployees.push_back(e);
    e.myName = "Bob";
    e.myCan = {"A", "D"};
    e.myPayLimit = INT_MAX;
    e.myCrateLimit = INT_MAX;
    e.myEfficiency = 2;
    theEmployees.push_back(e);

    std::vector<int> Budget = {20, 4, 20, 20, 20 };

    int index = 0;
    for (int b : Budget)
    {
        sCrate c;
        c.myName = std::string(1, (char)('A' + index));
        c.myBudget = b;
        theCrates.push_back(c);
        index++;
    }
}

void sortCratesByIncreasingPopularity()
{
    theOriginalCrates = theCrates;
    for (int c = 0; c < theCrates.size(); c++)
    {
        int popCount = 0;
        for (auto &e : theEmployees)
        {
            if (e.isCapable(theCrates[c].myName))
                popCount++;
        }
        theCrates[c].myPop = popCount;
    }

    std::sort(
        theCrates.begin(), theCrates.end(),
        [](const sCrate &a, const sCrate &b) -> bool
        {
            return (a.myPop < b.myPop);
        });

    // for (int c = 0; c < theCrates.size(); c++)
    //     std::cout << theCrates[c].myName << " " << theCrates[c].myPop << "\n";
}

raven::graph::sGraphData makeGraph()
{
    raven::graph::sGraphData gd;
    gd.g.directed();

    sortCratesByIncreasingPopularity();

    // loop over employees
    for (auto &e : theEmployees)
    {
        // connect source to each employe
        // limiting the capacity to the employees pay limit
        gd.g.add("src", e.myName);
        gd.edgeWeight.push_back(e.myPayLimit);

        // connect each employee to the crates they can push
        for (auto &c : e.myCan)
        {
            gd.g.add(e.myName, c);
            gd.edgeWeight.push_back(INT_MAX);
        }
    }

    // loop over crates
    for (int c = 0; c < theCrates.size(); c++)
    {
        // connect crates to the sink
        // limiting capacity to the budget for that crate
        gd.g.add(theCrates[c].myName, "snk");
        gd.edgeWeight.push_back(theCrates[c].myBudget);
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
    for (auto &c : theOriginalCrates)
        ss << c.myBudget << " ";
    ss << "\n=============\n";

    std::cout << "Flows\n";
    for (int e = 0; e < theGraph.g.edgeCount(); e++)
        std::cout << theGraph.g.userName(theGraph.g.src(e))
                  << " " << theGraph.g.userName(theGraph.g.dest(e))
                  << " " << theFlows[e]
                  << "\n";

    // calculate totals and display
    int totalDistance = 0;
    for (auto &e : theEmployees)
    {
        int totalPay = 0;
        for (int kc = 0; kc < theCrates.size(); kc++)
        {
            int ei = theGraph.g.find(
                e.myName,
                theCrates[kc].myName);
            if (ei > 0)
                totalPay += theFlows[ei];
        }
        ss << e.myName << " is paid " << totalPay << "\n";
        totalDistance += totalPay * e.myEfficiency;

        ss << "( ";
        for (int kc = 0; kc < theCrates.size(); kc++)
        {
            int ei = theGraph.g.find(
                e.myName,
                theCrates[kc].myName);
            if (ei > 0)
                if (theFlows[ei] > 0)
                    ss << " " << theFlows[ei]
                       << " for crate " << theCrates[kc].myName << " ";
        }
        ss << " )\n";
    }
    ss << "Total Distance " << totalDistance << "\n";

    return ss.str();
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
        int bestPop = 0;
        //std::string dropCrate;
        std::string lowestPayCrate, highestPopCrate;
        for (auto &c : e.myCan)
        {
            int ei = gd.g.find(
                e.myName,
                c);
            if (ei > 0)
            {
                int f = vEdgeFlow[ei];
                if (f > 0)
                {
                    // this employee has been paid for this crate
                    crateCount++;

                    auto it = std::find_if(
                        theCrates.begin(),theCrates.end(),
                        [&]( const sCrate& t ) -> bool
                        {
                            return t.myName == c;
                        });
                    if( it == theCrates.end() )
                        throw std::runtime_error("enforceCrateLimit error ");
                    if ( it->myPop > bestPop)
                    {
                        // this is the crate with the most alternative pushers
                        bestPop = it->myPop;
                        highestPopCrate = it->myName;
                    }
                    if( f < lowestPayment )
                    {
                        lowestPayment = f;
                        lowestPayCrate = it->myName;

                    }
                }
            }
        }

        if (crateCount <= e.myCrateLimit)
            continue; // limit not exceeded, continue to next employee

        std::cout << display() << e.myName << " exceeds crate limit\n";
        std::cout << "lowest pay " << lowestPayCrate 
            << " highest pushers " << highestPopCrate
            << "\n";
        std::cout << "\nxxxxx\n";

        gd.edgeWeight[ gd.g.find(e.myName,lowestPayCrate)] = 0;

        return false;
    }
    return true;
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
    generate1();
    run();

    menus();

    fm.move({50, 50, 1000, 500});
    fm.text("Crate Pushing");

    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            std::string d = display();
            S.text(d, {5, 5, 1000, 1000});
            std::cout << d;
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
                  ib.gridWidth(300);

                  std::string sp;
                  for (auto &c : theCrates)
                      sp += std::to_string(c.myBudget) + " ";
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

                  auto tokens = tokenize(ib.value("Budget"));
                  if (tokens.size() != theCrates.size())
                  {
                      wex::msgbox mb(
                          "Need budget for " + std::to_string(theCrates.size()) + " crates");
                      return;
                  }
                  int c = 0;
                  for (auto &s : tokenize(ib.value("Budget")))
                  {
                      theCrates[c].myBudget = atoi(s.c_str());
                      c++;
                  }

                  tokens = tokenize(ib.value("Pay Limits"));
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
