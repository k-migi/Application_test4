#include "SDL/SDL.h"
#include <string>
#include <vector>
#include <stdio.h>
#include"windows.h"

////自定义库
#include "sections/timer.h"

#define right 0    //小人的朝向
#define left 1

	FILE *com;
	char temp=0;

//屏幕
const int SCREEN_WIDTH = 1340;
const int SCREEN_HEIGHT =690;
const int SCREEN_BPP = 32;

//刷新率
int FRAMES_PER_SECOND = 28;
int PLAYER = 1;

//man的大小
const int DOT_WIDTH = 20;
const int DOT_HEIGHT = 20;
const int BOARD_LENGTH = 210;//板的長度
int BOARD_NUMBER = 20;//板的數量
const int COUNT_TIME =8 ;//刷這麼多次屏速度減1

   int Vy=0;//所有板的速度

//当前状态:主菜单,游戏设置菜单,游戏执行,游戏失败,确认退出
enum{menu,set_player,set_difficulty,run_game,game_over,comfirm_quit}state;

     //图片:鱼
SDL_Surface *man_right= NULL;//玩家
SDL_Surface *man_left= NULL;
SDL_Surface *man_right_2= NULL;
SDL_Surface *man_left_2= NULL;
SDL_Surface *man_right_3= NULL;
SDL_Surface *man_left_3= NULL;
SDL_Surface *man_right_4= NULL;
SDL_Surface *man_left_4= NULL;
      //板
SDL_Surface *board=NULL;
      //图片:背景
SDL_Surface *main_menu = NULL;
SDL_Surface *difficulty = NULL;
SDL_Surface *player = NULL;
SDL_Surface *end = NULL;//结束语:game over
SDL_Surface *quitting = NULL;//確認退出的界面
SDL_Surface *background  =  NULL;//大海背景
     //图片:鼠标按钮
SDL_Surface *buttonSheet = NULL;
     //屏幕指针
SDL_Surface *screen = NULL;


    //quit是结束整个游戏;over是结束本轮游戏,可选择进入下一轮
    bool quit = false;
    bool over = true;

//The event structure
SDL_Event event;

/////////////////////////////////////////////////////////////////////////
////--------------------------图片---------------------------////////////
void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

SDL_Surface *load_image( std::string  filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = SDL_LoadBMP( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface

            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0xFF, 0xFF, 0xFF ) );

        }
    }

    //Return the optimized surface
    return optimizedImage;
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Jump!Jump!Jump!", NULL );

    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the my_fish_right image
    man_right = load_image( "pictures/man_right.bmp" );
    man_left = load_image( "pictures/man_left.bmp" );
    man_right_2 = load_image( "pictures/man_right_2.bmp" );
    man_left_2 = load_image( "pictures/man_left_2.bmp" );
    man_right_3 = load_image( "pictures/man_right_3.bmp" );
    man_left_3 = load_image( "pictures/man_left_3.bmp" );
    man_right_4= load_image( "pictures/man_right_4.bmp" );
    man_left_4 = load_image( "pictures/man_left_4.bmp" );

    board = load_image( "pictures/board.bmp");

        buttonSheet = load_image( "pictures/button.bmp" );

    background  =  load_image("pictures/background.bmp");
    main_menu = load_image("pictures/main_menu.bmp");
    difficulty = load_image("pictures/difficulty.bmp");
    player = load_image("pictures/player.bmp");
    end = load_image("pictures/game_over.bmp");
    quitting = load_image("pictures/quit.bmp");


 		if(   (com=fopen("com3","r")) ==NULL   ) ;//            return false;//串口



    //If there was a problem in loading the dotbm
    if(    (main_menu!=NULL)&&(end!=NULL)&&(buttonSheet!=NULL)   )
    //If S loaded fine
      return true;
}

