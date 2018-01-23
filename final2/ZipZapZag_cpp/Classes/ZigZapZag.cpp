/*******************************************************
Copyright (C) 2016 meiry242@gmail.com
This file is part of http://www.gamedevcraft.com .
Author : Meir yanovich
********************************************************/

#include "ZigZapZag.h"
#include "SimpleAudioEngine.h"

#define SPEED 100.0
#define CIRCLE_SPEED 200.0
#define MAX_Z_ORDER 10000
#define MAX_BLOCKS_ON_SCREEN 50
#define GEM_IMG "Images/gem_30.png"
#define BLOCK_IMG "Images/block_80_180.png"
#define CIRCLE_IMG "Images/circle_30.png"
#define BLANK_IMG "Images/blank.png"

 
Scene* ZigZapZag::createScene()
{
	srand(time(NULL)); // Seed the time
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
	auto layer = ZigZapZag::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool ZigZapZag::init()
{
	 	 
    //////////////////////////////
    // 1. super init first
	if (!LayerColor::initWithColor(Color4B(255,255,255, 255)))
    {
        return false;
    }
		
	//set images into cache 
	Director::getInstance()->getTextureCache()->addImage(BLOCK_IMG);
	Director::getInstance()->getTextureCache()->addImage(CIRCLE_IMG);
	Director::getInstance()->getTextureCache()->addImage(GEM_IMG);
	
    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

	
	
	auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(ZigZapZag::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));


	 
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
	//setup score lable 
	setScoreLabel();
	//setup Level
	setLevel();
	//set touch listeners 
	setTouchListners();
	//set game over screen visible == false
	setGameOverScreen();
	//start game loop 
	this->schedule(CC_SCHEDULE_SELECTOR(ZigZapZag::gameLoop));	 

    return true;
}

//generate lavel
void ZigZapZag::setLevel()
{
	//init variables
	//keep track on which block the circle is currently on 
	currentZorder = 0;
	//How many block will be visible on every given time on the screen 
	blocksNum = MAX_BLOCKS_ON_SCREEN;
	//how deep the blocks z order supposed to be , if its long play consider to rise the number
	zCount = MAX_Z_ORDER;
	//the bitwise boolean holder
	currentState = 0;
	//keep the score
	iScore = 0;
	//set into score label , this is needed when game restart
	labelScore->setString("0");
	//stop game loop indicator
	stopGameLoop = false;
	//set the start bit to on 
	currentState |= GAME_START;
	//the circle must be alwas above all other Spirits
	circle = Sprite::create(CIRCLE_IMG);
	circle->setTag(CIRCLE);
	this->addChild(circle, zCount + 1);

	//scatter the blocks and Gems 
	Sprite* tempSprite = nullptr;
	for (int i = 0; i < blocksNum; i++)
	{
		blocksList.push_back(Sprite::create(BLOCK_IMG));
		blocksList.back()->setAnchorPoint(Vec2(0.0, 0.0));
		blocksList.back()->setUserData(new UserData(false, false));
		blocksList.back()->setTag(BLOCK);
		zCount = zCount - 1;
		this->addChild(blocksList.back(), zCount);
		float blockSizeWidth = blocksList.back()->getContentSize().width;
		float blockSizHeight = blocksList.back()->getContentSize().height;

		if (i == 0)
		{   //Start Game Reposition the blocks 
			blocksList.back()->setPosition(Vec2(visibleSize.width / 2 + origin.x - (blockSizeWidth / 2),
				visibleSize.height / 2 + origin.y - (blockSizHeight / 2)));
			circleblockY = blocksList.back()->getPositionY() + blockSizHeight - (circle->getContentSize().height / 2);
			circleblockX = blocksList.back()->getPositionX() + blockSizeWidth / 2;
			circle->setPosition(Vec2(circleblockX, circleblockY));
		}
		else
		{
			//As the first touch is to the right 
			//it is better to give the player some easy learning ajusting to the game 
			Vec2 bv2;
			if (i < 2)
			{

				bv2 = setBlockPostion(tempSprite, 0);
				
			}
			else if (i == 2)
			{
				bv2 = setBlockPostion(tempSprite, 0);
			}
			else if (i == 3)
			{
				bv2 = setBlockPostion(tempSprite, 1);
			}
			else if (i == 4)
			{
				bv2 = setBlockPostion(tempSprite, 1);
			}
			else if (i > 4)
			{
				bv2 = setBlockPostion(tempSprite, generateRandDirection(2));
			}
			//set the new position of the block
			blocksList.back()->setPosition(Vec2(bv2.x + origin.x, bv2.y + origin.y));
			//some random play ... 
			placeRandomGem(blocksList.back(),i);
			
		}
		tempSprite = blocksList.back();
	}

}

