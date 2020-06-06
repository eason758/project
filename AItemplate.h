#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

class AI : public AIInterface
{
private:
    friend class Range;
    int last_x, last_y; //opponent's previous placement in 9*9 /*updated by enemy's playOneRound()*/
    class Range
    {
        public:
        int x1, y1, x2, y2; // can place in (x1,y1) to (x2,y2)
    };
    Range myRange;                          // record current legal step range through last_x_y
    
    void setRange(TA::UltraBoard &inputMAP)
        {
        int offset_x = last_x % 3,offset_y = last_y % 3;
        myRange.x1 = offset_x * 3;
        myRange.y1 = offset_y * 3;
        myRange.x2 = myRange.x1 + 2;
        myRange.y2 = myRange.y1 + 2;

        for (int i = myRange.x1; i <= myRange.x2; i++)
            for (int j = myRange.y1; j <= myRange.y2; j++)
            {
                if (inputMAP.get(i, j) == TA::BoardInterface::Tag::None)
                    return;//not a full board
            }
        //full board
        myRange.x1 = 0;
        myRange.y1 = 0;
        myRange.x2 = 8;
        myRange.y2 = 8;
        return;
    }


public:
    void init(bool order) override
    {
        // any way
    }

    void callbackReportEnemy(int x, int y) override
    {
        last_x = x;
        last_y = y;
        // give last step
    }

    std::pair<int,int> queryWhereToPut(TA::UltraBoard MainBoard) override
    {
        int result_x;
        int result_y;
        setRange(MainBoard);

        //random blablabla... I dont know the principle...
        std::random_device rd;
        std::default_random_engine gen = std::default_random_engine(rd());
        std::uniform_int_distribution<int> dis_x(myRange.x1,myRange.x2);
        std::uniform_int_distribution<int> dis_y(myRange.y1,myRange.y2);
        
        // any way , it can generate random integer number by
        // let x = dis(gen);




        for(;1;){
            result_x = dis_x(gen);
            result_y = dis_y(gen);
            if(MainBoard.get(result_x,result_y) == TA::Board::Tag::None){
                return std::make_pair(result_x,result_y);
            }
        }        

    }
};
