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

namespace TA
{
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
            int round = 0;
            if( !prepareState() ) return ;

            //Todo: Play Game
            putToGui("Hello world %d\n", 123);//print in gui

            updateGuiGame();
    
                //test
                MainBoard.get(1,1) = BoardInterface::Tag::X ;
                //MainBoard.get(2,2) = BoardInterface::Tag::O ;
                //MainBoard.get(3,3) = BoardInterface::Tag::X ;
                //MainBoard.get(4,4) = BoardInterface::Tag::X ;
                //MainBoard.get(0,0) = BoardInterface::Tag::O ;
                //MainBoard.get(5,6) = BoardInterface::Tag::X ;
                //MainBoard.get(2,8) = BoardInterface::Tag::X ;
                //updateGuiGame();
                //MainBoard.reset();
                //test

            int x = 81;
            while (x--) {   //checkGameover()   
                    
                round++;

                //test
                AIInterface *first = m_P1;
                AIInterface *second = m_P2;
                //test
                
                BoardInterface::Tag tag ;//= BoardInterface::Tag::O;
                
                if(x%2) tag = BoardInterface::Tag::X;
                else tag = BoardInterface::Tag::O;
                //test

                
                if (!playOneRound(first, tag, second)) { //if first & second = nullptr then program will die here
                    //TODO: what will happen if player does not follow the rule?(or the game does not finished properly)
                }
                updateGuiGame();

            } 
                //TODO: what will happen if who wins?

        } 

   private:
        void updateGuiGame()
        {
            gui->updateGame(MainBoard);
        }

        bool playOneRound(AIInterface *user, BoardInterface::Tag tag, AIInterface *enemy)
        {
            //call (AIInterface *user)'s (queryWhereToPut function) to choose a place to set tag
            auto pos = call(&AIInterface::queryWhereToPut, user, MainBoard);

            //catch the step that does not follow the rule
            if(pos.first>8 || pos.second>8 || pos.first<0 || pos.second<0)
            {
                std::cout<<"put illegal place!"<<std::endl;

                //TODO: catch other kinds of illegal step
                exit(-1);
            } 

            //update the MainBoard
            MainBoard.get(pos.first,pos.second) = tag;
            
            //indicate that this round has finished safely
            return true;
        }

        bool checkGameover()
        {  
            //TODO: how to check whether gameover?











            return true; // Gameover!
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





        int m_size;
        std::vector<int> m_ship_size;
        std::chrono::milliseconds m_runtime_limit;

        AIInterface *m_P1;
        AIInterface *m_P2;
        GUIInterface *gui;

        UltraBoard MainBoard;
    };
}
