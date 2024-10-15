#include <vector>
#include <stdexcept>

#include "cssex.h"

bool cSSex::nextTestValues(
    std::vector<int> &test,
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

void cSSex::checkFunctionValue()
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

void cSSex::search(int count, int max)
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
    if (!myVarBestVals.size())
        throw std::runtime_error(
            "cSex::search low rez failed to find feasible solution");

    // search local space around low rez opt at high rez
    int tvMax = 2 * (myRez - 1);
    int tvd = tvMax / 2;
    myRez = 1;
    auto start = myVarBestVals;
    std::vector<int> test(count, 0);
    while (true)
    {
        if (!nextTestValues(test, tvMax))
            break;
        for (int i = 0; i < count; i++)
            myVarTestVals[i] = start[i] + test[i] - tvd;

        copyTestVals();
        checkFunctionValue();
    }

    copyOptVals();
}
