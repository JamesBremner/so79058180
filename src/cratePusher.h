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

    sEmployee(
        const std::string &name,
        const std::string &payLimit,
        const std::string &crateLimit,
        const std::string &efficiency);

    // true if employee can push crate
    bool isCapable(const std::string &crateName);

    void setCapability(std::vector<std::string> &tokens);

    static sEmployee &find(
        const std::string &name);
};

struct sCrate
{
    std::string myName;
    int myBudget;
    int myPop; // number employees capable of pushing this crate
    sCrate(
        const std::string &name,
        int budget)
        : myName(name),
          myBudget(budget)
    {
    }
};

struct sOptimizer
{
    raven::graph::sGraphData theGraph;
    std::vector<int> theFlows;
    bool fDropMostPushers;

    void run();

    int distance();
};

extern std::vector<sEmployee> theEmployees;
extern std::vector<sCrate> theCrates;
extern std::vector<sCrate> theOriginalCrates;
extern sOptimizer theOptimizer;

void generate1();
std::string display();