//Generate random number fo the blocks placemet  
//Generate random number fo the blocks placemet 
int  ZigZapZag::generateRandDirection(int modulo)
{
	int finalNum = rand() % (100 - 2) + 2; // Generate the number	
	int randDir = 0;
	if (finalNum % modulo)
	{
		randDir = 1;
	}
	return randDir;
}
//place the gem according to ramdom play ( can be modified ) 
void ZigZapZag::placeRandomGem(Sprite* &block,int seed)
{
	int placment = generateRandDirection(3);
	if (seed % 2 == 0)
	{
		if (placment == 1)
		{
			setGem(block);
		}
	}
}

//create place the gem above the block 
void ZigZapZag::setGem(Sprite* &block)
{
	((UserData*)block->getUserData())->hasGem = true;
	Sprite* gem = Sprite::create(GEM_IMG);
	gem->setTag(GEM);
	block->addChild(gem, 1);
    //the gem sprite ancor point is in the middle but the gem have also shadow
	//so there is need to fine tune the placment so it will be in the center of the block 
	float grayX = block->getContentSize().width / 2;
	float grayY = block->getContentSize().height - (block->getContentSize().width / 2) / 2;
	gem->setPositionX(grayX);
	gem->setPositionY(grayY);
}

//Main game loop 
void ZigZapZag::gameLoop(float delta)
{
	if (!stopGameLoop)
	{
		invokeGame(delta);
	}
}

//Game logic of the game 
void ZigZapZag::invokeGame(float delta)
{
	bool bBelow = false;

	if ((currentState & GAME_TOUCH_START) && (currentState & GAME_START))
	{
		if ((currentState & TOUCHED) && (currentState &  MOVE_RIGHT))
		{
			float x = circle->getPositionX() + (CIRCLE_SPEED * delta);
			circle->setPositionX(x);
		}
		else if ((currentState & TOUCHED) && (currentState &  MOVE_LEFT))
		{
			float x = circle->getPositionX() - (CIRCLE_SPEED * delta);
			circle->setPositionX(x);
		}
		//iterate the blocks 
		//1.check if circle fell
		//2.check if gem is on block 
		//3.move the blocks down and below the screen 
		//4.add new block on back 
		int randomSeed = 0;
		for (std::list<Sprite*>::iterator it = blocksList.begin(); it != blocksList.end(); ++it)
		{
			float circleY = std::round(circle->getPositionY());
			float circleX = std::round(circle->getPositionX());			 
			float blockHeight = (*it)->getPositionY() + (*it)->getContentSize().height;

			/*
			  c
			/ @ \
		  b@     @d
			\   /
			  @
			  a
			*/

			//calculate dimond shape points 
			Vec2 A;
			A.x = (*it)->getPositionX() + ((*it)->getContentSize().width / 2);
			A.y = blockHeight - (*it)->getContentSize().width / 2;

			Vec2 B;
			B.x = (*it)->getPositionX();
			B.y = blockHeight - ((*it)->getContentSize().width / 2) / 2;

			Vec2 C;
			C.x = (*it)->getPositionX() + ((*it)->getContentSize().width / 2);
			C.y = blockHeight + (*it)->getContentSize().width / 2;

			Vec2 D;
			D.x = (*it)->getPositionX() + ((*it)->getContentSize().width);
			D.y = blockHeight - ((*it)->getContentSize().width / 2) / 2;

			//our collision detection is based on checking on our block dimond shape is 
			//intersction with the circle , for this we devide the dimond shape to 2 Triangles
			//and checking each triangle if the circle inside .
			bool insideLeft = PointInTriangle(circle->getPosition(),
				A,
				B,
				C);

			bool insideRight = PointInTriangle(circle->getPosition(),
				A,
				C,
				D);


			if (insideRight || insideLeft)
			{
				handleCollision((*it));
				 
			}			 
			else
			{
				//when collision detection is detected stop game loop 
				if (((UserData*)(*it)->getUserData())->start)
				{
					if (currentZorder == (*it)->getZOrder())
					{
						((UserData*)(*it)->getUserData())->start = false;
						stopGameLoop = true;
						//if stoped then invoke the circle drop down animation 
						setFallingAnim();
					}
				}
			}

			if (bBelow)
			{
				blocksList.pop_front();
				Sprite* backSprite = blocksList.back();
			}
			float y = (*it)->getPositionY() - (SPEED * delta);
			(*it)->setPositionY(y);

			//check if the sprite is below the screen then next iteration remove it 
			if ((*it)->getPositionY() < 0 - ((*it)->getContentSize().height))
			{
				//mark to remove it in the next iteration 
				bBelow = true;
				//reuse it as new sprite to be on top of the FIFO list
				Sprite* frontSprite = blocksList.front();
				//log("frontSprite x:%f y:%f", frontSprite->getPositionX(), frontSprite->getPositionY());
				Sprite* backSprite = blocksList.back();
				//log("backSprite x:%f y:%f", backSprite->getPositionX(), backSprite->getPositionY());
				Vec2 bv2 = setBlockPostion(backSprite, generateRandDirection(2));
				frontSprite->setPosition(Vec2(bv2.x + origin.x, bv2.y + origin.y));
				//new Z order alway lower then the last one 
				zCount = zCount - 1;
				frontSprite->setZOrder(zCount);
				//some random play ... 
				placeRandomGem(frontSprite, randomSeed);
				
				blocksList.push_back(frontSprite);
				//++stopThis;
			}
			else
			{
				bBelow = false;
			}
			randomSeed++;
		}
	}
	else if ((currentState & GAME_TOUCH_START) && ((currentState & GAME_START) == 0))
	{
		float y = circle->getPositionY() - ((CIRCLE_SPEED * 10) * delta);
		circle->setPositionY(y);
	}
}

