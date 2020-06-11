#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

class AI : public AIInterface
{
private:

        
        bool first_player;               
        bool first_player_first_step;    // indicate this is first step
        std::pair<int,int> target_board[2]; //which board to force enemy to set
        bool i; // for target board
        TA::BoardInterface::Tag mytag;



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

        if(inputMAP.sub(offset_x,offset_y).full()){//full board
            myRange.x1 = 0;
            myRange.y1 = 0;
            myRange.x2 = 8;
            myRange.y2 = 8;
            return;
        }
        else{
            return;
        }
    }




    // 2 -> 0 ; 0->2 ; 1->1
    int opposite(int pos){
        if(pos == 2) return 0;
        else if(pos == 0) return 2;
        else return 1;
    }

    bool Ultra_settable(int x,int y,TA::UltraBoard board){
        return board.get(x,y) == TA::Board::Tag::None;
    }
    bool settable(int x,int y,TA::Board board){
        return board.get(x,y) == TA::Board::Tag::None;
    }
        //if got the chance ,make a line ,whatever the situation or the cost is
    std::pair<int,int> make_line_step(TA::Board &board,TA::UltraBoard MainBoard){

        	//if already has a wintag, det any place
			if(board.getWinTag()!=TA::BoardInterface::Tag::None){
                
                return random_set(MainBoard);

            }


			if(board.state(1,1)== mytag){

				if(board.state(0,0)== mytag && settable(2,2,board)){
        			return std::make_pair(2,2);
        		}
                if(board.state(1,0)== mytag && settable(1,2,board)){
        			return std::make_pair(1,2);
        		}
        		if(board.state(2,0)== mytag && settable(0,2,board)){
        			return std::make_pair(0,2);
        		}
                if(board.state(2,1)== mytag && settable(0,1,board)){
        			return std::make_pair(0,1);
        		}
                if(board.state(2,2)== mytag && settable(0,0,board)){
        			return std::make_pair(0,0);
        		}
                if(board.state(1,2)== mytag && settable(1,0,board)){
        			return std::make_pair(1,0);
        		}
                if(board.state(0,2)== mytag && settable(2,0,board)){
        			return std::make_pair(2,0);
        		}
                if(board.state(0,1)== mytag && settable(2,1,board)){
        			return std::make_pair(2,1);
        		}
			}
            
            if(board.state(0,0)== mytag){

                if(board.state(0,1)== mytag && settable(0,2,board)){
        			return std::make_pair(0,2);
        		}
                if(board.state(0,2)== mytag && settable(0,1,board)){
        			return std::make_pair(0,1);
        		}
                if(board.state(1,0)== mytag && settable(2,0,board)){
        			return std::make_pair(2,0);
        		}
                if(board.state(2,0)== mytag && settable(1,0,board)){
        			return std::make_pair(1,0);
        		}
			}

            if(board.state(2,2)== mytag){

                if(board.state(0,2)== mytag && settable(1,2,board)){
        			return std::make_pair(1,2);
        		}
                if(board.state(1,2)== mytag && settable(0,2,board)){
        			return std::make_pair(0,2);
        		}
                if(board.state(2,0)== mytag && settable(2,1,board)){
        			return std::make_pair(2,1);
        		}
                if(board.state(2,1)== mytag && settable(2,0,board)){
        			return std::make_pair(2,0);
        		}
			}

            if(board.state(2,0)== mytag){

                if(board.state(1,0)== mytag && settable(0,0,board)){
        			return std::make_pair(0,0);
        		}
                if(board.state(2,1)== mytag && settable(2,2,board)){
        			return std::make_pair(2,2);
        		}
			}

            if(board.state(0,2)== mytag){

                if(board.state(0,1)== mytag && settable(0,0,board)){
                    return std::make_pair(0,0);
        		}
                if(board.state(1,2)== mytag && settable(2,2,board)){
                    return std::make_pair(2,2);
        		}
			}
            return random_set(MainBoard);

    }

        //set randomly and legel
    std::pair<int,int>random_set(TA::UltraBoard MainBoard){
        
        int result_x;
        int result_y;

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
            if(Ultra_settable(result_x,result_y,MainBoard)){
                return std::make_pair(result_x,result_y);
            }
        }
    }