void clean_up()
{
    //Free the surface
		SDL_FreeSurface( screen );
		SDL_FreeSurface( background );
		SDL_FreeSurface( main_menu );
		SDL_FreeSurface( man_left );
		SDL_FreeSurface( man_right );
		SDL_FreeSurface( difficulty );
		SDL_FreeSurface( player );
		SDL_FreeSurface( quitting );
		SDL_FreeSurface( end );

		 	 SDL_FreeSurface( board  );

//		 	 free(com);

    //Quit SDL
    SDL_Quit();
}
///////////////////////////////////////////
//------碰撞检测------------------------////////////////////
bool check_collision( std::vector<SDL_Rect> &A, std::vector<SDL_Rect> &B )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Go through the A boxes
    for( int Abox = 0; Abox < A.size(); Abox++ )
    {
        //Calculate the sides of rect A
        leftA = A[ Abox ].x;
        rightA = A[ Abox ].x + A[ Abox ].w;
        topA = A[ Abox ].y;
        bottomA = A[ Abox ].y + A[ Abox ].h;

        //Go through the B boxes
        for( int Bbox = 0; Bbox < B.size(); Bbox++ )
        {
            //Calculate the sides of rect B
            leftB = B[ Bbox ].x;
            rightB = B[ Bbox ].x + B[ Bbox ].w;
            topB = B[ Bbox ].y;
            bottomB = B[ Bbox ].y + B[ Bbox ].h;

            //If no sides from A are outside of B
            if( ( ( bottomA <= topB ) || ( topA >= bottomB ) || ( rightA <= leftB ) || ( leftA >= rightB ) ) == false )
            {
                //A collision is detected
                return true;
            }
        }
    }
    //If neither set of collision boxes touched
    return false;
}

////////////////////////////////////////////////////////////////////////
// //////---------鱼的类定义-------------------------------//////////////

class Man
{
    private:
         //The X and Y offsets of the fish
        int x, y;

        //小人的方向
        int direction;

        //記錄速度的減少
        int count;
        //The velocity of the dot
        int xVel, yVel;

            //碰撞检测盒的坐标及大小
        std::vector<SDL_Rect> box;
        //移动检测盒
			    void shift_boxes();

    public:
        //Initializes the variables
        Man();

        //Takes key presses and adjusts the my_fish's velocity
        void handle_input(int);

        //踩到板往上跳
        void jump();

        //移动及检测
        void move();

        //显现
        void show(int);

        //碰撞盒
        void set_box();

        //取得Y的坐標
        int get_Y();

    //获取 的坐标及
       std::vector<SDL_Rect> &get_rects();
};

//构造函数
Man::Man()
{
    direction = 1;
    count = 0;
    //Initialize the offsets

	  x =rand()% (SCREEN_WIDTH/4)+SCREEN_WIDTH/4;
    y = SCREEN_HEIGHT/2;

    //Initialize the velocity
    xVel = 0;
    yVel = 0;

	  set_box();
}

void Man::set_box()
{
	        //Create the necessary SDL_Rects
    box.resize( 1 );

    box[ 0 ].w = 10;
    box[ 0 ].h = 10;

    //Move the collision boxes to their proper spot
    shift_boxes();
}

void Man::shift_boxes()
{
    //The row offset
    int r = 10;

    //Go through the dot's collision boxes
    for( int set = 0; set < box.size(); set++ )
    {
        //Center the collision box
        box[ set ].x = x -30;

        //Set the collision box at its row offset
        box[ set ].y = y + r;

        //Move the row offset down the height of the collision box
        r += box[ set ].h;
    }
}

