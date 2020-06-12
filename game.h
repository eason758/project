#pragma once

#include <UltraOOXX/UltraBoard.h>
#include <UltraOOXX/Wrapper/AI.h>
#include <GUI/GUIInterface.h>

#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <future>
#include <type_traits>
#include <typeinfo>
#include <string>

#include <fstream>

namespace TA
{
    //(test)write in coordinate
    std::ofstream file;
   // file.open("log_test.txt",std::ios::app);   //append

    class UltraOOXX
    {
    public:
        UltraOOXX(
            std::chrono::milliseconds runtime_limit = std::chrono::milliseconds(1000)
        ):
            m_runtime_limit(runtime_limit),
            m_P1(nullptr),
            m_P2(nullptr),
            MainBoard()
        {
            gui = new ASCII;
        }

        void setPlayer1(AIInterface *ptr) { assert(checkAI(ptr)); m_P1 = ptr; }
        void setPlayer2(AIInterface *ptr) { assert(checkAI(ptr)); m_P2 = ptr; }

        void run()
        {
            gui->title();
            round = 0;
            if( !prepareState() ) return ;

            //Todo: Play Game
            //putToGui("Hello world %d", 123);//print in gui
            
            updateGuiGame();

            
            while(1){
                putToGui("Which mode do you want to play?\n");
                putToGui("1: AI vs AI         2:You vs AI\n");
                std::cin>>game_mode;
                if(game_mode == 2){
                    PLYAYERvsAI();
                    break;
                }
                else if(game_mode == 1) {
                    AIvsAI();
                    break;
                }
                else if(game_mode == 3) break;
            }



            
             

        } 

   private:
        void updateGuiGame()
        {
            gui->updateGame(MainBoard);
        }

        // 6/13 have been editted
        bool playOneRound(AIInterface *user, BoardInterface::Tag tag, AIInterface *enemy)
        {
            std::pair<int,int> pos;
            if(user != nullptr)
                pos = call(&AIInterface::queryWhereToPut, user, MainBoard);
            else
            {
                int x,y;
                std::cin>>x>>y;
                pos = std::make_pair(x,y);
            }
            

           //check whether the pos is legal
			//1. out of range
			if(pos.first>8 || pos.first<0 || pos.second>8 || pos.second <0){
				putToGui("pos out of range\n"); 
				return false;	
			}
			//2. already occupied
			if(MainBoard.get(pos.first,pos.second)!=BoardInterface::Tag::None){
				putToGui("pos already occupied\n");
				return false;
			}
			//3. whether it meet the rules (in the correct board)
			if(round>1 && !MainBoard.sub(last_coor.first%3,last_coor.second%3).full()){  
			    if((pos.first/3)!=(last_coor.first%3) || (pos.second/3)!=(last_coor.second%3)){
                    putToGui("incorrect board: (%d,%d) (%d,%d)\n",pos.first,pos.second,last_coor.first,last_coor.second);
				    return false;
                }
            }
            
			//take the move  
			MainBoard.get(pos.first,pos.second)=tag;
			
            //update the last move
            last_coor=pos;
            
            //tell the enemy its coordinate  (maybe?)
            if(enemy != nullptr)
			    enemy->callbackReportEnemy(pos.first,pos.second);   			
			return true;

        }

        //added (06/06)
        bool update_status(BoardInterface& t)
        {  //if new updates, true

        	//1. if already has a wintag, do nothing
			if(t.getWinTag()!=BoardInterface::Tag::None) return false;

        	//2. check: any winner?
			//(1,1)
			if(t.state(1,1)==BoardInterface::Tag::O || t.state(1,1)==BoardInterface::Tag::X){
				//check horizontal, vertical, diagonal
				if(t.state(1,0)==t.state(1,1) && t.state(1,0)==t.state(1,2)){
        			t.setWinTag(t.state(1,1)); return true;
        		}
        		if(t.state(0,1)==t.state(1,1) && t.state(2,1)==t.state(1,1)){
        			t.setWinTag(t.state(1,1)); return true;
				}
				if(t.state(0,0)==t.state(1,1) && t.state(0,0)==t.state(2,2)){
					t.setWinTag(t.state(1,1)); return true;
				}
				if(t.state(0,2)==t.state(1,1) && t.state(1,1)==t.state(2,0)){
					t.setWinTag(t.state(1,1)); return true;
				}
			}
			//(0,0)
        	if(t.state(0,0)==BoardInterface::Tag::O || t.state(0,0)==BoardInterface::Tag::X){
				//check horizontal, vertical (diagonal already checked)
				if(t.state(0,0)==t.state(0,1) && t.state(0,0)==t.state(0,2)){
        			t.setWinTag(t.state(0,0)); return true;
        		}
        		if(t.state(0,0)==t.state(1,0) && t.state(0,0)==t.state(2,0)){
        			t.setWinTag(t.state(0,0)); return true;
				}
			}
			//(2,2)
			if(t.state(2,2)==BoardInterface::Tag::O || t.state(2,2)==BoardInterface::Tag::X){
				//only need to check vertical
				if(t.state(0,2)==t.state(1,2) && t.state(0,2)==t.state(2,2)){
					t.setWinTag(t.state(2,2)); return true;
				}
				if(t.state(2,0)==t.state(2,1) && t.state(2,0)==t.state(2,2)){
					t.setWinTag(t.state(2,2)); return true;
				}
			}

			//3. no winners yet!
			//   check if it's a tie (full!) (Ultraboard doesn't have full() function)
			for (int i=0;i<3;++i)
            {
                for (int j=0;j<3;++j)
                {
                    if (t.state(i,j) == BoardInterface::Tag::None)
                    	return false;
                }
            }


            //4. it is a tie (full and no winners)(update to tie)
            t.setWinTag(BoardInterface::Tag::Tie);
            return true;
		}