public:

    void init(bool order) override
    {
        if(order){
            i = false;
            mytag = TA::Board::Tag::O;
            first_player_first_step = true;
            first_player = true;
            
        }
        else{
            mytag = TA::Board::Tag::X;
            first_player_first_step = false;
            first_player = false;
        }

    }

    void callbackReportEnemy(int x, int y) override
    {
        last_x = x;
        last_y = y;
        //get enemy last step
    }

    std::pair<int,int> queryWhereToPut(TA::UltraBoard MainBoard) override
    {

        //int result_x;
        //int result_y;

        if(first_player){


            if(first_player_first_step){   //if first player first step ,then set in the middle
                first_player_first_step = false;
                return std::make_pair(4,4);
            }
        
            setRange(MainBoard);

            //step 1
            if(!MainBoard.sub(1,1).full()){
                return std::make_pair(myRange.x1+1,myRange.y1+1);
            }
            else{

            //target_board init
                if(!i){
                    i = true;
                    target_board[0].first = myRange.x1/3;
                    target_board[0].second = myRange.y1/3;
                    target_board[1].first = opposite(target_board[0].first);
                    target_board[1].second = opposite(target_board[0].second);
                    return std::make_pair(myRange.x1+myRange.x1/3,myRange.y1+myRange.y1/3);
                }
            
            // step 2
                else if(!MainBoard.sub(target_board[0].first,target_board[0].second).full()){
                    if(last_x%3 == 1 && last_y%3 == 1){ //enemy set middle
                        if(Ultra_settable(opposite(last_x/3)*3+target_board[0].first , opposite(last_y/3)*3+target_board[0].second,MainBoard) )
                            return std::make_pair(opposite(last_x/3)*3+target_board[0].first , opposite(last_y/3)*3+target_board[0].second);
                        else{
                            std::pair<int,int> tmp = make_line_step(MainBoard.sub(opposite(last_x/3),opposite(last_y/3)) ,MainBoard);
                            return std::make_pair(tmp.first+ 3*opposite(last_x/3) , tmp.second+3*opposite(last_y/3));
                        }
                    }
                    else{
                        if(Ultra_settable(myRange.x1+target_board[0].first,myRange.y1+target_board[0].second,MainBoard))
                            return std::make_pair(myRange.x1+target_board[0].first , myRange.y1+target_board[0].second);
                        else{
                            std::pair<int,int> tmp = make_line_step(MainBoard.sub(myRange.x1/3,myRange.y1/3) ,MainBoard);
                            return std::make_pair( tmp.first+myRange.x1 , tmp.second+myRange.y1 );
                        }
                    }
            
                }
            // step 3
                else{
                    if(myRange.x1 == 0 && myRange.x2 == 8){ //in the middle
                        if(Ultra_settable(target_board[1].first*4 ,target_board[1].second*4 ,MainBoard))
                            return std::make_pair(target_board[1].first*4 ,target_board[1].second*4);
                        else{
                            for(int i = 0; i < 3 ; i++)
                            for(int j = 0; j < 3 ; j++){

                                if(Ultra_settable(i*3+target_board[1].first ,j*3+target_board[1].second ,MainBoard))
                                {
                                    return std::make_pair(i*3+target_board[1].first ,j*3+target_board[1].second);
                                }
                            
                            }
                        }   

                    }
                    else
                        return std::make_pair(myRange.x1+target_board[1].first , myRange.y1+target_board[1].second);
                }
            }
        }

        // second player
        setRange(MainBoard);
        return random_set(MainBoard);


//below is the implement of random_set(MainBoard)
/*
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
            if(settable(result_x,result_y,MainBoard)){
                return std::make_pair(result_x,result_y);
            }
        }        
*/

    }
};