void Man::handle_input(int i)
{

	//这是串口的
	if(temp==3)
	     {
	     	  if(xVel>0)
	     	         xVel -= DOT_WIDTH;
	     	  if(xVel<0)
	     	         xVel += DOT_WIDTH;
	     }
	if(temp==1)
	     {
	     	 if(xVel>=0)
	     	        xVel -= DOT_WIDTH;
	     	  direction = left;
	     }
	if(temp==2)
	     {
	     	if(xVel<=0)
	             xVel += DOT_WIDTH;
	       direction = right;
	     }

	if(i==2)
	{
	       if( event.type == SDL_KEYDOWN )
                 {
        //Adjust the velocity
                       switch( event.key.keysym.sym )
                            {
                                 case SDLK_a: xVel   -=  DOT_WIDTH;  direction =left;   break;
																 case SDLK_d: xVel   +=   DOT_WIDTH;  direction = right;  break;
																 case SDLK_UP: yVel   -=   DOT_WIDTH;break;
                            }
								}
           //If a key was released
            else if( event.type == SDL_KEYUP )
                 {
                   //Adjust the velocity
                    switch( event.key.keysym.sym )
                        {
                            case SDLK_a: xVel   +=  DOT_WIDTH; break;
                            case SDLK_d: xVel   -=   DOT_WIDTH;break;
														case SDLK_UP: yVel   +=   DOT_WIDTH;break;
                         }
                  }
	   }
	else if(i==1)
	{
    //If a key was pressed
    if( event.type == SDL_KEYDOWN )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_LEFT: xVel   -=  DOT_WIDTH;  direction =left;   break;
            case SDLK_RIGHT: xVel   +=   DOT_WIDTH;  direction = right;  break;
            case SDLK_UP: yVel   -=   DOT_WIDTH;break;
        }
    }
    //If a key was released
    else if( event.type == SDL_KEYUP )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_LEFT: xVel   +=  DOT_WIDTH; break;
            case SDLK_RIGHT: xVel   -=   DOT_WIDTH;break;
            case SDLK_UP: yVel   +=   DOT_WIDTH;break;
        }
    }
	}
	else if(i==3)
	{
    //If a key was pressed
    if( event.type == SDL_KEYDOWN )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_j: xVel   -=  DOT_WIDTH;  direction =left;   break;
            case SDLK_l: xVel   +=   DOT_WIDTH;  direction = right;  break;
            case SDLK_i: yVel   -=   DOT_WIDTH;break;
        }
    }
    //If a key was released
    else if( event.type == SDL_KEYUP )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_j: xVel   +=  DOT_WIDTH; break;
            case SDLK_l: xVel   -=   DOT_WIDTH;break;
            case SDLK_i: yVel   +=   DOT_WIDTH;break;
        }
    }
	}
	else if(i==4)
	{
    //If a key was pressed
    if( event.type == SDL_KEYDOWN )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_4: xVel   -=  DOT_WIDTH;  direction =left;   break;
            case SDLK_6: xVel   +=   DOT_WIDTH;  direction = right;  break;
            case SDLK_8: yVel   -=   DOT_WIDTH;break;
        }
    }
    //If a key was released
    else if( event.type == SDL_KEYUP )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_4: xVel   +=  DOT_WIDTH; break;
            case SDLK_6: xVel   -=   DOT_WIDTH;break;
            case SDLK_8: yVel   +=   DOT_WIDTH;break;
        }
    }
	}

}

void Man::jump()
{
	count = 0;
	if(y<SCREEN_HEIGHT/5)
	       yVel = -17;
	 else if( (y<SCREEN_HEIGHT/4)&&(y>SCREEN_HEIGHT/5)  )
	       yVel = -18;
	 else if( (y<SCREEN_HEIGHT/3)&&(y>SCREEN_HEIGHT/4)  )
	       yVel = -20;
	 else if( (y<SCREEN_HEIGHT/2)&&(y>SCREEN_HEIGHT/3)  )
	       yVel = -23;
	  else
         yVel = -25;
}


void Man::move()
{
    //Move the dot left or right
    x += xVel;
    shift_boxes();

    yVel+=Vy/10;//看起来自然一点

    //If the my_fisht went too far to the left or right
    if( ( x < 0 ) || ( x> SCREEN_WIDTH ) )
    {
        //move back
          if(x<0)x=SCREEN_WIDTH;
          if(x>SCREEN_WIDTH)x=0;//穿過屏幕
        shift_boxes();
    }

    //Move the dot up or down
    y += yVel;
    shift_boxes();
    if(y<0)
         yVel++;
    //If the my_fish_right went too far up or down
    if( ( y + DOT_HEIGHT > SCREEN_HEIGHT +100) )
    {

        y = SCREEN_HEIGHT +101;
        shift_boxes();

    }
    	  count++;
	  //實現自然下落
	    if(count >= COUNT_TIME)
	       {
	       	  count = 0;
	       	  yVel += 11;
	       }
}

