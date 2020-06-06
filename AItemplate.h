#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
using namespace TA;

class AI : public AIInterface
{
public:

//test
int x_test;
int y_test;
//test

int last_step_x;
int lasy_step_y;

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
        //let (last_step_x, last_step_y) = (x,y)
    }

    std::pair<int,int> queryWhereToPut(UltraBoard MainBoard) override
    {

        //the place where do decisions
        //not only need to do decsion but also need to set( void call(...) in game.h )
        //or just return the last decision pair ( auto call(...) in game.h) 
        //the proccess dont take over 1 second(1000)millisecond

        //test

        //random blablabla... I dont know the principle...
        std::random_device rd;
        std::default_random_engine gen = std::default_random_engine(rd());
        std::uniform_int_distribution<int> dis(0,8);
        // any way , it can generate random integer number by
        // let x = dis(gen);

        int test_x;
        int test_y;



        for(;1;){
            test_x = dis(gen);
            test_y = dis(gen);
            if(MainBoard.get(test_x,test_y) == Board::Tag::None){
                std::cout<<test_x<<" "<<test_y<<std::endl;
                return std::make_pair(test_y,test_x);
            }
        }        
//test
    }


};
