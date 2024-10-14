#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

/// @brief Integer solution space explorer

class cSex
{

public:
    /// @brief search through variable test ranges
    /// @param count number of variables
    /// @param max maximum of variable range
    /// @return true if more to come, false if all done

    bool search(int count, int max);

    virtual void copy(int *p) = 0;
    virtual bool isFeasible() = 0;

    // caclulate value of funtion to be optimized with these variable testvalues
    virtual int optFunVal() = 0;

    int getOptimumValue() const
    {
        return myOptValue;
    }
    void copyTestVals()
    {
        copy(&myVarTestVals[0]);
    }
    void copyOptVals()
    {
        copy(&myVarBestVals[0]);
    }

protected:
    std::vector<int> myVarTestVals;
    std::vector<int> myVarBestVals;
    int myRez;
    int myOptValue;

private:
    bool nextTestValues(std::vector<int>& test, int max);
    void checkFunctionValue();
};

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
class cCratePusher : public cSex
{
public:
    void add(const sEmployee &e)
    {
        myEmployees.push_back(e);
    }
    void setCrateBudget(std::vector<int> vb)
    {
        myBudgets = vb;
    }

    void search();
    bool isFeasible();
    void copy(int *p);
    int optFunVal();
    void constructVariables();

    // Payment to employee for crate
    int Pec(int e, int c)
    {
        return vPec[e][c];
    }
    // Employee efficiency
    int f(int e)
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

bool cSex::nextTestValues(
    std::vector<int>& test,
    int max)
{
    int k = 0;
    while (true)
    {
        int *p = &test[k];
        *p += myRez;
        if (*p <= max)
            break;
        *p = 0;
        k++;
        if (k == myVarTestVals.size())
        {
            // search is complete
            return false;
        }
    }

    return true;
}

void cSex::checkFunctionValue()
{
    if (!isFeasible())
        return;
    // check for improved function value
    int o = optFunVal();
    if (o > myOptValue)
    {
        myOptValue = o;
        myVarBestVals = myVarTestVals;
    }
}

bool cSex::search(int count, int max)
{
    // search space at low rez
    myOptValue = 0;
    myVarTestVals.clear();
    myVarTestVals.resize(count, 0);
    myRez = max / 5;

    while (true)
    {
        // next test value set
        if (!nextTestValues(myVarTestVals, max))
        {
            // search is complete
            copyOptVals();
            break;
        }
        copyTestVals();
        checkFunctionValue();
    }

    // search local space around low rez opt at high rez

    std::vector<int> test(count,0);
    myRez = 1;
    auto start = myVarBestVals;
    while (true)
    {
        if (!nextTestValues(test,4))
        {
            copyOptVals();
            return true;
        }
        for (int i = 0; i < count; i++)
        {
            myVarTestVals[i] = start[i] + test[i] - 2;
        }
        copyTestVals();
        checkFunctionValue();
    }

    return true;
}

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
    return p < myEmployees[e].myPayLimit;
}

bool cCratePusher::isFeasible()
{
    for (int e = 0; e < myEmployees.size(); e++)
    {
        if (!isWithinPayLimit(e))
            return false;
        for (int c = 0; c < myBudgets.size(); c++)
        {
            if (Pec(e, c) > 0)
                if (!myEmployees[e].isCapable(c))
                    return false;
            if (!isWithinBudgetLimit(c))
                return false;
        }
    }
    return true;
}

int cCratePusher::optFunVal()
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
    ss << "total Distance " << optFunVal()
       << "\n";

    std::cout << ss.str();
}

void cCratePusher::copy(int *psol)
{
    for (int e = 0; e < myEmployees.size(); e++)
        for (int c = 0; c < myBudgets.size(); c++)
            vPec[e][c] = *psol++;
}

void cCratePusher::search()
{
    constructVariables();
    cSex::search(
        myEmployees.size() * myBudgets.size(),
        20);
}

main()
{
    cCratePusher cratePusher;

    // specify employees
    sEmployee alice(35, 2, 3, {0, 1, 2});
    sEmployee bob(INT_MAX, INT_MAX, 2, {0, 3});
    cratePusher.add(alice);
    cratePusher.add(bob);

    // specify crate budgets
    cratePusher.setCrateBudget({20, 20, 20, 20});

    // search solution space for optimum
    cratePusher.search();

    // display optimum on console
    cratePusher.display();
}
