
#include <iostream>
#include <algorithm>
#include "cratePusher.h"
#include "cGUI.h"

sEmployee::sEmployee(
    const std::string &name,
    const std::string &payLimit,
    const std::string &crateLimit,
    const std::string &efficiency)
    : myName(name),
      myPayLimit(atoi(payLimit.c_str())),
      myCrateLimit(atoi(crateLimit.c_str())),
      myEfficiency(atoi(efficiency.c_str()))
{
    if (payLimit == "none")
        myPayLimit = INT_MAX;
    if (crateLimit == "none")
        myCrateLimit = INT_MAX;
}
bool sEmployee::isCapable(const std::string &crateName)
{
    return (
        std::find(
            myCan.begin(), myCan.end(), crateName) != myCan.end());
}

sEmployee &sEmployee::find(
    const std::string &name)
{
    auto it = std::find_if(
        theEmployees.begin(), theEmployees.end(),
        [&](const sEmployee &e) -> bool
        {
            return (e.myName == name);
        });
    if (it == theEmployees.end())
    {
        static sEmployee null;
        return null;
    }
    return *it;
}

void sEmployee::setCapability(std::vector<std::string> &tokens)
{
    myCan.clear();
    for (int i = 2; i < tokens.size(); i++)
        myCan.push_back(tokens[i]);
}
sCrate &findCrate(const std::string &name)
{
    auto it = std::find_if(
        theCrates.begin(), theCrates.end(),
        [&](const sCrate &t) -> bool
        {
            return t.myName == name;
        });
    if (it == theCrates.end())
        throw std::runtime_error("Cannot find crate " + name);
    return *it;
}

std::vector<sCrate> theCrates, theOriginalCrates;
std::vector<sEmployee> theEmployees;

sOptimizer theOptimizer;

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

    std::vector<int> Budget = {20, 6, 20, 20, 20};
    // std::vector<int> Budget = {500, 0, 0, 0, 0};

    theCrates.clear();
    int index = 0;
    for (int b : Budget)
    {
        theCrates.emplace_back(
            std::string(1, (char)('A' + index)),
            b);
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

    // ss << "\nCrate Enforcer drops crate with ";
    // if (theOptimizer.fDropMostPushers)
    //     ss << "most alternative pushers";
    // else
    //     ss << "least payment";

    ss << "\n=============\n";

    std::cout << "Flows\n";
    for (int e = 0; e < theOptimizer.theGraph.g.edgeCount(); e++)
        std::cout << theOptimizer.theGraph.g.userName(theOptimizer.theGraph.g.src(e))
                  << " " << theOptimizer.theGraph.g.userName(theOptimizer.theGraph.g.dest(e))
                  << " " << theOptimizer.theFlows[e]
                  << "\n";

    // calculate totals and display
    for (auto &e : theEmployees)
    {
        int totalPay = 0;
        for (auto& crate : theCrates)
        {
            int f = theOptimizer.flow( e.myName, crate.myName );
            if( f > 0 )
                totalPay += f;
        }
        ss << e.myName << " is paid " << totalPay << "\n";

        ss << "( ";
        for ( auto& crate : theCrates )
        {
            int f = theOptimizer.flow( e.myName, crate.myName );
            if( f > 0 )
                ss << " " << f << " for crate " << crate.myName << " ";
        }
        ss << " )\n";
    }
    ss << "Total Distance " << theOptimizer.distance() << "\n";

    return ss.str();
}

bool enforceCrateLimit()
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
        int lowestBudget = INT_MAX;
        std::string lowestPayCrate, highestPopCrate, lowestBudgetCrate;
        int mostPopBudget;
        for (auto &c : e.myCan)
        {
            int ei = theOptimizer.theGraph.g.find(
                e.myName,
                c);
            if (ei > 0)
            {
                int f = theOptimizer.theFlows[ei];
                if (f > 0)
                {
                    // this employee has been paid for this crate
                    crateCount++;

                    auto &crate = findCrate(c);
                    if (crate.myPop > bestPop)
                    {
                        // this is the crate with the most alternative pushers
                        bestPop = crate.myPop;
                        highestPopCrate = crate.myName;
                        mostPopBudget = crate.myBudget;
                    }
                    if (f < lowestPayment)
                    {
                        // this is the crate with the lowest payment from this employee
                        lowestPayment = f;
                        lowestPayCrate = crate.myName;
                    }
                    if (crate.myBudget < lowestBudget)
                    {
                        lowestBudget = crate.myBudget;
                        lowestBudgetCrate = crate.myName;
                    }
                }
            }
        }

        if (crateCount <= e.myCrateLimit)
            continue; // limit not exceeded, continue to next employee

        /*
        drop the crate that has the lowest budget.
        The crate with the most alternative pushers
        should only be dropped when
        it has a budget no smaller than the other crates the employee is pushing.
        */

        std::string dropCrate;
        // if (mostPopBudget >= lowestBudget)
        //     dropCrate = lowestBudgetCrate;
        // else
        // dropCrate = highestPopCrate;

        if (theOptimizer.fDropMostPushers)
            dropCrate = highestPopCrate;
        else
            dropCrate = lowestBudgetCrate;

        std::cout << display() << e.myName << " exceeds crate limit\n";
        std::cout << "lowest pay " << lowestPayCrate
                  << " highest pushers " << highestPopCrate
                  << "\n";
        std::cout << "dropping crate " << dropCrate << "\n";
        std::cout << "\nxxxxx\n";

        // zero capacity from employee to dropped crate
        theOptimizer.theGraph.edgeWeight[theOptimizer.theGraph.g.find(e.myName, dropCrate)] = 0;

        return false;
    }
    return true;
}

int sOptimizer::flow(
    const std::string &employeeName,
    const std::string &crateName) const
{
    int ei = theGraph.g.find(employeeName, crateName);
    if (ei >= 0)
        return theFlows[ei];
    return 0;
}

int sOptimizer::distance()
{
    int totalDistance = 0;
    for (auto &e : theEmployees)
    {
        int totalPay = 0;
        for (auto &c : theCrates)
        {
            totalPay += flow(e.myName, c.myName);
        }
        totalDistance += totalPay * e.myEfficiency;
    }
    return totalDistance;
}

void sOptimizer::run()
{
    // make the flow graph
    theGraph = makeGraph();
    auto originalGraph = theGraph;

    // run maxflow algorithm
    theFlows = maxFlow(theGraph);

    // enforce crate limit
    // by dropping crate with most alternative pushers
    fDropMostPushers = false;
    if (enforceCrateLimit())
        return; // no crate limit exceeded
    theFlows = maxFlow(theGraph);
    int d1 = distance();
    auto flows = theFlows;

    // restore flow graph
    theGraph = originalGraph;
    theFlows = maxFlow(theGraph);

    // enforce crate limit
    // by dropping crate with smallest payment
    fDropMostPushers = true;
    enforceCrateLimit();
    theFlows = maxFlow(theGraph);
    int d2 = distance();

    // return best result
    if (d2 > d1)
        return;
    theFlows = flows;
    return;
}

main()
{
    cGUI GUI;

    return 0;
}
