#include <fstream>
#include "cratePusher.h"
#include "cGUI.h"
#include <inputbox.h>

cGUI::cGUI()
    : fm(wex::maker::make())
{
    generate1();
    theOptimizer.run();

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

void readfile(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        throw std::runtime_error(
            "Cannot open " + filename);

    theCrates.clear();
    theEmployees.clear();

    std::string line;
    while (getline(ifs, line))
    {
        auto tokens = tokenize(line);
        switch (line[0])
        {

        case 'e':
            if (tokens.size() != 5)
            {
                std::cout << line << "\n";
                throw std::runtime_error(
                    "Bad format");
            }
            theEmployees.emplace_back(
                tokens[1], tokens[2], tokens[3], tokens[4]);
            break;

        case 'c':
            if (tokens.size() != 3)
            {
                std::cout << line << "\n";
                throw std::runtime_error(
                    "Bad format");
            }
            theCrates.emplace_back(
                tokens[1],
                atoi(tokens[2].c_str()));
            break;

        case 'p':
            if (tokens.size() < 3)
            {
                std::cout << line << "\n";
                throw std::runtime_error(
                    "Bad format");
            }
            auto &e = sEmployee::find(tokens[1]);
            if (e.myEfficiency == INT_MAX)
            {
                std::cout << line << "\n";
                throw std::runtime_error(
                    "Bad employee name");
            }
            e.setCapability(tokens);
            break;
        }
    }
}

void cGUI::menus()
{
    wex::menubar mb(fm);

    wex::menu mf(fm);
    wex::menu mp(fm);

    mf.append("Open",
              [&](const std::string &title)
              {
                  // prompt for file to open
                  wex::filebox fb(fm);
                  auto paths = fb.open();
                  if (paths.empty())
                      return;
                  fm.text(paths);
                  readfile(paths);
                  theOptimizer.run();
                  fm.update();
              });

    mp.append("Parameters",
              [&](const std::string &title)
              {
                  // restore original crate order
                  theCrates = theOriginalCrates;

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

                  theOptimizer.run();

                  fm.update();
              });

    // static wex::menu malgo(fm);
    // malgo.append("Lowest Payment",
    //              [&](const std::string &title)
    //              {
    //                  theOptimizer.fDropMostPushers = false;
    //                  malgo.check(0);
    //                  malgo.check(1, false);
    //                  run();
    //                  fm.update();
    //              });
    // malgo.append("Most alternatives",
    //              [&](const std::string &title)
    //              {
    //                  theOptimizer.fDropMostPushers = true;
    //                  malgo.check(0, false);
    //                  malgo.check(1, true);
    //                  run();
    //                  fm.update();
    //              });
    // mf.append("Crate Limit Enforcer", malgo);

    // if (theOptimizer.fDropMostPushers)
    //     malgo.check(1);
    // else
    //     malgo.check(0);

    mb.append("File", mf);
    mb.append("Edit", mp);
}
