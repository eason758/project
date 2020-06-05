#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>

class AI : public AIInterface
{
public:

//test
int x_test;
int y_test;
//test

    void init(bool order) override
    {//init your AI

        //test
        x_test = 0;
        y_test = 0;
        //test
        // TODO: init any way you want
    }

    void callbackReportEnemy(int x, int y) override
    {
        (void) x;
        (void) y;
        //what is this? give enermy your last step?
        //but how? its return value is void?
        //TODO: give last step
    }

    std::pair<int,int> queryWhereToPut(TA::UltraBoard) override
    {

        //the place where do decisions
        //not only need to do decsion but also need to set( void call(...) in game.h )
        //or just return the last decision pair ( auto call(...) in game.h) 
        //the proccess dont take over 1 second(1000)millisecond

        //test
        int test_x = x_test;
        int test_y = y_test;
        if(x_test>=8)
        {
            y_test++;
            x_test = 0;
        }else x_test++;
        //test

        return std::make_pair(test_x,test_y);
    }
};
