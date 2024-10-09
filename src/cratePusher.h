#include <string>
#include <sstream>
#include <vector>
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
    bool isCapable(const std::string &crateName);
};

struct sCrate
{
    std::string myName;
    int myBudget;
    int myPop; // number employess capable of pushing this crate
};

struct sOptimizer
{
    raven::graph::sGraphData theGraph;
    std::vector<int> theFlows;
    bool fDropMostPushers;
};

extern std::vector<sEmployee> theEmployees;
extern std::vector<sCrate> theCrates;
extern sOptimizer theOptimizer;

void generate1();
std::string display();
void run();