        bool checkGameover()
        {  
            //TODO: how to check whether gameover?

            //1.update board status
            for(int i=0;i<3;i++)
            {
            	for(int j=0;j<3;j++)
            	{
            		if(update_status(MainBoard.sub(i,j)))
            		{
            			//2. if board is updated, then update the Ultraboard
            			if(update_status(MainBoard)){
                            putToGui("GameOver!\n");
            			    return true;
            			    //puts("GameOver\n");
            			}

					}
				}
			}
			//3. no updates done, game continues
            return false;
        }

        bool prepareState()
        {
            call(&AIInterface::init, m_P1, true);
            call(&AIInterface::init, m_P2, false);
            return true;
        }

        template<typename Func ,typename... Args, 
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value , int> = 0 >
        void call(Func func, AIInterface *ptr, Args... args)
        {
            std::future_status status;

            auto val = std::async(std::launch::async, func, ptr, args...);

            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )//if decision time > 1 then exit
            {
                exit(-1);
            }

            val.get();//void type

        }


        template<typename Func ,typename... Args, 
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value == false, int> = 0 >
        auto call(Func func, AIInterface *ptr, Args... args)
            -> std::invoke_result_t<Func, AIInterface, Args...>
        {
            std::future_status status;
            auto val = std::async(std::launch::async, func, ptr, args...);

            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )
            {
                exit(-1);
            }

            return val.get();//pair type
        }

        void putToGui(const char *fmt, ...)
        {
            va_list args1;
            va_start(args1, fmt);
            va_list args2;
            va_copy(args2, args1);
            std::vector<char> buf(1+std::vsnprintf(nullptr, 0, fmt, args1));
            va_end(args1);
            std::vsnprintf(buf.data(), buf.size(), fmt, args2);
            va_end(args2);

            if( buf.back() == 0 ) buf.pop_back();
            gui->appendText( std::string(buf.begin(), buf.end()) );
        }

        bool checkAI(AIInterface *ptr) 
        {
            return ptr->abi() == AI_ABI_VER;
        }


        // 6/13 move from run()
        void AIvsAI(){
            while (!checkGameover()) {   //checkGameover()   

                round++;
                AIInterface *first = nullptr;
                AIInterface *second = nullptr;
                BoardInterface::Tag tag;
                if(round%2){
                    first = m_P1;
                	second = m_P2;
                	tag = BoardInterface::Tag::O;
				}
				else{
					first = m_P2;
                	second = m_P1;
                	tag = BoardInterface::Tag::X;
				}

                if (!playOneRound(first, tag, second)) {  //one move
                    //the "first" AI made illegal move, lose the game!
                    
                    if(round%2)
                        putToGui("Player1\n");
                    else
                        putToGui("Player2\n");
                    putToGui(" lost!\n");
                    //updateGuiGame();
                    exit(-1);
                }
                updateGuiGame();
            }
            if(MainBoard.getWinTag()==BoardInterface::Tag::O){
                putToGui("O\n");
            }
            else if(MainBoard.getWinTag()==BoardInterface::Tag::X){
                putToGui("X\n");
            }
            else if(MainBoard.getWinTag()==BoardInterface::Tag::Tie){
                putToGui("Tie\n");
            }
            else{
                putToGui("None\n");
            }
            //file.close();
        }
        
        // 6/13 add
        void PLYAYERvsAI(){
            int order;
            while(1){
                putToGui("Choose your order!\n");
                putToGui("1:first   2:second\n");
                std::cin>>order;
                if(order == 1){
                    call(&AIInterface::init, m_P1, false);
                    break;
                }
                else if(order == 2){
                    call(&AIInterface::init, m_P1, true);
                    break;
                }
            }
        

            while (!checkGameover()) {   //checkGameover()   

                round++;
                AIInterface *first = nullptr;
                AIInterface *second = nullptr;
                BoardInterface::Tag tag;
                if((round+order)%2){
                    first = m_P1;
                	second = nullptr;
                	if(round%2)
                        tag = BoardInterface::Tag::O;
                    else
                        tag = BoardInterface::Tag::X;
                }
				else{
					first = nullptr;
                	second = m_P1;
                	if(round%2)
                        tag = BoardInterface::Tag::O;
                    else
                        tag = BoardInterface::Tag::X;
				}

                if (!playOneRound(first, tag, second)) {  //one move
                    //the "first" AI made illegal move, lose the game!
                    
                    if((round+order)%2)
                        putToGui("AI\n");
                    else
                        putToGui("You\n");
                    putToGui(" lost!\n");
                    //updateGuiGame();
                    exit(-1);
                }
                updateGuiGame();



            }
            if(MainBoard.getWinTag()==BoardInterface::Tag::O){
                putToGui("O\n");
            }
            else if(MainBoard.getWinTag()==BoardInterface::Tag::X){
                putToGui("X\n");
            }
            else if(MainBoard.getWinTag()==BoardInterface::Tag::Tie){
                putToGui("Tie\n");
            }
            else{
                putToGui("None\n");
            }
            //file.close();
        }


        // 6/13 add
        int game_mode;
        
        int m_size;
        
        std::vector<int> m_ship_size;
        std::chrono::milliseconds m_runtime_limit;

        AIInterface *m_P1;
        AIInterface *m_P2;
        GUIInterface *gui;

        UltraBoard MainBoard;
        
        //record the last move
        std::pair<int,int> last_coor;
        int round;
    };
}
