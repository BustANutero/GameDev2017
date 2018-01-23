/*******************************************************
Copyright (C) 2016 meiry242@gmail.com
This file is part of http://www.gamedevcraft.com .
Author : Meir yanovich
********************************************************/


#ifndef __ZIGZAPZAG_H__
#define __ZIGZAPZAG_H__
#include <time.h>
#include "cocos2d.h"
USING_NS_CC;
 
//Inner data of each block Sprite
struct UserData
{
	UserData(bool bs ,bool g)
	{		 
		start = bs;
		hasGem = g;
	}
	bool start;
	bool hasGem;
};

//TAG names of the game sprites 
enum TAGS
{
	GEM,
	BLOCK,
	CIRCLE
};

class ZigZapZag : public cocos2d::LayerColor
{
public:
	 
	//bitwish flags 
	enum OperetionFlags
	{
		//No use
		NONE = 0x01,
		//when touch is triggered
		TOUCHED = 0x02,
		//when circle is moving right 
		MOVE_RIGHT = 0x04,
		//when circle is moving left 
		MOVE_LEFT = 0x08,
		//when touching is started
		GAME_TOUCH_START = 0x10,
		//when game is stoped
		GAME_STOP = 0x20,
		//when game is started 
        GAME_START = 0x40
	};


    static cocos2d::Scene* createScene();
    virtual bool init();    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);    
    // implement the "static create()" method manually
	CREATE_FUNC(ZigZapZag);
	

private:
 	//function comments in source file  
	Vec2 setBlockPostion(Sprite* const &block, int dir);
	void gameLoop(float delta);
	void invokeGame(float delta);
    void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event  *event);
    void onTouchesMoved(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event  *event);
    void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event  *event);
    void onTouchesCancelled(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event  *event);
	int  generateRandDirection(int modulo);
	void placeRandomGem(Sprite* &block, int seed);
	void removeGemFromBlock(Sprite* &block);
	bool handleCollision(Sprite* &block);
	void setTouchListners();
	void setLevel();
	void setGem(Sprite* &block);
	void setIncScore();
	void setScoreLabel();
	void setGameOverScreen();
	void setGameOverScreenVisible(bool isVisible);
	void setFallingAnim();
	float sign(const Vec2&  p1, const Vec2& p2, const Vec2& p3);
	bool  PointInTriangle(const Vec2& pt, const Vec2& v1, const Vec2& v2, const Vec2& v3);
	void gameOverCallback();

	std::list<Sprite*> blocksList;
	Sprite* circle;
	Sprite* gameOverScreen;
	Size visibleSize;
	Vec2 origin;
	Label* labelScore;
	float circleblockY;
	float circleblockX;	
	int blocksNum;
	int currentState;
	int zCount;
	int currentZorder;
	int iScore;
	bool stopGameLoop;
};

#endif // __ZIGZAPZAG_H__