//handle the action when collision is detected
bool ZigZapZag::handleCollision(Sprite* &block)
{
	currentZorder = block->getZOrder();
	((UserData*)(block)->getUserData())->start = true;
	bool isGem = ((UserData*)(block)->getUserData())->hasGem;
	//if gem is here hide it increase points by 1
	if (isGem)
	{
		removeGemFromBlock(block);		
		return true;
	}
	return false;
}

void ZigZapZag::removeGemFromBlock(Sprite* &block)
{
	block->getChildByTag(GEM)->setVisible(false);
	block->removeAllChildren();
	block->visit();
	
	((UserData*)(block)->getUserData())->hasGem = false;
	setIncScore();
}

float ZigZapZag::sign(const Vec2&  p1, const Vec2& p2, const Vec2& p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}
//check if pt it is in the triangle
bool  ZigZapZag::PointInTriangle(const Vec2& pt, const Vec2& v1,
										const Vec2& v2, const Vec2& v3)
{
	bool b1, b2, b3;

	b1 = sign(pt, v1, v2) < 0.0f;
	b2 = sign(pt, v2, v3) < 0.0f;
	b3 = sign(pt, v3, v1) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
}

//set the circle falling animation 
void ZigZapZag::setFallingAnim()
{
	MoveTo* moveDownAction;
	MoveTo* moveSideAction;
	float down = -50.0;
	float iMoveSide = -30.0f;

	if ((currentState & MOVE_RIGHT))
	{
		iMoveSide = iMoveSide * -1;
	}
	float moveSideDir = circle->getPositionX() + iMoveSide;
	//move circle to the side 
	moveSideAction = MoveTo::create(0.2f, Vec2(moveSideDir, circle->getPositionY()));
	//move the circle down 
	moveDownAction = MoveTo::create(1.0f, Vec2(circle->getPositionX(), -50.0));
	//call function when circle down animation is done 
	CallFunc *gameOverAction = CallFunc::create(std::bind(&ZigZapZag::gameOverCallback, this));
	//order all actions
	auto seq1 = Sequence::create(moveSideAction,
								 moveDownAction,
								 gameOverAction, nullptr);
	//execute all animations on circle
	circle->runAction(seq1);
}

//invoke when felling animation sequence is done 
void ZigZapZag::gameOverCallback()
{
	//invoke the game over screen 
	setGameOverScreenVisible(true);
	//turn off game start bit 
	currentState &= ~GAME_START;
}

//set the game over screen visibilty 
void ZigZapZag::setGameOverScreenVisible(bool isVisible)
{
	if (isVisible)
	{
		gameOverScreen->setVisible(true);
	}
	else
	{
		gameOverScreen->setVisible(false);
	}
}

//create the game over screen 
void ZigZapZag::setGameOverScreen()
{
	gameOverScreen = Sprite::create(BLANK_IMG);
	gameOverScreen->setTextureRect(Rect(0, 0, visibleSize.width - 50, visibleSize.height/3));
	gameOverScreen->setColor(Color3B(100, 255, 255));
	gameOverScreen->setOpacity(100);
	gameOverScreen->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	gameOverScreen->setGlobalZOrder(zCount + 3);
	this->addChild(gameOverScreen);

 
	TTFConfig ttfConfig("fonts/arial.ttf", 20);
	Label* labelGameOver = Label::createWithTTF(ttfConfig, "Game Over \n\n Try Again! ", TextHAlignment::CENTER, gameOverScreen->getContentSize().width / 1.5);
	labelGameOver->setTextColor(Color4B::BLACK);
	labelGameOver->setPosition(Vec2(gameOverScreen->getContentSize().width / 2,
		gameOverScreen->getContentSize().height/2));
	labelGameOver->setGlobalZOrder(zCount + 4);
	gameOverScreen->addChild(labelGameOver);

	setGameOverScreenVisible(false);

}

