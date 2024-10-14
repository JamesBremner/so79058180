#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

typedef std::vector<std::vector<int>> vPec_t;

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

std::vector<sEmployee> theEmployees;
std::vector<int> theBudgets;

vPec_t vPec;

int Pec(int e, int c)
{
    return vPec[e][c];
}
int f(int e)
{
    return theEmployees[e].myEfficiency;
}

bool isWithinBudgetLimit(int c)
{
    int b = 0;
    for (int e = 0; e < theEmployees.size(); e++)
    {
        b += Pec(e, c);
    }
    return b <= theBudgets[c];
}

bool isWithinPayLimit(int e)
{
    int p = 0;
    for (int c = 0; c < theBudgets.size(); c++)
        p += Pec(e, c);
    return p < theEmployees[e].myPayLimit;
}

bool isFeasible()
{
    for (int e = 0; e < theEmployees.size(); e++)
    {
        if (!isWithinPayLimit(e))
            return false;
        for (int c = 0; c < theBudgets.size(); c++)
        {
            if (Pec(e, c) > 0)
                if (!theEmployees[e].isCapable(c))
                    return false;
            if (!isWithinBudgetLimit(c))
                return false;
        }
    }
    return true;
}

int O()
{
    int o = 0;
    for (int e = 0; e < theEmployees.size(); e++)
        for (int c = 0; c < theBudgets.size(); c++)
        {
            o += Pec(e, c) * f(e);
        }
    return o;
}

bool inc(std::vector<int> &vvv, int base, int res)
{
    int k = 0;
    while (true)
    {
        vvv[k] += res;
        if (vvv[k] <= base)
            return true;
        vvv[k] = 0;
        k++;
        if (k == vvv.size())
            return false;
    }
}

void testInc()
{
    std::vector<int> vvv(8);
    while (inc(vvv, 20, 3))
        std::cout << vvv[0] << " " << vvv[1] << " " << vvv[2] << "\n";
}

void display()
{
    std::stringstream ss;
    for (int e = 0; e < theEmployees.size(); e++)
    {
        ss << "Employee " << e;
        ss << " ( ";
        for (int c = 0; c < theBudgets.size(); c++)
        {
            int f = Pec(e, c);
            if (f > 0)
                ss << " " << f << " for crate " << c << " ";
        }
        ss << " )\n";
    }
    ss << "total Distance " << O() << "\n";

    std::cout << ss.str();
}

void Search()
{
    // construct test variables
    vPec_t twoDimVector(
        theEmployees.size(),
        std::vector<int>(theBudgets.size(), 0));
    vPec = twoDimVector;
    auto vPecOpt = twoDimVector;

    // optimum value so far
    int o = 0;

    // max variable range
    const int maxPay = 20;

    // 1D variable values
    int soldim = theEmployees.size() * theBudgets.size();
    std::vector<int> vsol(soldim);

    // while more variable values to test
    while (inc(vsol, maxPay, 3))
    {
        // copy variable test values
        int *psol = (int *)vsol.data();
        for (int e = 0; e < theEmployees.size(); e++)
            for (int c = 0; c < theBudgets.size(); c++)
                vPec[e][c] = *psol++;

        // check feasability
        if (!isFeasible())
            continue;
        
        // calculate value
        int v = O();

        // check for new optimum
        if (v <= o)
            continue;

        // save new optimum
        o = v;
        vPecOpt = vPec;
    }

    // return optimum found
    vPec = vPecOpt;
}

main()
{

    sEmployee alice(35, 2, 3, {0, 1, 2});
    sEmployee bob(INT_MAX, INT_MAX, 2, {0, 3});
    theEmployees.push_back(alice);
    theEmployees.push_back(bob);
    theBudgets = std::vector<int>(4, 20);

    Search();
    display();
}
