#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "cRunWatch.h"
#include "cssex.h"


struct sEmployee
{
    // configuration
    std::string myName;

    int myPayLimit;
    int myCrateLimit;
    int myEfficiency;
    std::vector<int> myCapable;

    sEmployee(
        int pay,
        int crate,
        int eff,
        const std::vector<int> caps)
        : myPayLimit(pay),
          myCrateLimit(crate),
          myEfficiency(eff),
          myCapable(caps)
    {
    }
    bool isCapable(int c)
    {
        return (
            std::find(
                myCapable.begin(), myCapable.end(), c) != myCapable.end());
    }
};

typedef std::vector<std::vector<int>> vPec_t;
class cCratePusher : public cSSex
{
public:
cCratePusher()
{
     regObjectiveFunction( [this]()->int{ return objective();} );
     regFeasibleFunction( [this]()->bool{ return isFeasible();} );
     regCopyTestFunction( [this](int*p){ copyTest(p);} );
}
    void add(const sEmployee &e)
    {
        myEmployees.push_back(e);
    }
    void setCrateBudget(std::vector<int> vb)
    {
        myBudgets = vb;
    }

    bool isFeasible();
    void copyTest(int *p);
    int objective() const;            // funtion to be optimized
    void constructVariables();

    // Payment to employee for crate
    int Pec(int e, int c) const
    {
        return vPec[e][c];
    }
    // Employee efficiency
    int f(int e) const
    {
        return myEmployees[e].myEfficiency;
    }

    void display();

private:
    std::vector<sEmployee> myEmployees;
    std::vector<int> myBudgets;

    vPec_t vPec;

    bool isWithinBudgetLimit(int c);
    bool isWithinPayLimit(int e);
};


bool cCratePusher::isWithinBudgetLimit(int c)
{
    int b = 0;
    for (int e = 0; e < myEmployees.size(); e++)
    {
        b += Pec(e, c);
    }
    return b <= myBudgets[c];
}

bool cCratePusher::isWithinPayLimit(int e)
{
    int p = 0;
    for (int c = 0; c < myBudgets.size(); c++)
        p += Pec(e, c);
    return p <= myEmployees[e].myPayLimit;
}

bool cCratePusher::isFeasible()
{
    for (int e = 0; e < myEmployees.size(); e++)
    {
        if (!isWithinPayLimit(e))
            return false;
        for (int c = 0; c < myBudgets.size(); c++)
        {
            if (Pec(e, c) < 0)
                return false;
            if (Pec(e, c) != 0)
                if (!myEmployees[e].isCapable(c))
                    return false;
            if (!isWithinBudgetLimit(c))
                return false;
        }
    }
    return true;
}

int cCratePusher::objective() const
{
    int o = 0;
    for (int e = 0; e < myEmployees.size(); e++)
        for (int c = 0; c < myBudgets.size(); c++)
        {
            o += Pec(e, c) * f(e);
        }
    return o;
}

void cCratePusher::constructVariables()
{
    vPec_t twoDimVector(
        myEmployees.size(),
        std::vector<int>(myBudgets.size(), 0));
    vPec = twoDimVector;
    SolutionSpace(
        myEmployees.size() * myBudgets.size(),
        20);
}

void cCratePusher::display()
{
    std::stringstream ss;
    for (int e = 0; e < myEmployees.size(); e++)
    {
        ss << "Employee " << e;
        ss << " ( ";
        for (int c = 0; c < myBudgets.size(); c++)
        {
            int f = Pec(e, c);
            if (f > 0)
                ss << " " << f << " for crate " << c << " ";
        }
        ss << " )\n";
    }
    ss << "total Distance " << objective()
       << "\n";

    std::cout << ss.str();
}

void cCratePusher::copyTest(int *psol)
{
    for (int e = 0; e < myEmployees.size(); e++)
        for (int c = 0; c < myBudgets.size(); c++)
            vPec[e][c] = *psol++;
}

main()
{
    raven::set::cRunWatch::Start();

    cCratePusher cratePusher;

    // specify employees
    sEmployee alice(35, 2, 3, {0, 1, 2});
    sEmployee bob(INT_MAX, INT_MAX, 2, {0, 3});
    cratePusher.add(alice);
    cratePusher.add(bob);

    // specify crate budgets
    cratePusher.setCrateBudget({20, 20, 20, 20});

    cratePusher.constructVariables();

    // search solution space for optimum
    cratePusher.search();

    //cratePusher.anneal();

    //cratePusher.greedy();

    // display optimum on console
    cratePusher.display();

    raven::set::cRunWatch::Report();
}
