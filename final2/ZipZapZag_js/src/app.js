
/*******************************************************
Copyright (C) 2016 meiry242@gmail.com
This file is part of http://www.gamedevcraft.com .
Author : Meir yanovich
********************************************************/
var SPEED = 100.0;
var CIRCLE_SPEED =  200.0;
var MAX_Z_ORDER = 10000;
var MAX_BLOCKS_ON_SCREEN = 35;
 
//sprites tags names 
var TAGS = Object.freeze({
    "GEM":0,
	"BLOCK":1,
	"CIRCLE":3
});

//block sprite inner data 
function UserData(bs,g)
{		 
    //start moving On top of a block .
    this.start = bs;
	//this block has GEM on top or not
    this.hasGem = g;
}

//bitwish flags 
var OperetionFlags = Object.freeze({ 
    //No use
    "NONE":0x01,
    //when touch is triggered
    "TOUCHED":0x02,
    //when circle is moving right 
    "MOVE_RIGHT":0x04,
    //when circle is moving left 
    "MOVE_LEFT":0x08,
    //when touching is started
    "GAME_TOUCH_START":0x10,
    //when game is stoped
    "GAME_STOP":0x20,
    //when game is started 
    "GAME_START":0x40
});


var ZipZapZagLayer = cc.LayerColor.extend({
    sprite:null,
    stopGameLoop:false,
    labelScore:null,
    winSize:0,
    origin:0,
    blocksList : [] ,
    circle:null,
    gameOverScreen:null,
    circleblockY:0,
	circleblockX:0,	
	blocksNum:MAX_BLOCKS_ON_SCREEN,
	currentState:OperetionFlags.NONE,
	zCount:MAX_Z_ORDER,
	currentZorder:0,
	iScore:0,
    visibleSize:0,
	

    ctor:function () {
        //////////////////////////////
        // 1. super init first
        this._super();
        this.init( cc.color(255,255,255, 255) ); 
        this.winSize = cc.winSize;
        this.origin = cc.director.getVisibleOrigin(); 
        this.visibleSize = cc.director.getVisibleSize(); 
        //set game over screen visible == false
        this.setGameOverScreen();
        //set touch listeners
        var listener = cc.EventListener.create({
            event: cc.EventListener.TOUCH_ONE_BY_ONE,
            
            swallowTouches: true,
            onTouchBegan: function (touch, event) {  
               var target = event.getCurrentTarget();               
               target.onTouchesBegan(touch, event);
               return true;
                
            },
            onTouchMoved: function (touch, event) {
               
            },
            onTouchEnded: function (touch, event) { 
                
            }
        });
        cc.eventManager.addListener(listener, this);
         //setup score lable 
        this.setScoreLabel();
        //setup Level
        this.setLevel();
        //start game loop 
        this.schedule(this.gameLoop); 
        return true;
    },
    onEnter:function () {
		this._super();
        
    },
    setLevel:function () {
        //init variables
        //keep track on which block the circle is currently on 
        this.currentZorder = 0;
        //How many block will be visible on every given time on the screen 
        this.blocksNum = MAX_BLOCKS_ON_SCREEN;
        //how deep the blocks z order supposed to be , if its long play consider to rise the number
        this.zCount = MAX_Z_ORDER;
        //the bitwise boolean holder
        this.currentState = 0;
        //keep the score
        this.iScore = 0;
        //set into score label , this is needed when game restart
        this.labelScore.setString("0");
        //stop game loop indicator
        this.stopGameLoop = false;
        //set the start bit to on 
        this.currentState |= OperetionFlags.GAME_START;
        //the circle must be alwas above all other Spirits
        this.circle = new cc.Sprite(res.Circle_png); 
        this.circle.setTag(TAGS.CIRCLE);
        this.addChild(this.circle, this.zCount + 1);
        //scatter the blocks and Gems 
        var tempSprite = null;
        for (var i = 0; i < this.blocksNum; i++)
        {
            var spriteBlock = new cc.Sprite(res.Block_png);
            this.blocksList.push(spriteBlock);
            var back = this.blocksList.length-1;
            this.blocksList[back].setAnchorPoint(cc.p(0.0, 0.0));
            var userData  = new UserData(false, false);
            this.blocksList[back].setUserData(userData);
            this.blocksList[back].setTag(TAGS.BLOCK);
            this.zCount = this.zCount - 1;
            this.addChild(this.blocksList[back], this.zCount);
            var blockSizeWidth = this.blocksList[back].getContentSize().width;
            var blockSizHeight = this.blocksList[back].getContentSize().height;

            if (i == 0)
            {   //Start Game Reposition the blocks 
                this.blocksList[back].setPosition(cc.p(this.visibleSize.width / 2 + this.origin.x - (blockSizeWidth / 2),
                    this.visibleSize.height / 2 + this.origin.y - (blockSizHeight / 2)));
                this.circleblockY = this.blocksList[back].getPositionY() + blockSizHeight - (this.circle.getContentSize().height / 2);
                this.circleblockX = this.blocksList[back].getPositionX() + blockSizeWidth / 2;
                this.circle.setPosition(cc.p(this.circleblockX, this.circleblockY));
            }
            else
            {
                //As the first touch is to the right 
                //it is better to give the player some easy learning ajusting to the game 
                var bv2 = cc.p(0,0);
                if (i < 2)
                {
                    bv2 = this.setBlockPostion(tempSprite, 0);                  
                }
                else if (i == 2)
                {
                    bv2 = this.setBlockPostion(tempSprite, 0);
                }
                else if (i == 3)
                {
                    bv2 = this.setBlockPostion(tempSprite, 1);
                }
                else if (i == 4)
                {
                    bv2 = this.setBlockPostion(tempSprite, 1);
                }
                else if (i > 4)
                {
                    bv2 = this.setBlockPostion(tempSprite, this.generateRandDirection(2));
                }
                //set the new position of the block
                this.blocksList[back].setPosition(cc.p(bv2.x + this.origin.x, bv2.y + this.origin.y));
                //some random play ... 
                this.placeRandomGem(this.blocksList[back],i);                
            }
            tempSprite = this.blocksList[back];
        }
    },
    gameLoop:function (delta) {
        if (!this.stopGameLoop)
        {
            this.invokeGame(delta);
        }
    },
    invokeGame:function (delta) {
    
        var bBelow = false;

        if ((this.currentState & OperetionFlags.GAME_TOUCH_START) && (this.currentState & OperetionFlags.GAME_START))
        {
            if ((this.currentState & OperetionFlags.TOUCHED) && (this.currentState &  OperetionFlags.MOVE_RIGHT))
            {
                var x = this.circle.getPositionX() + (CIRCLE_SPEED * delta);
                this.circle.setPositionX(x);
            }
            else if ((this.currentState & OperetionFlags.TOUCHED) && (this.currentState &  OperetionFlags.MOVE_LEFT))
            {
                var x = this.circle.getPositionX() - (CIRCLE_SPEED * delta);
                this.circle.setPositionX(x);
            }
            //iterate the blocks 
            //1.check if circle fell
            //2.check if gem is on block 
            //3.move the blocks down and below the screen 
            //4.add new block on back 
            var randomSeed = 0;             
            for (var  i = 0;  i< this.blocksList.length;  i++)
            {
                var it = this.blocksList[i];
                var circleY = Math.round(this.circle.getPositionY());
                var circleX = Math.round(this.circle.getPositionX());			 
                var blockHeight = it.getPositionY() + it.getContentSize().height;               
                //calculate dimond shape points 
                var A = cc.p(0,0);
                A.x = it.getPositionX() + (it.getContentSize().width / 2);
                A.y = blockHeight - it.getContentSize().width / 2;

                var B = cc.p(0,0);
                B.x = it.getPositionX();
                B.y = blockHeight - (it.getContentSize().width / 2) / 2;

                var C = cc.p(0,0);
                C.x = it.getPositionX() + (it.getContentSize().width / 2);
                C.y = blockHeight + (it.getContentSize().width / 2);

                var D = cc.p(0,0);
                D.x = it.getPositionX() + (it.getContentSize().width);
                D.y = blockHeight - (it.getContentSize().width / 2) / 2;

                //our collision detection is based on checking on our block dimond shape is 
                //intersction with the circle , for this we devide the dimond shape to 2 Triangles
                //and checking each triangle if the circle inside .
                var insideLeft = this.PointInTriangle(this.circle.getPosition(),
                    A,
                    B,
                    C);

                var insideRight = this.PointInTriangle(this.circle.getPosition(),
                    A,
                    C,
                    D);
					
                if (insideRight || insideLeft)
                {
                   this.handleCollision((it));
                }			 
                else
                {
                    //when circle is out side the block stop the game
                    if (it.getUserData().start == true)
                    {
                        if (this.currentZorder == it.getLocalZOrder())
                        {
                            it.getUserData().start = false;
                            this.stopGameLoop = true;
                            //if stoped then invoke the circle drop down animation 
                            this.setFallingAnim();
                        }
                    }
                }

                if (bBelow)
                {
                    this.blocksList.shift();
                    var backSprite = this.blocksList[this.blocksList.length-1];
                }
                var y = it.getPositionY() - (SPEED * delta);
                //move block down
                it.setPositionY(y);

                //check if the sprite is below the screen then next iteration remove it 
                if (it.getPositionY() < 0 - (it.getContentSize().height))
                {
                    //mark to remove it in the next iteration 
                    bBelow = true;
                    //reuse it as new sprite to be on top of the FIFO list
                    var frontSprite = this.blocksList[0];
                    //log("frontSprite x:%f y:%f", frontSprite->getPositionX(), frontSprite->getPositionY());
                    var backSprite = this.blocksList[this.blocksList.length-1];
                    //log("backSprite x:%f y:%f", backSprite->getPositionX(), backSprite->getPositionY());
                    var bv2 = this.setBlockPostion(backSprite, this.generateRandDirection(2));
                    frontSprite.setPosition(cc.p(bv2.x + this.origin.x, bv2.y + this.origin.y));
                    //new Z order alway lower then the last one 
                    this.zCount = this.zCount - 1;
                    frontSprite.setLocalZOrder(this.zCount);
                    //some random play ... 
                    this.placeRandomGem(frontSprite, randomSeed);
                    
                    this.blocksList.push(frontSprite);
                    //++stopThis;
                }
                else
                {
                    bBelow = false;
                }
                randomSeed++;
            }
        }
        else if ((this.currentState & OperetionFlags.GAME_TOUCH_START) && ((this.currentState & OperetionFlags.GAME_START) == 0))
        {
            var y = circle.getPositionY() - ((CIRCLE_SPEED * 10) * delta);
            this.circle.setPositionY(y);
        }
    },
    //set the blocks positions based on previous block 
    setBlockPostion:function (block,dir)
    {
        var vec2 = cc.p(-1,-1);
        var x = block.getPositionX();
        var y = block.getPositionY();
        var w = block.getContentSize().width;
        var h = block.getContentSize().height;
        
        if ((x + w) > this.visibleSize.width)
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
    },
    //Generate random number fo the blocks placemet 
    generateRandDirection:function(modulo)
    {
		//Math.floor(Math.random() * (100 - 2 + 1)) + 2;
        var finalNum = Math.floor(Math.random() * (300 - 2 + 1)) + 2; // Generate the number	
        var randDir = 0;
		var finmodulo = finalNum % modulo
        if (finmodulo!=0)
        {
            randDir = 1;
        }
        return randDir;
    },
    //place the gem according to ramdom play ( can be modified ) 
    placeRandomGem:function (block,seed)
    {
        var placment = this.generateRandDirection(3);
		var finalNum = Math.floor(Math.random() * (300 - 2 + 1)) + 2; // Generate the number	
		var seedresult = finalNum % 2;
        if (seedresult == 0)
        {
            if (placment == 1)
            {
                this.setGem(block);
            }
        }
    },
    //create place the gem above the block 
    setGem:function (block)
    {
        block.getUserData().hasGem = true;
        var gem =new cc.Sprite(res.Gem_png);
        gem.setTag(TAGS.GEM);
        block.addChild(gem, 1);
        //the gem sprite ancor point is in the middle but the gem have also shadow
        //so there is need to fine tune the placment so it will be in the center of the block 
        var grayX = block.getContentSize().width / 2;
        var grayY = block.getContentSize().height - (block.getContentSize().width / 2) / 2;
        gem.setPositionX(grayX);
        gem.setPositionY(grayY);
    },
    //handle the action when collision is detected
    handleCollision:function(block)
    {
        this.currentZorder = block.getLocalZOrder();
        block.getUserData().start = true;
        var isGem = block.getUserData().hasGem;
        //if gem is here hide it increase points by 1
        if (isGem == true)
        {
            this.removeGemFromBlock(block);		
            return true;
        }
        return false;
    },
    removeGemFromBlock:function(block)
    {
        block.getChildByTag(TAGS.GEM).setVisible(false);
        block.removeAllChildren();
        block.visit();
        
        block.getUserData().hasGem = false;
        this.setIncScore();
    },
    sign:function(p1,p2,p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    },
    //check if pt it is in the triangle
    PointInTriangle:function(pt,v1,v2,v3)
    {
        var b1 = false;
        var b2 = false;
        var b3 = false;

        b1 = this.sign(pt, v1, v2) < 0.0;
        b2 = this.sign(pt, v2, v3) < 0.0;
        b3 = this.sign(pt, v3, v1) < 0.0;

        return ((b1 == b2) && (b2 == b3));
    },
    //set the circle falling animation 
    setFallingAnim:function()
    {
       
        var down = -50;
        var iMoveSide = -30;		 
        if ((this.currentState & OperetionFlags.MOVE_RIGHT))
        {
             iMoveSide =  iMoveSide * -1;
        }
        var moveSideDir = this.circle.getPositionX() +  iMoveSide;
        //move circle to the side 
		var circleY =  this.circle.getPositionY();
        var moveSideAction = cc.moveTo(0.2, cc.p(moveSideDir,circleY));
        //move the circle down .
		var circleX =  this.circle.getPositionX();
        var moveDownAction = cc.moveTo(1, cc.p(circleX, -50));
        //call function when circle down animation is done 
        var gameOverAction = cc.callFunc(this.gameOverCallback, this);
        //order all actions
        var seq1 =  cc.sequence(moveSideAction,
                            moveDownAction ,
                            gameOverAction );
        //execute all animations on circle
        this.circle.runAction(seq1);
    },
	//invoke when felling animation sequence is done 
	gameOverCallback:function()
	{
		//invoke the game over screen 
		this.setGameOverScreenVisible(true);
		//turn off game start bit 
		this.currentState &= ~OperetionFlags.GAME_START;
	},
	//set the game over screen visibilty 
	setGameOverScreenVisible:function(isVisible)
	{
		if (isVisible)
		{
			this.gameOverScreen.setVisible(true);
		}
		else
		{
			this.gameOverScreen.setVisible(false);
		}
	},

	//create the game over screen 
	setGameOverScreen:function()
	{
		this.gameOverScreen = new cc.Sprite(res.Blank_png); 
		this.gameOverScreen.setTextureRect(cc.rect(0, 0, this.visibleSize.width - 50, this.visibleSize.height/3));
		this.gameOverScreen.setColor(cc.color(100, 255, 255));
		this.gameOverScreen.setOpacity(200);
		this.gameOverScreen.setPosition(this.visibleSize.width / 2, this.visibleSize.height / 2);
		this.gameOverScreen.setLocalZOrder(this.zCount + 3);
		this.addChild(this.gameOverScreen);
 	 
		var labelGameOver =  new cc.LabelTTF("Game Over Try Again! ","Arial", 44, cc.size(this.gameOverScreen.getContentSize().width / 2,this.gameOverScreen.getContentSize().height /2), cc.TEXT_ALIGNMENT_CENTER); 
		labelGameOver.color = cc.color.BLACK;   
		labelGameOver.setPosition(cc.p(this.gameOverScreen.getContentSize().width / 2,
			this.gameOverScreen.getContentSize().height/2));
		labelGameOver.setLocalZOrder(this.zCount + 4);
		this.gameOverScreen.addChild(labelGameOver);

		this.setGameOverScreenVisible(false);

	},
	//setup the score lable
	setScoreLabel:function()
	{ 		 
	 	this.labelScore =  new cc.LabelTTF("0","Arial", 44, cc.size(this.winSize.width /2, 50), cc.TEXT_ALIGNMENT_CENTER); 
		this.labelScore.color = (cc.color.BLACK);
		this.labelScore.setPosition(cc.p(this.origin.x + this.labelScore.getContentSize().width ,
			this.origin.y + this.visibleSize.height - this.labelScore.getContentSize().height));
		this.addChild(this.labelScore, MAX_Z_ORDER + 1);
    },
	//increase the scroe by 1 
	setIncScore:function()
	{
		this.iScore++;
		this.labelScore.setString(this.iScore);
	},
    onTouchesBegan:function (touch, event)
    {
            //check if game is started 
            if (this.currentState & OperetionFlags.GAME_START)
            {
                //check if touch is invoketed 
                if ((this.currentState & OperetionFlags.GAME_TOUCH_START) == 0)
                {
                    //set touch bit to on 
                    this.currentState |= OperetionFlags.GAME_TOUCH_START;
                }
                //check if right circle movment bit is off
                if ((this.currentState & OperetionFlags.MOVE_RIGHT) == 0)
                {
                    //turn off left bit 
                    this.currentState &= ~OperetionFlags.MOVE_LEFT;
                    //turn on touched and move right bit on 
                    this.currentState |= OperetionFlags.TOUCHED | OperetionFlags.MOVE_RIGHT;
                }
                else if ((this.currentState & OperetionFlags.MOVE_RIGHT)) //check if the right bit if on 
                {
                    //turn off right bit 
                    this.currentState &= ~OperetionFlags.MOVE_RIGHT;
                    //turn on touched and move left bit on 
                    this.currentState |= OperetionFlags.TOUCHED | OperetionFlags.MOVE_LEFT;
                }
            }
            else if((this.currentState & OperetionFlags.GAME_START) == 0)
            {
                //game is ended 
                //Retry touch is trigered reshuffle all blocks and start over 
                var  allNodes = this.getChildren();               
                for (var  it = 0;  it< this.blocksList.length;  it++)
                {
                    var node = this.blocksList[it];
                    if (node.getTag() == TAGS.BLOCK)
                    {
                        this.removeChild(node);
                    }
                    else if (node.getTag() == TAGS.CIRCLE)
                    {
                        this.removeChild(node);
                    }
                }			
                //clean blocks container
                this.blocksList.length = 0;
                //set gameover screen to un visible
                this.setGameOverScreenVisible(false);
                //Start Level all over again 
                this.setLevel();

            }
        
    },
});

var ZipZapZagScene = cc.Scene.extend({
    onEnter:function () {
        this._super();
        var layer = new ZipZapZagLayer();
        this.addChild(layer);
    }
});

