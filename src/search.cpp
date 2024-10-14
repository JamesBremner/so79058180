#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

/// @brief Integer solution space explorer

class cSex
{

public:
    /// @brief set next variable test values
    /// @param count nunber of variables
    /// @param max maximum of variable range
    /// @return true if more to come, false if all done

    bool inc(int count, int max);

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
    void setVarCount(int count)
    {
        myOptValue = 0;
        if (myVarTestVals.size())
            return;
        myVarTestVals.clear();
        myVarTestVals.resize(count);
    }

    void setRez(int max)
    {
        if (myRez)
            return;
        myRez = max / 4;
    }
};

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

class cCratePusher : public cSex
{
public:
    bool isFeasible();
    void copy(int *p);
    int optFunVal();
    void constructVariables();
};

cCratePusher cratePusher;

bool cSex::inc(int count, int max)
{
    setVarCount(count);
    setRez(max);

    while (true)
    {
        do
        {
            int k = 0;
            while (true)
            {
                int *p = &myVarTestVals[k];
                *p += myRez;
                if (*p <= max)
                {
                    copyTestVals();
                    break;
                }
                *p = 0;
                k++;
                if (k == myVarTestVals.size())
                {
                    copyOptVals();
                    return false;
                }
            }
        } while (!isFeasible());

        // check for improved function value
        int o = optFunVal();
        if (o > myOptValue)
        {
            myOptValue = o;
            myVarBestVals = myVarTestVals;
        }
    }

    return true;
}

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

bool cCratePusher::isFeasible()
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

int cCratePusher::optFunVal()
{
    int o = 0;
    for (int e = 0; e < theEmployees.size(); e++)
        for (int c = 0; c < theBudgets.size(); c++)
        {
            o += Pec(e, c) * f(e);
        }
    return o;
}

void cCratePusher::constructVariables()
{
    vPec_t twoDimVector(
        theEmployees.size(),
        std::vector<int>(theBudgets.size(), 0));
    vPec = twoDimVector;
    auto vPecOpt = twoDimVector;
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
    ss << "total Distance " << cratePusher.optFunVal()
       << "\n";

    std::cout << ss.str();
}

void cCratePusher::copy(int *psol)
{
    for (int e = 0; e < theEmployees.size(); e++)
        for (int c = 0; c < theBudgets.size(); c++)
            vPec[e][c] = *psol++;
}

main()
{

    sEmployee alice(35, 2, 3, {0, 1, 2});
    sEmployee bob(INT_MAX, INT_MAX, 2, {0, 3});
    theEmployees.push_back(alice);
    theEmployees.push_back(bob);
    theBudgets = std::vector<int>(4, 20);
    cratePusher.constructVariables();

    cratePusher.inc(
        theEmployees.size() * theBudgets.size(),
        20);

    display();
}