int Man::get_Y()
{
	  return y;
}


std::vector<SDL_Rect> &Man::get_rects()
{
    //Retrieve the collision boxes
    return box;
}

void Man::show(int i)
{
	if(i==1)
	  {
        if(direction==right)
              apply_surface( x, y, man_right, screen );
       if(direction==left)
              apply_surface(x,y,man_left,screen  );
	  }
	else if (i==2)
	   {
        if(direction==right)
              apply_surface( x, y, man_right_2, screen );
         if(direction==left)
              apply_surface(x,y,man_left_2,screen  );
	   }
	  else if(i==3)
	   {
        if(direction==right)
              apply_surface( x, y, man_right_3, screen );
         if(direction==left)
              apply_surface(x,y,man_left_3,screen  );
	   }
	  else if(i==4)
	   {
        if(direction==right)
              apply_surface( x, y, man_right_4, screen );
         if(direction==left)
              apply_surface(x,y,man_left_4,screen  );
	   }
}

////////////////////////////////////////////////
////-----跳板--------------/////////////////////////////
class Board
{
    private:
           //位置
            int x, y;
        //The velocity of the dot
            int xVel, yVel;

           //記錄速度的減少
          int count;

           //碰撞检测盒
            std::vector<SDL_Rect> box;
            //移动检测盒
			    void shift_boxes();

    public:
        Board();
        //踩到時板向下運動
        void down(int);

        //移动及检测
        void move(int);

        //显现鱼
        void show();

        //回到開頭
        void re_new();

        //获取鱼的碰撞盒
        std::vector<SDL_Rect> &get_rects();
};


//构造函数
Board::Board()
{
	    xVel =0;
			yVel = 0;

      count = 0;
	    x = rand()%SCREEN_WIDTH;
	    y = rand()%800;

     //   set_box
         box.resize( 1 );
             box[0].y= y-3;
             box[ 0 ].w = BOARD_LENGTH;
						 box[ 0 ].h = 30;
}

void Board::shift_boxes()
{
    //The row offset
    int r = 0;

    //Go through the dot's collision boxes
    for( int set = 0; set < box.size(); set++ )
    {
        //Center the collision box
        box[ set ].x = x + ( DOT_WIDTH - box[ set ].w ) / 2;

        //Set the collision box at its row offset
        box[ set ].y = y + r;

        //Move the row offset down the height of the collision box
        r += box[ set ].h;
    }
}

void Board::down(int doub=1 )
{
    yVel = 10*doub;
    count = 0;
}

void Board::move(int V)
{
    //Move the dot left or right
    yVel=V;

    x += xVel;
    shift_boxes();
    y += yVel;
    shift_boxes();

    if(y>SCREEN_HEIGHT)//走到尽头后再走一次
         {
            re_new();
         }

         if(count++ ==COUNT_TIME )
              {
              	count = 0;
              	if(yVel>0)
              	     {
              	     	  yVel--;
              	     }

              	  if(Vy>0)
              	     Vy--;
              }

}

void Board::show()
{
       apply_surface( x, y, board, screen );
}

void Board::re_new()
{
	    xVel =0;
			yVel = 0;

			x = rand()%SCREEN_WIDTH;

			int i=rand()%2;
      if (i==1)
         {
	            y = 0;
         }
     else
         y=rand()%SCREEN_HEIGHT - SCREEN_HEIGHT;
}

std::vector<SDL_Rect> &Board::get_rects()
{
    //Retrieve the collision boxes
    return box;
}

//The button
class Button
{
    private:
    //The attributes of the button
    SDL_Rect box;

    public:
    //Initialize the variables
    Button( int x, int y, int w, int h );

    //Handles events and set the button's sprite region
    bool handle_events();

    //Shows the button on the screen
    void show();
};

Button::Button( int x, int y, int w, int h )
{
    //Set the button's attributes
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;

}