//setup the score lable
void ZigZapZag::setScoreLabel()
{ 
	 
	TTFConfig ttfConfig("fonts/arial.ttf", 44);
	labelScore = Label::createWithTTF(ttfConfig, "0", TextHAlignment::CENTER, visibleSize.width / 1.5);
	labelScore->setTextColor(Color4B::BLACK);
	labelScore->setPosition(Vec2(origin.x + labelScore->getContentSize().width ,
		origin.y + visibleSize.height - labelScore->getContentSize().height));
	this->addChild(labelScore, MAX_Z_ORDER + 1);
}

//increase the scroe by 1 
void  ZigZapZag::setIncScore()
{
	++iScore;
	char scroe[100];
	sprintf(scroe, "%d", iScore);
	labelScore->setString(std::string(scroe));
	 
}

// 0 = left ,1 = right 
//set the blocks positions based on previous block 
Vec2 ZigZapZag::setBlockPostion(Sprite* const &block,  int dir)
{
	Vec2 vec2;
	float x = block->getPositionX();
	float y = block->getPositionY();
	float w = block->getContentSize().width;
	float h = block->getContentSize().height;
 	 
	if ((x + w) > visibleSize.width)
	{
		dir = 1;
	}
	else if (x < 0)
	{
		dir = 0;
	}

    if (dir == 1)
	{
		vec2.x = x - w/2;
		vec2.y = y + (w/2)/2;
        vec2.x = vec2.x + 1.0;
		vec2.y = vec2.y + 1.0;

	}
    else if (dir == 0)
    {
        vec2.x = x + w / 2;
        vec2.y = y + (w / 2) / 2;
        vec2.x = vec2.x - 1.0;
		vec2.y = vec2.y - 1.0;
    }

	return vec2;
}

//set up the touch lesteners 
void ZigZapZag::setTouchListners()
{
	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(ZigZapZag::onTouchesBegan, this);
	listener->onTouchesMoved = CC_CALLBACK_2(ZigZapZag::onTouchesMoved, this);
	listener->onTouchesEnded = CC_CALLBACK_2(ZigZapZag::onTouchesEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

//The one and only touch is used to triger the left right movment of the circle
void ZigZapZag::onTouchesBegan(const std::vector<Touch*>& touches, Event  *event)
{
    for (auto &item : touches)
    {	
		auto touch = item;
		auto location = touch->getLocation();
		//check if game is started 
		if (currentState & GAME_START)
		{
			//check if touch is invoketed 
			if ((currentState & GAME_TOUCH_START) == 0)
			{
				//set touch bit to on 
				currentState |= GAME_TOUCH_START;
			}
			//check if right circle movment bit is off
			if ((currentState & MOVE_RIGHT) == 0)
			{
				//turn off left bit 
				currentState &= ~MOVE_LEFT;
				//turn on touched and move right bit on 
				currentState |= TOUCHED | MOVE_RIGHT;
			}
			else if ((currentState & MOVE_RIGHT)) //check if the right bit if on 
			{
				//turn off right bit 
				currentState &= ~MOVE_RIGHT;
				//turn on touched and move left bit on 
				currentState |= TOUCHED | MOVE_LEFT;
			}
		}
		else if((currentState & GAME_START) == 0)
		{
			//game is ended 
			//Retry touch is trigered reshuffle all blocks and start over 
			Vector<Node*> allNodes = this->getChildren();
			for (auto &node : allNodes)
			{
				if (node->getTag() == BLOCK)
				{
					this->removeChild(node);
				}
				else if (node->getTag() == CIRCLE)
				{
					this->removeChild(node);
				}
			}			
			//clean blocks container
			blocksList.clear();
			//set gameover screen to un visible
			setGameOverScreenVisible(false);
			//Start Level all over again 
			setLevel();

		}
    }
}

void ZigZapZag::onTouchesMoved(const std::vector<Touch*>& touches, Event  *event)
{
    for (auto &item : touches)
    {
        auto touch = item;
       
    }
}

void ZigZapZag::onTouchesEnded(const std::vector<Touch*>& touches, Event  *event)
{
    for (auto &item : touches)
    {
        auto touch = item;
       
    }
}
void ZigZapZag::onTouchesCancelled(const std::vector<Touch*>& touches, Event  *event)
{
    onTouchesEnded(touches, event);
}

void ZigZapZag::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