bool Button::handle_events()
{
    //The mouse offsets
    int x = 0, y = 0;

    //If the mouse moved
    if( event.type == SDL_MOUSEMOTION )
    {
        //Get the mouse offsets
        x = event.motion.x;
        y = event.motion.y;

        //If the mouse is over the button
        if( ( x > box.x ) && ( x < box.x + box.w ) && ( y > box.y ) && ( y < box.y + box.h ) )
        {
            show();
        }

    }
    //If a mouse button was pressed
    if( event.type == SDL_MOUSEBUTTONDOWN )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            //Get the mouse offsets
            x = event.button.x;
            y = event.button.y;

            //If the mouse is over the button
            if( ( x > box.x ) && ( x < box.x + box.w ) && ( y > box.y ) && ( y < box.y + box.h ) )
            {
                //Set the button sprite
                 show();
                return true;
            }
        }
    }
    return false;
}

void Button::show()
{
    //Show the button
    apply_surface( box.x-20, box.y, buttonSheet, screen);
}


///////////////////////////////////////////////////////////
//-----------------------操作游戏流程的函数-----------/////////////
//////////////////////////////////////////////////////////

//显示主菜单
int Main_menu();

int Set_difficulty();

int Set_player();

int Run_game();

int Game_over();

int Comfirm_quit();
////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//--------------------------------主程序-------------------------//////////
///////////////////////////////////////////////////////////////////
int main( int argc, char* args[] )
{

    //Initialize
    if( init() == false )
            {
                return 1;
            }

    //Load the files
    if( load_files() == false )
            {
               return 1;
            }

    //大框架开始了!!
    while(1)
    {

    	fscanf(com,"%c",&temp);

       if(quit==false)
 	               switch(state)
 	               	     {
	   	                  	   case menu :
	     	                                  	                  Main_menu();
	     	                                   	                 break;
	   	                  	   case set_player :
                                          	                  Set_player();
                                         	                  break;
	    	                 	   case set_difficulty:
                                        	                     Set_difficulty();
                                        	                    break;
	     	 	                  case run_game:
	     	                               	                        Run_game();

	     	                             	                         if(state == run_game)
	     	                             	                               {
	     	                                        	                       state = game_over;
	     	                          	                                 }
	     	                            	                          if(quit==true)
	     	                                	                           return 0;
	     	                             	                        break;
	     	  	                 case game_over:
                               	                            Game_over();
                              	                             break;
	     	                }

             //清空内存并退出
							if(quit == true)
                    {
                    	 Comfirm_quit();
                     }
						 if(quit == true)
                      return 0;

    }

}




///////////////////////////////////////////////////////////
//-----------------------操作游戏流程的函数-----------/////////////
//////////////////////////////////////////////////////////

int Main_menu()
{
	           //鼠標按鈕
    Button start_button( 510, 225, 300, 60 );//橫坐標,縱坐標,寬,高
		Button player_button( 510, 315, 300, 60 );
		Button difficulty_button( 520, 410, 300, 60 );
		Button quit_button( 520, 490, 300, 60 );


	                //开始界面
             apply_surface( 0, 0, main_menu, screen );


        if( SDL_PollEvent( &event ) )
        {
            //如果鼠標點了"開始"
            if(start_button.handle_events() ==true )
                    {
                    	  over = false;
                    	  state = run_game;
                    }
             //如果点了"选手按钮"
						if(player_button.handle_events() ==true)
						       {
						       	   state = set_player;
 						       }
						  //如果點了"難度按鈕"
						if(difficulty_button.handle_events() ==true)
						       {
						       	   state = set_difficulty;
						       }
             //如果鼠標點了"結束"
            if(quit_button.handle_events() ==true )
                  {
                  	  quit = true;
                  }

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }

                        if( SDL_Flip( screen ) == -1 )
                                {
                                    return 1;
                                }
        }
}

int Set_difficulty()
{
	  Button easy_button(510, 225, 300, 60  );//橫坐標,縱坐標,寬,高
		Button middle_button( 510, 315, 300, 60 );
		Button crazy_button( 520, 410, 300, 60 );
		Button quit_button( 520, 490, 300, 60 );

         apply_surface( 0, 0, difficulty, screen );

            if( SDL_PollEvent( &event ) )
        {
                if(easy_button.handle_events() ==true )
                     {
                     	  FRAMES_PER_SECOND = 28;
                     	  BOARD_NUMBER = 15;
                    	  state = menu;
                     }
                if(middle_button.handle_events() ==true )
                     {
                     	  FRAMES_PER_SECOND = 35;
                     	  BOARD_NUMBER = 11;
                    	  state = menu;
                     }
                if(crazy_button.handle_events() ==true )
                     {
                     	  FRAMES_PER_SECOND = 50;
                     	  BOARD_NUMBER = 9;
                    	  state = menu;
                     }
                if(quit_button.handle_events() ==true )
                     {
                    	  quit = true;
                     }
                if( event.type == SDL_QUIT )
                      {
                        //Quit the program
                               quit = true;
											}
      				if( SDL_Flip( screen ) == -1 )
								 {
													return 1;
							   }
        }
}

int Set_player()
{
	  Button oneP_button( 510, 185, 300, 60 );//橫坐標,縱坐標,寬,高
		Button twoP_button( 530, 240, 300, 60 );
		Button threeP_button( 520, 315, 300, 60 );
		Button fourP_button( 520, 410, 300, 60 );
		Button quit_button( 520, 490, 300, 60 );

         apply_surface( 0, 0, player, screen );

            if( SDL_PollEvent( &event ) )
             {
                if(oneP_button.handle_events() ==true )
                     {
                     	  PLAYER=1;
                    	  state = menu;
                     }
                if(twoP_button.handle_events() ==true )
                     {
                     	  PLAYER=2;
                    	  state = menu;
                     }
                if(threeP_button.handle_events() ==true )
                     {
                     	  PLAYER=3;
                    	  state = menu;
                     }
                if(fourP_button.handle_events() ==true )
                     {
                     	  PLAYER=4;
                    	  state = menu;
                     }
                if(quit_button.handle_events() ==true )
                     {
                    	  quit = true;
                     }
                if( event.type == SDL_QUIT )
                      {
                        //Quit the program
                               quit = true;
											}
      				if( SDL_Flip( screen ) == -1 )
								 {
													return 1;
							   }
        }
}

int Run_game()
{

    //The frame rate regulator
    Timer fps;
    bool collision = false; //表示是否跳到板上

    //玩家控制的鱼
       	  Man myMan[PLAYER];


    //其他鱼
    Board boards[BOARD_NUMBER];
									for(int i=0;i<PLAYER;i++)
                        myMan[i].jump();

	             while( over == false )
									{
               //Start the frame timer
                        fps.start();

               		fscanf(com,"%c",&temp);//接受串口数据
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);
              			fscanf(com,"%c",&temp);

              			myMan[0].handle_input(1);

		 					     //While there's events to handle
                        if( SDL_PollEvent( &event ) )
                          {
                                   //Handle events for the dot
                                   for(int i=0;i<PLAYER;i++)
                                     {
                                        myMan[i].handle_input(i+1);
                                     }

                                    //If the user has Xed out the window
                                    if( event.type == SDL_QUIT )
                                      {
                                        //Quit the program
                                        Comfirm_quit();
                                        if(quit==true||over == true)
                                              return 0;
                                      }
                         }
                   collision = false;
										//逐个检测碰撞
										 for(int i=0;i<PLAYER;i++)
										   {
                            for(int j=0;j<BOARD_NUMBER;j++)
                              {
        	                         //如果检测到有碰撞发生
                                  if(check_collision(myMan[i].get_rects() , boards[j].get_rects() )==1   )
                                       {
                                          myMan[i].jump();
                                          collision = true;
													        	    }
                              }
										   }

                          //如果小人坐標太高,則板下沉

                        for(int i=0;i<PLAYER;i++)
                         {
                            for(int j=0;j<PLAYER;j++)
                                {
                         	           if(  (myMan[j].get_Y()>=2*SCREEN_HEIGHT/3)&&(myMan[j].get_Y()!=SCREEN_HEIGHT+101)  )
                         	              {
                         	              	   i = PLAYER;
                         	              	   Vy=0;
                         	              	   collision= false;
                              	              break;//如果有一个小人在底部,则不能向上移屏
                         	              }
                                  }
                          if(i==PLAYER)
                               break;
                         	    //如果都过了1/3高度,则向上移屏,移多少由他们的高度决定
                          if(    myMan[i].get_Y()<= (2*SCREEN_HEIGHT/3)   &&   collision == true )
                                {
                                	 if(myMan[i].get_Y()<=SCREEN_HEIGHT/4)
                                        	         Vy=15;
                                	 else if((myMan[i].get_Y()>SCREEN_HEIGHT/4)  &&  (myMan[i].get_Y()<=SCREEN_HEIGHT/3))
                                        	         Vy=  10;
                                    else
                                                 Vy=5;
                                     Vy+=20;
                                    collision = false;
                                 }
                         }

                        //移动玩家
											 for(int i=0;i<PLAYER;i++)
                           myMan[i].move();
                        //移动板
                        for(int i=0;i<BOARD_NUMBER;i++)
                                {
                                    boards[i].move(Vy);

                                }

                        //Fill the screen white
                        apply_surface(0,0,background,screen);

                        //Show the my_fish on the screen
                        for(int i=0;i<PLAYER;i++)
                           myMan[i].show(i+1);

                        for(int i=0;i<BOARD_NUMBER;i++)
                                {
                                    boards[i].show();
                                }

                        //Update the screen
                        if( SDL_Flip( screen ) == -1 )
                                {
                                    return 1;
                                }


                   //判断是否死亡(两人都落下了)
                   over = true;
                   for(int i=0;i<PLAYER;i++)
                      {
                      	  if(myMan[i].get_Y()<SCREEN_HEIGHT+100)
                      	         over = false;
                      }

                 if(over == true)
                   {
												apply_surface( 0, 0, end, screen );

                        if( SDL_Flip( screen ) == -1 )
                                {
                                    return 1;
                                }
                   }

        //Cap the frame rate
                        if( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
                                {
                                    SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
                                }

             }
    //游戏结束
}

int Game_over()
{
			 apply_surface( 0, 0, end, screen );

   	    Button restart_button( 510, 315, 300, 60 );
   	    Button menu_button( 520, 410, 300, 60  );
	    	Button quit_button( 520, 490, 300, 60 );


        if( SDL_PollEvent( &event ) )
        {
                if(menu_button.handle_events() ==true )
                     {
                    	  state = menu;
                     }
                if(restart_button.handle_events() ==true )
                     {
                    	  over = false;
                    	  state = run_game;
                     }
                if(quit_button.handle_events() ==true )
                     {
                    	  quit = true;
                     }
                if( event.type == SDL_QUIT )
                      {
                        //Quit the program
                               quit = true;
											}
      				if( SDL_Flip( screen ) == -1 )
								 {
													return 1;
							   }
        }
}

int Comfirm_quit()
{
   	    Button continue_button( 510, 315, 300, 60 );
   	    Button menu_button( 520, 410, 300, 60 );
	    	Button quit_button( 520, 490, 300, 60 );
     while(1)
     {
     	 	apply_surface( 0, 0, quitting, screen );

fscanf(com,"%c",&temp);

        if( SDL_PollEvent( &event ) )
        {
                if(continue_button.handle_events() ==true )
                     {
                    	  quit = false;
                    	  break;
                     }
                if(menu_button.handle_events() ==true )
                     {
                     	  quit = false;
                     	  over = true;
                    	  state = menu;
                    	  break;
                     }
                if(quit_button.handle_events() ==true||( event.type == SDL_QUIT ) )
                     {
                     	   over = true;
                     	   quit = true;
                         clean_up();
                    	  return 0;
                     }

      	if( SDL_Flip( screen ) == -1 )
								 {
													return 1;
							   }
         }
     }
}
